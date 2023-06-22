#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <asm/io.h>
#include "ascii.h"
#define IRQ_NUM 1

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mansurov Vladislav");

char *my_tasklet_data = "my_tasklet data";
struct tasklet_struct* my_tasklet;


void my_tasklet_function(unsigned long data)
{
    printk( ">> my_tasklet: Bottom-half handled time=%llu\n", ktime_get());
    int code = my_tasklet->data;
    if (my_tasklet->data < 84)
        printk(">> my_tasklet: Key is %s\n", ascii[code]);
    printk(">> my_tasklet: ------------------------------------\n");
}

irqreturn_t my_handler(int irq, void *dev)
{
    printk(">> my_tasklet: Top-half start time=%llu\n", ktime_get());
    int code;
    if (irq == IRQ_NUM)
    {
        code = inb(0x60);
        my_tasklet->data = code;
        printk(">> my_tasklet: Key code is %d\n", code);
        tasklet_schedule(my_tasklet);
		printk(">> my_tasklet: Bottom-half sheduled time=%llu\n", ktime_get());
        return IRQ_HANDLED;
    }
    printk(">> my_tasklet: irq wasn't handled\n");
    return IRQ_NONE;
}

static int __init my_init(void)
{
    if (request_irq(IRQ_NUM, my_handler, IRQF_SHARED, "my_tasklet", &my_handler))
    {
        printk(">> my_tasklet: ERROR request_irq\n");
        return -1;
    }
    my_tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
    if (!my_tasklet)
    {
        printk(">> my_tasklet: ERROR kmalloc!\n");
        return -1;
    }
    tasklet_init(my_tasklet, my_tasklet_function, (unsigned long)my_tasklet_data);

    printk(">> my_tasklet: module loaded\n");
    return 0;
}

static void __exit my_exit(void)
{
    tasklet_kill(my_tasklet);
    kfree(my_tasklet);
    free_irq(IRQ_NUM, &my_handler);
    printk(">> my_tasklet: " "module unloaded\n");
}

module_init(my_init)
module_exit(my_exit)

















// sudo dmesg -wH | grep my_tasklet 
// cat /proc/interrupts 
// cat /proc/interrupts |head -n 1 && cat /proc/interrupts | grep my_tasklet

// Регистрируем прерывание (IRQ_NUM=1 - клавиатура, my_handler - указатель на обработчик прерывания 
// IRQF_SHARED - разрешает разделение irq несколькими устройствами, 
// "name" - имя устройства, вызывающего прерывание (для справки), 
// my_handler - id девайса, нужно при удалении. Если больше одного обработчика, то 
// то при удалении обязательно указывать, какой обработчик удаляем. Технически это id обработчика. 
// Так как у нас один обработчик, то можно вписывать туда NULL. Это назвали dev_id, тк подразумевается, 
// что у одного девайса один обработчик прерываний.)
