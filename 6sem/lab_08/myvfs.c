#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/time.h>

#include <asm/atomic.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mansurov Vladislav");

#define MYVFS_MAGIC_NUMBER 0xAAAABBBB //адрес файловой системы
#define SLAB_NAME "MyVFSCache"

static void **cache_mem_area = NULL;
struct kmem_cache * cache = NULL;

static struct my_vfs_inode
{
    int i_mode;
    unsigned long i_ino;
} my_vfs_inode;

static int size = sizeof(struct my_vfs_inode);

static struct inode *my_vfs_make_inode(struct super_block *sb, int mode)
{
    struct inode *ret = new_inode(sb);

    if (ret)
    {
        inode_init_owner(&init_user_ns, ret, NULL, mode);
        ret->i_size = PAGE_SIZE;
        ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);
        ret->i_private = &my_vfs_inode;
        ret->i_ino = 1;
    }mk

    printk(KERN_INFO "MyVFS: struct inode created\n");
    
    return ret;
}

static void my_vfs_put_super(struct super_block *sb)
{
    printk(KERN_INFO "MyVFS: super block destroyed!\n");
}

static struct super_operations const my_vfs_sup_ops = {
    .put_super = my_vfs_put_super,
    .statfs = simple_statfs,
    .drop_inode = generic_delete_inode
};

static int my_vfs_fill_sb(struct super_block *sb, void *data, int silent)
{
    struct inode *root_inode;
    sb->s_blocksize = PAGE_SIZE;
    sb->s_blocksize_bits = PAGE_SHIFT;
    sb->s_magic = MYVFS_MAGIC_NUMBER;
    sb->s_op = &my_vfs_sup_ops;

    root_inode = my_vfs_make_inode(sb, S_IFDIR | 0755);
    if (!root_inode)
    {
        printk(KERN_ERR "MyVFS: my_vfs_make_inode error\n");
        return -ENOMEM;
    }

    root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = current_time(root_inode);
    root_inode->i_op = &simple_dir_inode_operations;
    root_inode->i_fop = &simple_dir_operations;

    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root)
    {
        printk(KERN_ERR "MyVFS: d_make_root error\n");
        iput(root_inode);
        return -ENOMEM;
    }
    else
        printk(KERN_INFO "MyVFS: VFS root created\n");

    printk(KERN_INFO "MyVFS: super_block init");

    return 0;
}

static struct dentry *
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

static void func_init(void *p)
{
    *(int *)p = (int)p;
}

void my_kill_litter_super(struct super_block *sb)
{
    printk(KERN_INFO "MyVFS: my_kill_litter_super");

    return kill_litter_super(sb);
}

static struct file_system_type my_vfs_type = {
    .owner   = THIS_MODULE,
    .name    = "MyVFS",
    .mount   = my_vfs_mount, 
    .kill_sb = my_kill_litter_super
};

static int __init my_vfs_init(void)
{
    int ret = register_filesystem(&my_vfs_type);

    if (ret != 0)
    {
        printk(KERN_ERR "MyVFS: register_filesystem error\n");
        return ret;
    }

    cache_mem_area = (void **)kmalloc(sizeof(void *), GFP_KERNEL);
    if (!cache_mem_area)
    {
        printk(KERN_ERR "MyVFS: cache_mem_area error\n");
        kfree(cache_mem_area);
        return -ENOMEM;
    }

    cache = kmem_cache_create(SLAB_NAME, sizeof(void *), 0,
                              SLAB_HWCACHE_ALIGN, func_init);
    if (!cache)
    {
        printk(KERN_ERR "MyVFS: kmem_cache_create error\n");
        kmem_cache_destroy(cache);
        kfree(cache_mem_area);
        return -ENOMEM;
    }

    if (((*cache_mem_area) = kmem_cache_alloc(cache, GFP_KERNEL)) == NULL)
    {
        printk(KERN_ERR "MyVFS: kmem_cache_alloc error\n");
        kmem_cache_free(cache, *cache_mem_area);
        kmem_cache_destroy(cache);
        kfree(cache_mem_area);
        return -ENOMEM;
    }

    printk(KERN_INFO "MyVFS: loaded\n");

    return 0;
}


static void __exit my_vfs_exit(void)
{
    kmem_cache_free(cache, *cache_mem_area);
    kmem_cache_destroy(cache);
    kfree(cache_mem_area);

    if (unregister_filesystem(&my_vfs_type) != 0)
        printk(KERN_ERR "MyVFS: unregister_filesystem error\n");

    printk(KERN_INFO "MyVFS: exit\n");
}


module_init(my_vfs_init);
module_exit(my_vfs_exit);