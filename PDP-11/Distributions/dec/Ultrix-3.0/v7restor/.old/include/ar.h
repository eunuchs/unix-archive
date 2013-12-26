#define	ARMAG	0177545
struct	ar_hdr {
	char	ar_name[14];
	long	ar_date;
#ifndef waterloo
	char	ar_uid;
	char	ar_gid;
#else
	int	ar_uid;
#endif
	int	ar_mode;
	long	ar_size;
};
