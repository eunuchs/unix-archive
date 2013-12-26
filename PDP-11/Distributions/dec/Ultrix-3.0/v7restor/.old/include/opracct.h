/*
 * Structure of an
 * opr accounting record.
 * Stored in a file controlled
 * by the prof/t.a., and indexed
 * by user ID.
 */
struct	opracct {
	int	o_jobs;
	int	o_fail;
	long	o_lines;
	long	o_limit;
};

/*
 * Size of header & trailer pages in lines
 * (for charging purposes).
 */
#define HEADSIZ	100
