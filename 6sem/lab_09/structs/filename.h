struct filename {
	const char          *name;
	const __user char	*uptr;
	int			        refcnt;
	struct audit_names	*aname;
	const char		    iname[];
};
