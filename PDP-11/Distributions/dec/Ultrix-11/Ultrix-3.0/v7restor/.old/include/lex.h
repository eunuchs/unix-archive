/*
 * lex library header file -- accessed through
 *	#include <lex.h>
 */

#include <stdio.h>

/*
 * description of scanning
 * tables.
 * the entries at the front of
 * the struct must remain in
 * place for the assembler routines
 * to find.
 */
struct	lextab {
	int	llendst;
	char	*lldefault;
	char	*llnext;
	char	*llcheck;
	int	*llbase;
	int	llnxtmax;

	int	(*llmove)();
	int	*llfinal;
	int	(*llactr)();
	int	*lllook;
} *_tabp;

extern FILE	*lexin;		/* scanner input file */
extern FILE	*lexout;	/* scanner output file */
int	_lmovb();	/* state #'s are bytes */
int	_lmovi();	/* state #'s are ints */

#define	LEXERR	256

extern	char	llbuf[];
extern	char	*llend;
