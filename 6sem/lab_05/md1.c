#include <linux/init.h> 
#include <linux/module.h> 
#include "md.h"

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Mansurov Vladislav");  
char* md1_data = "Hello World!"; 
extern char* md1_proc( void ) { 
    return md1_data; 
} 
static char* md1_local( void ) { 
    return md1_data; 
} 
extern char* md1_noexport( void ) { 
    return md1_data; 
} 
EXPORT_SYMBOL( md1_data ); 
EXPORT_SYMBOL( md1_proc );

static int __init md_init( void ) { 
    printk( "+ module md1 start!\n" ); 
    return 0; 
}
static void __exit md_exit( void ) { 
    printk( "+ module md1 unloaded!\n" ); 
}
module_init( md_init ); 
module_exit( md_exit );