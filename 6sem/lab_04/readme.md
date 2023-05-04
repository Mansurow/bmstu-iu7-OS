# Загруженные модули ядра. Кольцевой список процессов.
## Условие лабораторной работы 

Реализовать загружаемый модуль ядра, осуществляющий перебор списка задач (struct task_struct) и в системный файл /var/log/messages выводит идентификатор каждого встреченного процесса и имя его исполняемого файла, идентификатор процесса предка и имя этого файла. При инициализации модуля следует также использовать символ current для вывода такой же информации о текущем процессе. При выгрузке модуля записывается “Good by”. Модуль должен собираться при помощи Make-файла. 
Загружаемый модуль должен содержать:
- Указание лицензии GPL
- Указание автора

- Команда загрузка модуля в ядро - `sudo insmod my_module.ko`
- Команда выгрузки модуля из ядра - `sudo rmmod my_module.ko`

## Необходимая информация

Инструмент поиска необходимой информации в исходном коде Linux - [elixir](https://elixir.bootlin.com)  

### Информация о процессе - task
- [Структура task_struct в исходном коде](https://elixir.bootlin.com/linux/latest/source/include/linux/sched.h#L737)
- [Manual sched.h](https://man7.org/linux/man-pages/man7/sched.7.html)

1. `int policy` - алгоритм планирования

Значения поля policy - [строка в исходном коде](https://elixir.bootlin.com/linux/latest/source/include/uapi/linux/sched.h#L114):
```c
#define SCHED_NORMAL  0 // планирование по умолчанию для обычных процессов
#define SCHED_FIFO    1 // алгоритм планирование fifo, предназнчен для процессов в реальном времени и migration
                        // алгоритм fifo выполняется от начала и до конца без вытеснения и переключения.
#define SCHED_RR      2 // алгоритм round robin 
                        // данный алгоритм с переключением, каждому процессу выделяет квант времени и 
                        // по истечению кванта процесс помещается в конец очереди и переключается 
                        // на следующий процесс из очереди.
#define SCHED_BATCH   3 // планирование с динамическим приоритетом.
```
2. `int prio` - приотет процесса
  - 1-99 - процессы в реальном времени;
  - 100-139 - обычные процессы, при этом по умолчанию устанавливается 120;
  - 0 - migration - процесс, который переспределяет процессы между ядрами (количество ядер соотвествует количесву логических ядер);

**Примечание:** Для процессa migration алгоритм планирования устанавливается fifo, по причине того что данный процесс имеет наивысший приоритет и ему нужно выполнится от начало и до конца.
```shell
// Example
[  320.117946] TASK: taskp - 18446619926608295040, task - migration/0, pid - 16, parent - kthreadd, ppid - 2, state - 1, flags - 4208040, prio - 0, root - /, policy - 1, pwd - /
[  320.117963] TASK: taskp - 18446619926619731072, task - migration/1, pid - 22, parent - kthreadd, ppid - 2, state - 1, flags - 4208040, prio - 0, root - /, policy - 1, pwd - /
[  320.117980] TASK: taskp - 18446619926619815936, task - migration/2, pid - 28, parent - kthreadd, ppid - 2, state - 1, flags - 4208040, prio - 0, root - /, policy - 1, pwd - /
[  320.117995] TASK: taskp - 18446619926619881472, task - migration/3, pid - 34, parent - kthreadd, ppid - 2, state - 1, flags - 4208040, prio - 0, root - /, policy - 1, pwd - /
[  320.118012] TASK: taskp - 18446619926621913088, task - migration/4, pid - 40, parent - kthreadd, ppid - 2, state - 1, flags - 4208040, prio - 0, root - /, policy - 1, pwd - /
[  320.118029] TASK: taskp - 18446619926622024832, task - migration/5, pid - 46, parent - kthreadd, ppid - 2, state - 1, flags - 4208040, prio - 0, root - /, policy - 1, pwd - /
[  320.118046] TASK: taskp - 18446619926622050880, task - migration/6, pid - 52, parent - kthreadd, ppid - 2, state - 1, flags - 4208040, prio - 0, root - /, policy - 1, pwd - /
[  320.118063] TASK: taskp - 18446619926623663232, task - migration/7, pid - 58, parent - kthreadd, ppid - 2, state - 1, flags - 4208040, prio - 0, root - /, policy - 1, pwd - /
```

3. `int state` - состояние процесса
Значения поля state - [строка в исходном коде](https://elixir.bootlin.com/linux/latest/source/include/linux/sched.h#L85)
```c
#define TASK_RUNNING         0   // процесс выполняется
#define TASK_INTERRUPTIBLE   1   // INTERRUPTIBLE SLEEP - прерываемый сон (в ожидании завершения события), обозначаетс S при ps -ajx
#define TASK_UNINTERRUPTIBLE 2   // UNINTERRUPTIBLE_SLEEP - непрерываемый сон (в ожидании завершения ввода/вывода), обозначается D при ps -ajx
#define TASK_ZOMBIE          4   // процесс зобми, у которого отобраны все ресурсы, кроме строки в таблицы процессов
#define TASK_STOPPED         8   // процесс остановлен по сигналу или ptrace()
#define TASK_EXCLUSIVE       32  // используется совместно с TASK_INTERRUPTIBLE или с TASK_UNINTERRUPTIBLE (по OR)
```

**Примечание:** Взято из man ps 
```
Here are the different values that the s, stat and state output
       specifiers (header "STAT" or "S") will display to describe the state of
       a process:

               D    uninterruptible sleep (usually IO)
               I    Idle kernel thread
               R    running or runnable (on run queue)
               S    interruptible sleep (waiting for an event to complete)
               T    stopped by job control signal
               t    stopped by debugger during the tracing
               W    paging (not valid since the 2.6.xx kernel)
               X    dead (should never be seen)
               Z    defunct ("zombie") process, terminated but not reaped by
                    its parent
```
4. `int flags` - флаги процесса
Значения поля flags (формируется побитным или) - [строка в исходном коде](https://elixir.bootlin.com/linux/latest/source/include/linux/sched.h#L1718):
```c
#define PF_VCPU			0x00000001	/* I'm a virtual CPU */
#define PF_IDLE			0x00000002	/* I am an IDLE thread */
#define PF_EXITING		0x00000004	/* Getting shut down */
#define PF_POSTCOREDUMP		0x00000008	/* Coredumps should ignore this task */
#define PF_IO_WORKER		0x00000010	/* Task is an IO worker */
#define PF_WQ_WORKER		0x00000020	/* I'm a workqueue worker */
#define PF_FORKNOEXEC		0x00000040	/* Forked but didn't exec */
#define PF_MCE_PROCESS		0x00000080      /* Process policy on mce errors */
#define PF_SUPERPRIV		0x00000100	/* Used super-user privileges */
#define PF_DUMPCORE		0x00000200	/* Dumped core */
#define PF_SIGNALED		0x00000400	/* Killed by a signal */
#define PF_MEMALLOC		0x00000800	/* Allocating memory */
#define PF_NPROC_EXCEEDED	0x00001000	/* set_user() noticed that RLIMIT_NPROC was exceeded */
#define PF_USED_MATH		0x00002000	/* If unset the fpu must be initialized before use */
#define PF__HOLE__00004000	0x00004000
#define PF_NOFREEZE		0x00008000	/* This thread should not be frozen */
#define PF__HOLE__00010000	0x00010000
#define PF_KSWAPD		0x00020000	/* I am kswapd */
#define PF_MEMALLOC_NOFS	0x00040000	/* All allocation requests will inherit GFP_NOFS */
#define PF_MEMALLOC_NOIO	0x00080000	/* All allocation requests will inherit GFP_NOIO */
#define PF_LOCAL_THROTTLE	0x00100000	/* Throttle writes only against the bdi I write to,
						 * I am cleaning dirty pages from some other bdi. */
#define PF_KTHREAD		0x00200000	/* I am a kernel thread */
#define PF_RANDOMIZE		0x00400000	/* Randomize virtual address space */
#define PF__HOLE__00800000	0x00800000
#define PF__HOLE__01000000	0x01000000
#define PF__HOLE__02000000	0x02000000
#define PF_NO_SETAFFINITY	0x04000000	/* Userland is not allowed to meddle with cpus_mask */
#define PF_MCE_EARLY		0x08000000      /* Early kill for mce process policy */
#define PF_MEMALLOC_PIN		0x10000000	/* Allocation context constrained to zones which allow long term pinning. */
#define PF__HOLE__20000000	0x20000000
#define PF__HOLE__40000000	0x40000000
#define PF_SUSPEND_TASK		0x80000000      /* This thread called freeze_processes() and should not be frozen */
```
***Примечание:** У процесса migration поле flags равно числу 4208040 в 16 сс, это значит, что у него имеются следующие флаги:
- PF_FORKNOEXEC - процесс был создан, но не вызвана exec;
- PF_KTHREAD - процесс в режиме ядра;
- PF_NOFREEZE - процесс не блокируется;
- PF_NO_SETAFFINITY	 - процесс не может быть замаскирован, т.е.связан с конкретным ядром.
