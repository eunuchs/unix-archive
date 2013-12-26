/ This program does a standalone getfile to floppy
/ This performs the same action as getfile.c, but is
/ shorter, so that loading with micro-odt isn't too
/ tedious.
rxc0 = 174204
rxc1 = 174206
rxc2 = 174210
rxca = 174222
rxsc = 174224
rxbi = 174226
rxcs = 177560
rxdb = 177562
txcs = 177564
txdb = 177566
ps = 177776
done = 10
reset = 60
write = 160
high = 177400
prefix = 5
ackch = 7
nakch = 11
halt = 0
	mov	$340,*$ps
	clr	r2
	mov	$157776,sp
1:
	bit	$done,*$rxc0
	beq	1b
3:
	jsr	r5,getcar
	cmp	$prefix,r0
	bne	3b
	clr	r3
	mov	$512.,r1
	clr	*$rxca
back:
	jsr	r5,getcar
	add	r0,r3
	mov	r0,*$rxbi
	sob	r1,back
	jsr	r5,getcar
	bic	$high,r3
	cmp	r0,r3
	bne	nak
	mov	r2,r5
	mov	r4,-(sp)
	asl	r5
	clr	r4
	div	$10.,r4
	add	r4,r5
	mov	(sp),r4
	asl	r4
	add	r4,r5
	clr	r4
	div	$10.,r4
	inc	r5
	mov	r5,*$rxc2
	mov	(sp)+,r4
	inc	r4
	mov	r4,*$rxc1
	mov	$write,*$rxc0
	clr	*$rxsc
2:
	bit	$done,*$rxc0
	beq	2b
	tstb	*$rxc0
	bmi	nak
	inc	r2
	mov	$ackch,r0
	jsr	r5,putcar
	br	3b
nak:
	mov	$reset,*$rxc0
	clr	*$rxsc
1:
	bit	$done,*$rxc0
	beq	1b
	mov	$nakch,r0
	jsr	r5,putcar
	br	3b
putcar:
	tstb	*$txcs
	bge	putcar
	mov	r0,*$txdb
	rts	r5
getcar:
	tstb	*$rxcs
	bge	getcar
	mov	*$rxdb,r0
	rts	r5
