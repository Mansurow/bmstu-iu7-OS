#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vladislav Mansurov");

#define COOKIE_POT_SIZE PAGE_SIZE
#define file_name "fortuneFile"
#define dir_name "fortuneDir"
#define sym_link "fortuneLink"

static struct proc_dir_entry *fortuneFile;
static struct proc_dir_entry *fortuneDir;
static struct proc_dir_entry *fortuneLink;

static char *cookiePot;
char tmpBuf[COOKIE_POT_SIZE];

static int readInd = 0;
static int writeInd = 0;

static void freeMemory(void) {
  if (fortuneFile != NULL) remove_proc_entry(file_name, NULL);

  if (fortuneDir != NULL) remove_proc_entry(dir_name, NULL);

  if (fortuneLink != NULL) remove_proc_entry(sym_link, NULL);

  vfree(cookiePot);
}

static int fortuneOpen(struct inode *spInode, struct file *spFile) {
  printk(KERN_INFO "fortune: open called\n");
  return 0;
}

static int fortuneRelease(struct inode *spInode, struct file *spFile) {
  printk(KERN_INFO "fortune: release called\n");
  return 0;
}

static ssize_t fortuneWrite(struct file *file, const char __user *buf,
                            size_t len, loff_t *fPos) 
{
  printk(KERN_INFO "fortune: write called\n");

  if (len > COOKIE_POT_SIZE - writeInd + 1) {
    printk(KERN_ERR "fortune: buffer overflow\n");
    return -ENOSPC;
  }

  if (copy_from_user(&cookiePot[writeInd], buf, len) != 0) {
    printk(KERN_ERR "fortune: copy_from_user error\n");
    return -EFAULT;
  }

  writeInd += len;
  cookiePot[writeInd - 1] = '\0';

  return len;
}

static ssize_t fortuneRead(struct file *file, char __user *buf, size_t len,
                           loff_t *fPos) {
  int readLen;

  printk(KERN_INFO "fortune: read called\n");

  if ((*fPos > 0) || (writeInd == 0)) return 0;

  if (readInd >= writeInd) readInd = 0;

  readLen = snprintf(tmpBuf, COOKIE_POT_SIZE, "%s\n", &cookiePot[readInd]);

  if (/(buf, tmpBuf, readLen) != 0) {
    printk(KERN_ERR "fortune: copy_to_user error\n");
    return -EFAULT;
  }

  readInd += readLen;
  *fPos += readLen;

  return readLen;
}

static const struct proc_ops fops = {.proc_open = fortuneOpen,
                                     .proc_read = fortuneRead,
                                     .proc_write = fortuneWrite,
                                     .proc_release = fortuneRelease};

static int __init md_init(void) {
  printk(KERN_INFO "fortune: init\n");

  if ((cookiePot = vmalloc(COOKIE_POT_SIZE)) == NULL) {
    printk(KERN_ERR "fortune: memory error\n");
    return -ENOMEM;
  }

  memset(cookiePot, 0, COOKIE_POT_SIZE);

  if ((fortuneFile = proc_create(file_name, 0666, NULL, &fops)) == NULL) {
    printk(KERN_ERR "fortune: create file err\n");
    freeMemory();

    return -ENOMEM;
  }


  if ((fortuneDir = proc_mkdir(dir_name, NULL)) == NULL) {
    printk(KERN_ERR "fortune: create dir err\n");
    freeMemory();

    return -ENOMEM;
  }


  if ((fortuneLink = proc_symlink(sym_link, NULL, dir_name)) == NULL) {
    printk(KERN_ERR "fortune: create link err\n");
    freeMemory();

    return -ENOMEM;
  }

  readInd = 0;
  writeInd = 0;

  printk(KERN_INFO "fortune: loaded\n");

  return 0;
}

static void __exit md_exit(void) {
  printk(KERN_INFO "fortune: exit\n");
  freeMemory();
}

module_init(md_init);
module_exit(md_exit);