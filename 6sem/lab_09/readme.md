# Открытые файлы (Системный вызов open)

## Условие

Составить отчет по системному вызову open по открытому исходному коду.

Отчет должен включать: титульный лист и схему алгоритма работы системного вызова open(). Дополнительно можно включить исходный код систменого вызова open, но необбяхательно. Указать версию ядра, по которому составлялись схемы алгоритмов. 

В схему нужно обязательно включить:
1. копирование названия файла из пространства пользователя в пространтсво ядра;
2. блокировка/разблокировка (spinlock) структуры files_struct;
3. алгоритм поиска свободного дескриптора открытого файла;
4. работу со структурой nameidata;
5. алгоритм разбора пути;
6. инициализация struct file;
7. открытие файла для чтения, запси или выполнения;
8. создание inode в случае отсуствия открываемого файла.

Системный вызовы определяется с помощью макроса `SYSCALL_DEFINE`.
Системный вызов open не является исключением и является оберткой функции ядра ksys_open(), которая в свою очередь вызывает функцию `do_sys_open()`.

## Необходимая информация

Manuals:
- [Linux Manual](https://man7.org/linux/man-pages/man2/open.2.html)
- [Manual на русском](https://www.opennet.me/man.shtml?topic=open&category=2&russian=0)

Исходный код макросов и функций:
1. [fs/open.c](https://elixir.bootlin.com/linux/latest/source/fs/open.c)
    - [SYSCALL_DEFINE3](https://elixir.bootlin.com/linux/latest/source/fs/open.c#L1376)
    - [do_sys_open](https://elixir.bootlin.com/linux/latest/source/fs/open.c#L1369)
    - [do_sys_openat2](https://elixir.bootlin.com/linux/latest/source/fs/open.c#L1340)
    - [build_open_how](https://elixir.bootlin.com/linux/latest/source/fs/open.c#L1140)
    - [build_open_flags](https://elixir.bootlin.com/linux/latest/source/fs/open.c#L1156) 
2. [fs/namei.c](https://elixir.bootlin.com/linux/latest/source/fs/namei.c)
    - [getname_flags](https://elixir.bootlin.com/linux/latest/source/fs/namei.c#L130)
    - [do_filp_open](https://elixir.bootlin.com/linux/latest/source/fs/namei.c#L3734)
    - [path_openat](https://elixir.bootlin.com/linux/latest/source/fs/namei.c#L3695)
    - [open_last_lookups](https://elixir.bootlin.com/linux/latest/source/fs/namei.c#L3695)
    - [lookup_open](https://elixir.bootlin.com/linux/latest/source/fs/namei.c#L3321)
    - [do_open](https://elixir.bootlin.com/linux/latest/source/fs/namei.c#L3516)
3. [fs/file.c](https://elixir.bootlin.com/linux/latest/source/fs/file.c)
    - [alloc_fd](https://elixir.bootlin.com/linux/latest/source/fs/file.c#L499)


