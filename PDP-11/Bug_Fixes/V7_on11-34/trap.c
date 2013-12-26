/*
 *	@(#)trap.c	1.6	(Chemeng) 11/7/83
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/reg.h>
#include <sys/seg.h>
#include <sys/sysinfo.h>
#ifdef	SHARE
#include <sys/lnode.h>
#include <sys/share.h>
#endif	SHARE

#define EBIT	1		/* user error bit in PS: C-bit */
#define SETD	0170011 	/* SETD instruction */
#define SYS	0104400 	/* sys (trap) instruction */
#define USER	020		/* user-mode flag added to dev */
#define MEMORY	((physadr)0177740) /* 11/70 "memory" subsystem */
#ifdef	PDP11_60
#define CPU_ERR ((int *)0177766) /* 11/60 cpu error register */
#endif


int	callshare;		/* Set when time to reschedule */

/*
 * Offsets of the user's registers relative to
 * the saved r0. See reg.h
 */
char	regloc[9] =
{
	R0, R1, R2, R3, R4, R5, R6, R7, RPS
};

/*
 * In case of segmentation violations SP is stored
 * in backsp to allow modification by the instruction
 * backup routine.
 */
int	backsp;

/*
 * Called from l40.s or l45.s when a processor trap occurs.
 * The arguments are the words saved on the system stack
 * by the hardware and software during the trap processing.
 * Their order is dictated by the hardware and the details
 * of C's calling sequence. They are peculiar in that
 * this call is not 'by value' and changed user registers
 * get copied back on return.
 * dev is the kind of trap that occurred.
 */
trap(dev, sp, r1, nps, r0, pc, ps)
int *pc;
dev_t dev;
{
	register i;
	register union {
		int *ip;
		struct proc *pp;
	} a;
	register struct sysent *callp;
	int (*fetch)();
	time_t syst;
	extern char *panicstr;

	syst = u.u_stime;
#if	FPU
	u.u_fpsaved = 0;
#endif
	if ((ps&UMODE) == UMODE)
		dev |= USER;
	u.u_ar0 = &r0;
	switch(minor(dev)) {

	/*
	 * Trap not expected.
	 * Usually a kernel mode bus error.
	 * The numbers printed are used to
	 * find the hardware PS/PC as follows.
	 * (all numbers in octal 18 bits)
	 *	address_of_saved_ps =
	 *		(ka6*0100) + aps - 0140000;
	 *	address_of_saved_pc =
	 *		address_of_saved_ps - 2;
	 *
	 *	Trap 0:
	 *		Illegal instruction
	 *		Bus Error
	 *		Stack Limit
	 *		Illegal Internal Address
	 *		Microbreak (!?)
	 */
#ifdef	PDP11_60
	case 0:
		panicstr = (char *)1;
		printf("cer %o\n", *CPU_ERR);
		goto panic_trap;
#endif

	default:
		panicstr = (char *)1;
	panic_trap:
		printf("ka6 = %o\n", ka6->r[0]);
#if	BIG_UNIX
		printf("ka5 = %o\n", *((int *)ka6 - 1) );
#endif
		printf("aps = %o\n", &ps);
		printf("pc = %o ps = %o\n", pc, ps);
		printf("trap type %o\n", dev);
		panic("trap");

	case 0+USER: /* bus error */
		i = SIGBUS;
		break;

	/*
	 * If illegal instructions are not
	 * being caught and the offending instruction
	 * is a SETD, the trap is ignored.
	 * This is because C produces a SETD at
	 * the beginning of every program which
	 * will trap on CPUs without 11/45 FPU.
	 */
	case 1+USER: /* illegal instruction */
		if(fuiword((caddr_t)(pc-1)) == SETD && u.u_signal[SIGINS] == 0)
			goto out;
		i = SIGINS;
		break;

	case 2+USER: /* bpt or trace */
		i = SIGTRC;
		ps &= ~TBIT;
		break;

	case 3+USER: /* iot */
		i = SIGIOT;
		break;

#ifdef	PDP11_60
	case 4: 	/* power fail */
	case 4+USER:
		panicstr = (char *)1;
		printf("power fail\n");
		goto panic_trap;
#endif

	case 5+USER: /* emt */
#if OVERLAY
		if(u.u_ovdata.uo_ovbase != 0 && r0 <= 7 && r0 > 0)
		{
			ps &= ~EBIT;
			save(u.u_qsav);
			u.u_ovdata.uo_curov = r0;
			estabur(u.u_tsize,u.u_dsize,u.u_ssize,u.u_sep,RO);
			goto out;
		}
		else
			i = SIGEMT;
#else
		i = SIGEMT;
#endif OVERLAY
		break;

	case 6+USER: /* sys call */
		sysinfo.syscall++;
#ifdef	SHARE
		u.u_procp->p_lnode->l_cost += shprm.sh_syscall;
#ifdef	SHDEBUG
		shcnt.sc_syscall++;
#endif	SHDEBUG
#endif	SHARE
		u.u_error = 0;
		ps &= ~EBIT;
		a.ip = pc;
		callp = &sysent[fuiword((caddr_t)(a.ip-1))&077];
		if (callp == sysent) { /* indirect */
			a.ip = (int *)fuiword((caddr_t)(a.ip));
			pc++;
			i = fuword((caddr_t)a.ip);
			a.ip++;
			if ((i & ~077) != SYS)
				i = 0;		/* illegal */
			callp = &sysent[i&077];
			fetch = fuword;
		} else {
			pc += callp->sy_narg - callp->sy_nrarg;
			fetch = fuiword;
		}
		for (i=0; i<callp->sy_nrarg; i++)
			u.u_arg[i] = u.u_ar0[regloc[i]];
		for(; i<callp->sy_narg; i++)
			u.u_arg[i] = (*fetch)((caddr_t)a.ip++);
		u.u_dirp = (caddr_t)u.u_arg[0];
		u.u_r.r_val1 = u.u_ar0[R0];
		u.u_r.r_val2 = u.u_ar0[R1];
		u.u_ap = u.u_arg;
		if (save(u.u_qsav)) {
			if (u.u_error==0)
				u.u_error = EINTR;
		} else {
			(*callp->sy_call)();
		}
		if(u.u_error) {
			ps |= EBIT;
			u.u_ar0[R0] = u.u_error;
		} else {
			u.u_ar0[R0] = u.u_r.r_val1;
			u.u_ar0[R1] = u.u_r.r_val2;
		}
		goto out;

#if	FPU | PDP11_40
	/*
	 * Since the floating exception is an
	 * imprecise trap, a user generated
	 * trap may actually come from kernel
	 * mode. In this case, a signal is sent
	 * to the current process to be picked
	 * up later.
	 */
	case 8: /* floating exception */
#if	! PDP11_40
		stst(&u.u_fper);	/* save error code */
#endif	! PDP11_40
		psignal(u.u_procp, SIGFPT);
		return;

	case 8+USER:
		i = SIGFPT;
#if	! PDP11_40
		stst(&u.u_fper);
#endif	! PDP11_40
		break;
#endif	FPU | PDP11_40

	/*
	 * If the user SP is below the stack segment,
	 * grow the stack automatically.
	 * This relies on the ability of the hardware
	 * to restart a half executed instruction.
	 * On the 11/40 this is not the case and
	 * the routine backup/l40.s may fail.
	 * The classic example is on the instruction
	 *	cmp	-(sp),-(sp)
	 */
	case 9+USER: /* segmentation exception */
	{
		backsp = sp;
		if(backup(u.u_ar0) == 0)
			if(grow((unsigned)backsp))
				goto out;
		i = SIGSEG;
		break;
	}

	/*
	 * The code here is a half-hearted
	 * attempt to do something with all
	 * of the 11/70 parity registers.
	 * In fact, there is little that
	 * can be done.
	 */
	case 10:
	case 10+USER:
		if ((dev & USER) == 0)
			panicstr = (char *)1;
		printf("\nmemory parity\n");
#if	PDP11_23 | PDP11_34 | PDP11_40
		a.ip = MEMPARCSR;
		while ( (i = *a.ip++) >= 0 ) ;
		/* We may get a bus error if we run out of regs */
		printf("address = %o000\n", (i>>3)&0774 );
#endif	PDP11_23 | PDP11_34 | PDP11_40
#if	PDP11_70
		for(i=0; i<4; i++)
			printf("%o ", MEMORY->r[i]);
		printf("\n");
		MEMORY->r[2] = -1;
#endif	PDP11_70
		if (dev & USER) {
			i = SIGKIL;
			uprints("\nMemory parity\n");
			break;
		}
		panic("parity");

	/*
	 * Allow process switch
	 */
	case USER+12:
#ifdef	SHARE
		if (callshare) {
			callshare = 0;
			share();
		}
#endif	SHARE
		goto out;

	/*
	 * Locations 0-2 specify this style trap, since
	 * DEC hardware often generates spurious
	 * traps through location 0.  This is a
	 * symptom of hardware problems and may
	 * represent a real interrupt that got
	 * sent to the wrong place.  Watch out
	 * for hangs on disk completion if this message appears.
	 */
	case 15:
	case 15+USER:
		printf("Interrupt at 0 ignored\n");
		return;
	}
	psignal(u.u_procp, i);

out:
	a.pp = u.u_procp;
	if (a.pp->p_sig && issig()) {
		psig();
	}
#ifndef SHARE
	curpri = setpri(a.pp);
#else
	curpri = a.pp->p_pri = (a.pp->p_cpu >> 1) + PUSER +
			    a.pp->p_nice - NZERO
#ifndef SHDEBUG
			    + a.pp->p_lnode->l_nice;
#else
			    ;
#endif
#endif	SHARE
	while (a.pp->p_flag & SHALT)
		sleep((caddr_t)a.pp, PSLEP);
	if (runrun)
	{
		sysinfo.preempt++;
		qswtch();
	}
	if(u.u_prof.pr_scale)
		addupc((caddr_t)pc, &u.u_prof, (int)(u.u_stime-syst));
#if	FPU
	if (u.u_fpsaved)
		restfp(&u.u_fps);
#endif	FPU
}

/*
 * nonexistent system call-- set fatal error code.
 */
nosys()
{
	u.u_error = EINVAL;
}

/*
 * Ignored system call
 */
nullsys()
{
}
