# Создание виртуальной файловой системы VFS

## Условие лабораторной работы 

Создать виртуальную файловую систему с минимальным набором действий для регистрации и монтирования собственной файловой системы. 

## Необходимая информация

Процесс монтирование — подготовка раздела диска к использованию файловой состемы, для этого в начале раздела диска выделяется структура суперблок, одним из полей которой является список айнодов, с помощью который можно получить доступ к любому файлу файловой системы.

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

**Примечание:** у VFS 4 точки входа:
- init
- exit
- mount
- kill_sb

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
Таким образом, для того чтобы зарегестрировать файловую систему необходимо определить поле `name` (имя файловой системы, которое на символьном уровне), дополнительно можно определить владельца `owner`. Для монтирования файловой системы достаточно зарегестрировать собственные функции `mount` и `kill_sb`.

**Примечание:** Символьный уровень - уровень пользователя для обращения к файлам по имени, представленным симвальным массимом, так как не удобно обращаться по 16-ти значному числу.

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
В которой вызывается функция ядра `mount_nodev` указывающая, что монтирования выполняется без блочных устройств, куда передаем функцию `my_vfs_fill_sb` заполнения структуры [struct super_block ](https://elixir.bootlin.com/linux/latest/source/include/linux/fs.h#L1136) (структура, которая описывает подмонтированную файловую систему).

Собственная функция `my_vfs_fill_sb` инициализирует необходимые поля структуры `superblock`, а именно:
```c
static struct super_operations const my_vfs_sup_ops = {
    .put_super = my_vfs_put_super,
    .statfs = simple_statfs,
    .drop_inode = generic_delete_inode
};


static int my_vfs_fill_sb(struct super_block *sb, void *data, int silent) 
{
    ...    
    sb->s_blocksize = PAGE_SIZE; // размер страницы, т.к выделяется память страницами
    sb->s_blocksize_bits = PAGE_SHIFT;
    sb->s_magic = MYVFS_MAGIC_NUMBER; // магическое число - адрес файловой системы
    sb->s_op = &my_vfs_sup_ops; // операции над superblock
    ...
    root_inode = my_vfs_make_inode(sb, S_IFDIR | 0755);
    ...
    sb->s_root = d_make_root(root_inode);
}
```
**Примечание:** 
- simple_statfs - это функция, которая предоставляет статистику файловой системы (например, общий размер диска, количество свобод места, количество файловых блоков и т.д.). Эта функция используется, когда файловая система не поддерживает собственную реализацию метода
- generic_delete_inode - это функция, используемая для удаления индексного узла (inode) из файловой системы, когда ссылок на этот inode больше нет. Она используется, когда файловая система не имеет своей собственной реализации метода delete_inode.

В `superblock` есть поле `s_root`, структуры [struct dentry](https://elixir.bootlin.com/linux/latest/source/include/linux/dcache.h#L82), которая является корневым каталогом и точкой монтирования при монтировании файловой системы. Для создании вызывается функция ядра `d_make_root`, возвращает корневой каталог, но прежде необходимо создать и проинициализировать `inode`, для это есть собственная функция `my_vfs_make_inode`, в котрой inode создается функцией ядра `new_inode` в качестве парметра ей передается экземпляр (объект) `superblock`, это сделано потому, что необходимо связать созданный inode с конкретной подмантированной файловой системой, точнее заносим inode в поле `s_inodes`, представляеющее список всех `inode`'ов.

**Примечание:** Структура [struct inode](https://elixir.bootlin.com/linux/latest/source/include/linux/fs.h#L595) описывает файл на диске. Это часть файловой системы в задачи, которой входят долговременное хранение и обеспечение доступа - для этого и создаются inode -> для того, чтобы не терять доступ к файлам dentry содержит указатель на суперблок. Rогда обращаемся к файлу, то налету будут создаваться объекты dentry, которые сохранены в долговременной памяти.

### Slab Кеш

Slab-кеш (кеш ядра Linux) - предназначен для повторного использования уже инициализированных объектов (их не придется заново инициализировать), пересень объектов при этом в ядре ограничен. При удалении объекта память не освобождается, а записывается в slab кеш, что приводит к эффективному управлению памяти и ускорения работы с объектами. Побочным эффектом является устранение фрагментации памяти.

Если необходимо вывести инфoрмацию о slab-кеш:
```bash
cat /proc/slabinfo
``` 

Просто кеш хранится в оперативной памяти и предназчен для быстрого доступа к часто запрашиваемой информации. Если информация не запрашивается долгое время, то она выгружается из кеша.
