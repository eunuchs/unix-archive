/*
 * structure filled by getrmpw (B)
 * must consist only of strings, unless
 * that routine, and the rmpass command
 * are changed.
 * if this is changed, getrmpw and rmpass
 * must be recompiled.
 */

struct	rmpass {
	char	*r_uid;
	char	*r_name;
	char	*r_hwname;
	char	*r_hwpass;
	char	*r_hwprio;
	char	*r_ibmid;
	char	*r_ibmbox;
	char	*r_vmid;
	char	*r_vmpass;
};
