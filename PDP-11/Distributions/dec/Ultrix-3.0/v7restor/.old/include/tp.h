/*
 * tp directory entry
 */

struct	tpdir {
	char	tp_name[32];
	int	tp_mode;
	char	tp_uid;
	char	tp_gid;
	char	tp_pad1;
	char	tp_size0;
	int	tp_size1;
	long	tp_mtime;
	unsigned tp_tapea;
	char	tp_pad2[16];
	int	tp_check;
};
