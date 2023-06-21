# Загружаемые модули ядра. История ошибок

## **Условие**

Реализовать три загружаемых модуля ядра:
- Вызываемый модуль md1
- Вызывающий модуль md2
- «Отладочный» модуль md3
Каждый загружаемый модуль должен содержать:
- Указание лицензии GPL
- Указание автора

Загружаемые модули должны собираться при помощи Make-файла (сборка командой
make). Вызов каждой функции модуля должен сопровождаться записью в
системный журнал информации, какая функция какого модуля была вызвана.

### **Модуль md1**
Модуль md1 демонстрирует возможность создания экспортируемых данных и функций.

Данный модуль ядра должен содержать:
- Экспортируемые строковые (char *) и численные (int) данные.
- Экспортируемые функции возвращающие строковые и числовые значения.
Например:
- Функция, возвращающая в зависимости от переданного целочисленного
параметра различные строки (на усмотрение студента);
- Функция, производящая подсчет факториала переданного целочисленного
параметра;
- Функция возвращающая 0;

#### **Модуль md2**
Модуль md2 демонстрирует использование данных и функций экспортируемых первым модулем (md1).

Данный модуль должен при загрузке:● Вызывать все экспортированные модулем md1 процедуры и вывести в системный
журнал возвращаемые ими значения с указанием имени вызванной процедуры.
- Вывести в системный журнал все экспортированные модулем md1 данные.

### **Модуль md3**
Модуль md3 демонстрирует сценарий некорректного завершения установки модуля, и возможность использования загружаемого модуля в качестве функции выполняемой в пространстве ядре.

Процедура инициализации этого загружаемого модуля должна возвращать ненулевое значение и выводить в системный журнал данные и возвращаемые значения экспортированных модулем md1 процедур (аналогично md2).
Данный модуль включен в работу для проработки вопросов, связанных с отладкой модулей ядра.

## **Процесс**

### Ошибки на этапе компиляции 

1. Объявить и описать функцию в md1, но определить ее в заголовном файле md.h и затем вызвать функцию в md2, что приведет к ошибке компиляции:
```bash
$ make
make -w -C /lib/modules/5.19.0-45-generic/build M=/home/vladislav/OS/6sem/lab_05 modules 
make[1]: Entering directory '/usr/src/linux-headers-5.19.0-45-generic'
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0
  You are using:           gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0
  CC [M]  /home/vladislav/OS/6sem/lab_05/md1.o
/home/vladislav/OS/6sem/lab_05/md1.c:11:14: warning: ‘md1_local’ defined but not used [-Wunused-function]
   11 | static char* md1_local( void ) {
      |              ^~~~~~~~~
  CC [M]  /home/vladislav/OS/6sem/lab_05/md2.o
In file included from ./include/linux/kernel.h:29,
                 from ./arch/x86/include/asm/percpu.h:27,
                 from ./arch/x86/include/asm/nospec-branch.h:14,
                 from ./arch/x86/include/asm/paravirt_types.h:40,
                 from ./arch/x86/include/asm/ptrace.h:97,
                 from ./arch/x86/include/asm/math_emu.h:5,
                 from ./arch/x86/include/asm/processor.h:13,
                 from ./arch/x86/include/asm/timex.h:5,
                 from ./include/linux/timex.h:67,
                 from ./include/linux/time32.h:13,
                 from ./include/linux/time.h:60,
                 from ./include/linux/stat.h:19,
                 from ./include/linux/module.h:13,
                 from /home/vladislav/OS/6sem/lab_05/md2.c:2:
/home/vladislav/OS/6sem/lab_05/md2.c: In function ‘md_init’:
/home/vladislav/OS/6sem/lab_05/md2.c:14:57: error: implicit declaration of function ‘md1_noexport’ [-Werror=implicit-function-declaration]
   14 |      printk( "+ string returned md1_noexport() : %s\n", md1_noexport() );
cc1: some warnings being treated as errors
make[2]: *** [scripts/Makefile.build:257: /home/vladislav/OS/6sem/lab_05/md2.o] Error 1
make[1]: *** [Makefile:1857: /home/vladislav/OS/6sem/lab_05] Error 2
make[1]: Leaving directory '/usr/src/linux-headers-5.19.0-45-generic'
make: *** [Makefile:10: default] Error 2
```

2. 
```c
static char* md1_local( void ) { 
    return md1_data; 
}
```
По причине того, что функция md1_local - `static` она не может экспортироваться в другие модули, для это необзодимо использовать `excport`. Следовательно возникнет ошибка при компиляции:
```bash
$ make
make -w -C /lib/modules/5.19.0-45-generic/build M=/home/vladislav/OS/6sem/lab_05 modules 
make[1]: Entering directory '/usr/src/linux-headers-5.19.0-45-generic'
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0
  You are using:           gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0
  CC [M]  /home/vladislav/OS/6sem/lab_05/md1.o
/home/vladislav/OS/6sem/lab_05/md1.c:11:14: error: static declaration of ‘md1_local’ follows non-static declaration
   11 | static char* md1_local( void ) {
      |              ^~~~~~~~~
In file included from /home/vladislav/OS/6sem/lab_05/md1.c:3:
/home/vladislav/OS/6sem/lab_05/md.h:4:14: note: previous declaration of ‘md1_local’ with type ‘char *(void)’
    4 | extern char* md1_local(void);
      |              ^~~~~~~~~
/home/vladislav/OS/6sem/lab_05/md1.c:11:14: warning: ‘md1_local’ defined but not used [-Wunused-function]
   11 | static char* md1_local( void ) {
      |              ^~~~~~~~~
make[2]: *** [scripts/Makefile.build:257: /home/vladislav/OS/6sem/lab_05/md1.o] Error 1
make[1]: *** [Makefile:1857: /home/vladislav/OS/6sem/lab_05] Error 2
make[1]: Leaving directory '/usr/src/linux-headers-5.19.0-45-generic'
make: *** [Makefile:10: default] Error 2
```

## Ошибки при сборке загрузаемого модуля

Если не указать функцию/макрос ядра `EXPORT_SYMBOL( md1_noexport )`, то даже в случае если указать, что функция extern и в заголовоном файле и модуле, то без этого функция для другого модуля будет считаться непередаваемым и возникает ошибка при сборке модуля modpost:
```bash
$ make
make -w -C /lib/modules/5.19.0-45-generic/build M=/home/vladislav/OS/6sem/lab_05 modules 
make[1]: Entering directory '/usr/src/linux-headers-5.19.0-45-generic'
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0
  You are using:           gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0
  CC [M]  /home/vladislav/OS/6sem/lab_05/md1.o
  CC [M]  /home/vladislav/OS/6sem/lab_05/md2.o
  CC [M]  /home/vladislav/OS/6sem/lab_05/md3.o
  MODPOST /home/vladislav/OS/6sem/lab_05/Module.symvers
ERROR: modpost: "md1_noexport" [/home/vladislav/OS/6sem/lab_05/md2.ko] undefined!
make[2]: *** [scripts/Makefile.modpost:128: /home/vladislav/OS/6sem/lab_05/Module.symvers] Error 1
make[1]: *** [Makefile:1771: modules] Error 2
make[1]: Leaving directory '/usr/src/linux-headers-5.19.0-45-generic'
make: *** [Makefile:10: default] Error 2
```
## Загрузка модулей

Так модуль md2 зависит от md1 и ему необходим физический адресс (абсолютный адресс) модуля md1, то и здесь при загрузки собранных модулей могут возникнут ошибки:

1. Пытаемся загрзуть md2, но md1 не загружен:
```bash
$ sudo insmod md2.ko
insmod: ERROR: could not insert module md2.ko: Unknown symbol in module
```
2. Пытаемся выгрузить md1, но md2 не выгружен:
```bash
$ sudo insmod md1.ko
$ sudo insmod md2.ko
$ sudo rmmod md1.ko
rmmod: ERROR: Module md1 is in use by: md2
```

Верная последняя загрузка и выгрузка модулей в данном случае:
```bash
sudo insmod md1.ko
sudo insmod md2.ko
sudo rmmod md2.ko
sudo rmmod md1.ko
```

Модуль md3 не загрузится, так как функция init возвращает -1, а должно возвращаться 0, как впррочем для любой функции модуля: 
```c
static int __init md_init( void ) { 
    printk( "+ module md2 start!\n" ); 
    printk( "+ data string exported from md1 : %s\n", md1_data ); 
    printk( "+ string returned md1_proc() is : %s\n", md1_proc() ); 
    return -1; 
} 
```

```bash
$ sudo insmod md3.ko
insmod: ERROR: could not insert module md3.ko: Unknown symbol in module
```