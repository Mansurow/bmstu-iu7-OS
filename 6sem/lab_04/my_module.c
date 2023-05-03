#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/fs_struct.h>
#include <linux/path.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mansurov Vladislav");
MODULE_DESCRIPTION("Print info about task");
MODULE_VERSION("Version 1.0");

static int __init md_init(void)
{
    struct task_struct *task = &init_task;
    do
    {
        printk(KERN_INFO "TASK: taskp - %llu, task - %s, pid - %d, parent - %s, ppid - %d, state - %d, flags - %x, prio - %d, root - %s, policy - %d, pwd - %s\n", 
        task,
        task->comm,
        task->pid,
        task->parent->comm,
        task->parent->pid,
        task->__state,
        task->flags,
        task->prio,
        task->fs->root.dentry->d_iname,
        task->policy,
        task->fs->pwd.dentry->d_name.name
        );
    } while ((task = next_task(task)) != &init_task);

    printk(KERN_INFO "CURRENT: taskp - %llu, task - %s, pid - %d, parent - %s, ppid - %d, state - %d, flags - %x, prio - %d, root - %s, policy - %d, pwd - %s\n\n",
        current,
        current->comm,
        current->pid,
        current->parent->comm,
        current->parent->pid,
        current->__state,
        current->flags,
        current->prio,
        current->fs->root.dentry->d_iname,
        current->policy, 
        current->fs->pwd.dentry->d_name.name
    );

  return 0;
}

static void __exit md_exit(void)
{
    printk(KERN_INFO "TASK_INFO: Good buy!\n");
}

module_init(md_init);
module_exit(md_exit);