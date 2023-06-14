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
На вход, которой передается структура `struct file_system_type`
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