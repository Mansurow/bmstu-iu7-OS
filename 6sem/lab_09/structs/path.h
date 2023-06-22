struct path {
	struct vfsmount *mnt;
	struct dentry *dentry;
} __randomize_layout;
