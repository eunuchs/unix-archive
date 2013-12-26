/*
 * coroutine package -- header file
 */
typedef struct _corout {
	struct _corout	*_caller;	/* coroutine starting this level with call() */
	struct _corout	*_invoker;	/* coroutine starting this cr. with resume() */
	char	*_display;	/* r5 used to restart this cr. */
	int	_stack[];	/* stack follows */
} COROUT;

COROUT	*_curr;
int	_fcvsize;

#define	destroy(cs)	free(cs)
#define	cospace(nb)	(_fcvsize = sizeof(*_curr)+(nb))
#define	invoker()	(_curr->_invoker)
#define	self()	(_curr)
