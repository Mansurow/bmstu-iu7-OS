# Создание виртуальной файловой системы VFS

## Условие лабораторной работы 

Создать виртуальную файловую систему с минимальным набором действий для регистрации и монтирования собственной файловой системы. 

## Необходимая информация

- Сборка командой - `make`
- Очистка созданных файлов после сборки - `make clean`

Сборка и загрузка драйвера системы ничем не отличается от сборки и загрузки обычного модуля, т.е. используются команды:

- Команда загрузка модуля в ядро - `sudo insmod myvfs.ko`
- Команда выгрузки модуля из ядра - `sudo rmmod myvfs.ko`

Вместо реального диска для экспериментов будем использовать `loop` устройство. Это такой драйвер "диска", который пишет данные не на физическое устройство, а в файл (образ диска). 

Создадим образ диска, пока он не хранит никаких данных, поэтому все просто:
```bash
touch image
```
Кроме того, нужно создать каталог, который будет точкой монтирования (корнем) файловой системы:
```bash
mkdir directory
```
Теперь, используя этот образ, примонтируем файловую систему:
```bash
sudo mount -o loop -t MyVFS ./image ./directory
```
Если операция завершилась удачно, то в системном логе можно увидеть сообщения от модуля: 
```bash
sudo dmesg
```
Если необходимо вывести дерево каталогов: 
```bash
$ ls -ail

1389486 drwxrwxr-x  3 vladislav vladislav    4096 Jun 14 18:46 .
1389438 drwxrwxr-x 11 vladislav vladislav    4096 May  5 01:43 ..
      1 drwxr-xr-x  1 root      root         4096 Jun 14 15:49 directory
1393153 -rw-rw-r--  1 vladislav vladislav       0 Jun 13 13:02 image
1393724 -rw-rw-r--  1 vladislav vladislav     374 Jun  5 13:41 Makefile
1393468 -rw-rw-r--  1 vladislav vladislav    4343 Jun 14 16:51 myvfs.c
1389335 -rw-rw-r--  1 vladislav vladislav    2071 Jun 14 18:40 readme.md
``` 

Если необходимо продемонстрировать зарегестрированные файловые системы:
- Через интерфейс proc:
```bash
$ cat /proc/filesystems

nodev   sysfs
nodev   tmpfs
nodev   bdev
nodev   proc
nodev   cgroup
nodev   cgroup2
nodev   cpuset
nodev   devtmpfs
nodev   configfs
nodev   debugfs
nodev   tracefs
nodev   securityfs
nodev   sockfs
nodev   bpf
nodev   pipefs
nodev   ramfs
nodev   hugetlbfs
nodev   devpts
        ext3
        ext2
        ext4
        squashfs
        vfat
nodev   ecryptfs
        fuseblk
nodev   fuse
nodev   fusectl
nodev   efivarfs
nodev   mqueue
nodev   pstore
nodev   autofs
nodev   binfmt_misc
nodev   MyVFS
```
**Примечание:** Если nodev указан то это означает, что не требуются блочные устройства для монтирования файловой системы. Если же пусто, то это означает, что требуются блочные устройства для монтирования файловой системы.

Если необходим продемонтрировать подмонтированные файловые системы:
- Через интерфейс proc:
```bash
$ cat /proc/mounts | grep MyVFS

/dev/loop23 /home/vladislav/OS/6sem/lab_08/directory MyVFS rw,relatime 0 0
```
- Через команду `mount`:
```bash
$ mount | grep MyVFS
/var/lib/snapd/snaps/telegram-desktop_4826.snap on /home/vladislav/OS/6sem/lab_08/directory type MyVFS (rw,relatime)
```
Чтобы размонтировать файловую систему делаем так:
```bash
sudo umount ./directory
```
Для удаления созданого образа и директории:
```bash
rm image
rm -rf directory
```

Для регистрации собственной файловой системы используется функция:
```c
int register_filesystem(struct file_system_type *);
```
На вход, которой передается структура [struct file_system_type](https://elixir.bootlin.com/linux/latest/source/include/linux/fs.h#L2189):
```c
struct file_system_type {
	const char *name;
	int fs_flags;
#define FS_REQUIRES_DEV		1 
#define FS_BINARY_MOUNTDATA	2
#define FS_HAS_SUBTYPE		4
#define FS_USERNS_MOUNT		8	/* Can be mounted by userns root */
#define FS_DISALLOW_NOTIFY_PERM	16	/* Disable fanotify permission events */
#define FS_ALLOW_IDMAP         32      /* FS has been updated to handle vfs idmappings. */
#define FS_RENAME_DOES_D_MOVE	32768	/* FS will handle d_move() during rename() internally. */
	int (*init_fs_context)(struct fs_context *);
	const struct fs_parameter_spec *parameters;
	struct dentry *(*mount) (struct file_system_type *, int,
		       const char *, void *);
	void (*kill_sb) (struct super_block *);
	struct module *owner;
	struct file_system_type * next;
	struct hlist_head fs_supers;

	struct lock_class_key s_lock_key;
	struct lock_class_key s_umount_key;
	struct lock_class_key s_vfs_rename_key;
	struct lock_class_key s_writers_key[SB_FREEZE_LEVELS];

	struct lock_class_key i_lock_key;
	struct lock_class_key i_mutex_key;
	struct lock_class_key invalidate_lock_key;
	struct lock_class_key i_mutex_dir_key;
};
```
Таким образом, для того чтобы зарегестрировать файловую систему необходимо определить поле `name` (имя файловой системы), дополнительно можно определить владельца `owner`. Для монтирования файловой системы достаточно зарегестрировать собственные функции `mount` и `kill_sb`.

Так как до того как зарегестрировать функцию необходимо ее определить и описать:
- `mount`
```c
my_vfs_mount(struct file_system_type *type, int flags,
             const char *dev, void *data)
{
    struct dentry *const root = mount_nodev(type, flags, data, my_vfs_fill_sb);

    if (IS_ERR(root))
        printk(KERN_ERR "MyVFS: mounting failed\n");
    else
        printk(KERN_ERR "MyVFS: mounted\n");

    return root;
}
```
В которой вызывается функция ядра `mount_nodev` указывающая, что монтирования выполняется без блочных устройств, куда передаем функцию `my_vfs_fill_sb` заполнения структуры [struct super_block ](https://elixir.bootlin.com/linux/latest/source/include/linux/fs.h#L1136): 

```c
struct super_block {
	struct list_head	s_list;		/* Keep this first */
	dev_t			    s_dev;		/* search index; _not_ kdev_t */
	unsigned char		s_blocksize_bits;
	unsigned long		s_blocksize;
	loff_t			    s_maxbytes;	/* Max file size */
	struct file_system_type	*s_type;
	const struct super_operations	*s_op;
	const struct dquot_operations	*dq_op;
	const struct quotactl_ops	*s_qcop;
	const struct export_operations *s_export_op;
	unsigned long		s_flags;
	unsigned long		s_iflags;	/* internal SB_I_* flags */
	unsigned long		s_magic;
	struct dentry		*s_root;
	struct rw_semaphore	s_umount;
	int			s_count;
	atomic_t		s_active;
#ifdef CONFIG_SECURITY
	void                    *s_security;
#endif
	const struct xattr_handler **s_xattr;
#ifdef CONFIG_FS_ENCRYPTION
	const struct fscrypt_operations	*s_cop;
	struct fscrypt_keyring	*s_master_keys; /* master crypto keys in use */
#endif
#ifdef CONFIG_FS_VERITY
	const struct fsverity_operations *s_vop;
#endif
#if IS_ENABLED(CONFIG_UNICODE)
	struct unicode_map *s_encoding;
	__u16 s_encoding_flags;
#endif
	struct hlist_bl_head	s_roots;	/* alternate root dentries for NFS */
	struct list_head	s_mounts;	/* list of mounts; _not_ for fs use */
	struct block_device	*s_bdev;
	struct backing_dev_info *s_bdi;
	struct mtd_info		*s_mtd;
	struct hlist_node	s_instances;
	unsigned int		s_quota_types;	/* Bitmask of supported quota types */
	struct quota_info	s_dquot;	/* Diskquota specific options */

	struct sb_writers	s_writers;

	/*
	 * Keep s_fs_info, s_time_gran, s_fsnotify_mask, and
	 * s_fsnotify_marks together for cache efficiency. They are frequently
	 * accessed and rarely modified.
	 */
	void			*s_fs_info;	/* Filesystem private info */

	/* Granularity of c/m/atime in ns (cannot be worse than a second) */
	u32			s_time_gran;
	/* Time limits for c/m/atime in seconds */
	time64_t		   s_time_min;
	time64_t		   s_time_max;
#ifdef CONFIG_FSNOTIFY
	__u32			s_fsnotify_mask;
	struct fsnotify_mark_connector __rcu	*s_fsnotify_marks;
#endif

	char			s_id[32];	/* Informational name */
	uuid_t			s_uuid;		/* UUID */

	unsigned int		s_max_links;
	fmode_t			s_mode;

	/*
	 * The next field is for VFS *only*. No filesystems have any business
	 * even looking at it. You had been warned.
	 */
	struct mutex s_vfs_rename_mutex;	/* Kludge */

	/*
	 * Filesystem subtype.  If non-empty the filesystem type field
	 * in /proc/mounts will be "type.subtype"
	 */
	const char *s_subtype;

	const struct dentry_operations *s_d_op; /* default d_op for dentries */

	struct shrinker s_shrink;	/* per-sb shrinker handle */

	/* Number of inodes with nlink == 0 but still referenced */
	atomic_long_t s_remove_count;

	/*
	 * Number of inode/mount/sb objects that are being watched, note that
	 * inodes objects are currently double-accounted.
	 */
	atomic_long_t s_fsnotify_connectors;

	/* Being remounted read-only */
	int s_readonly_remount;

	/* per-sb errseq_t for reporting writeback errors via syncfs */
	errseq_t s_wb_err;

	/* AIO completions deferred from interrupt context */
	struct workqueue_struct *s_dio_done_wq;
	struct hlist_head s_pins;

	/*
	 * Owning user namespace and default context in which to
	 * interpret filesystem uids, gids, quotas, device nodes,
	 * xattrs and security labels.
	 */
	struct user_namespace *s_user_ns;

	/*
	 * The list_lru structure is essentially just a pointer to a table
	 * of per-node lru lists, each of which has its own spinlock.
	 * There is no need to put them into separate cachelines.
	 */
	struct list_lru		s_dentry_lru;
	struct list_lru		s_inode_lru;
	struct rcu_head		rcu;
	struct work_struct	destroy_work;

	struct mutex		s_sync_lock;	/* sync serialisation lock */

	/*
	 * Indicates how deep in a filesystem stack this SB is
	 */
	int s_stack_depth;

	/* s_inode_list_lock protects s_inodes */
	spinlock_t		s_inode_list_lock ____cacheline_aligned_in_smp;
	struct list_head	s_inodes;	/* all inodes */

	spinlock_t		s_inode_wblist_lock;
	struct list_head	s_inodes_wb;	/* writeback inodes (список измененныйх inodes) */
} __randomize_layout;
```