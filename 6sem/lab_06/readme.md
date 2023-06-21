# Proc. Часть 2. Fortune

## Условие

Написать программу – загружаемый модуль ядра (LKM) – которая поддерживает чтение из пространства пользователя и запись в пространство пользователя из пространства ядра. 
После загрузки модуля пользователь должен иметь возможность загружать в него строки с помощью команды echo, а затем считывать их с помощью команды cat.

- Команда загрузка модуля в ядро - `sudo insmod fortune.ko`
- Команда выгрузки модуля из ядра - `sudo rmmod fortune.ko`

## Процесс

```bash
$ sudo insmod fortune.ko

$ echo "aaaaa" > /proc/fortuneDir/fortuneFile 
$ echo "bbbbb" > /proc/fortuneDir/fortuneFile 
$ echo "ccccc" > /proc/fortuneDir/fortuneFile 

$ cat /proc/fortuneDir/fortuneFile
aaaaa
$ cat /proc/fortuneDir/fortuneFile
bbbbb
$ cat /proc/fortuneDir/fortuneFile 
ccccc
$ cat /proc/fortuneDir/fortuneFile
aaaaa
$ cat /proc/fortuneDir/fortuneFile
bbbbb
$ cat /proc/fortuneDir/fortuneFile 
ccccc

$ sudo rmmod fortune.ko
```

Syslog:
```
$ dmesg | grep fortune

[13500.302535] fortune: init
[13500.302568] fortune: loaded
[13520.536139] fortune: open called
[13520.536170] fortune: write called
[13520.536180] fortune: release called
[13520.536139] fortune: open called
[13520.536170] fortune: write called
[13520.536180] fortune: release called
[13520.536139] fortune: open called
[13520.536170] fortune: write called
[13520.536180] fortune: release called
[13538.588673] fortune: open called
[13538.588705] fortune: read called
[13538.588730] fortune: read called
[13538.588748] fortune: release called
[13538.588673] fortune: open called
[13538.588705] fortune: read called
[13538.588730] fortune: read called
[13538.588748] fortune: release called
[13538.588673] fortune: open called
[13538.588705] fortune: read called
[13538.588730] fortune: read called
[13538.588748] fortune: release called
[13538.588673] fortune: open called
[13538.588705] fortune: read called
[13538.588730] fortune: read called
[13538.588748] fortune: release called
[13538.588673] fortune: open called
[13538.588705] fortune: read called
[13538.588730] fortune: read called
[13538.588748] fortune: release called
[13538.588673] fortune: open called
[13538.588705] fortune: read called
[13538.588730] fortune: read called
[13538.588748] fortune: release called
[13565.499322] fortune: exit
```

**Примечание:** Если заметить то read выводится 2 раза, это обусловлено, тем что впервый раз возвращается количество прочитанных байтов из файла, а второй раз 0, так как файл прочитан.

## Необходимая информация

В данной программе 6 точек входа:
- init и exit, которые вызываются при загрузки (insmod) и выгрузки (rmmod) модуля соотвественно;
- read, которая вызывается после открытия файла (open) для чтения; 
- write, которая вызывается после открытия файла (open) для записи;
- open, которая вызыывается при каждом чтении или записи для открытия файла, например команда `cat` или `echo`;
- release, которая вызывается пр каждом чтении или записи для закрытия файла;

которые регистрируются в структуре `struct proc_ops`.

**Примечание:** Используем буфер данных кольцевой, следовательно вывод данных не закончится при вызовах `cat`, а начнется снова.

В собственных функцияx `read` и `write`, используются функции `copy_from_user` и `copy_to_user`, которые используются по причине того, что ядра оперирует физическими адрессами, а у процесса виртуальное адресное пространство.

**Примечание:** При этом у виртуального адресного пространтсва есть недостаток - когда ядро обращается к буферу пользователя, страницы с ним может не быть в памяти. Если страницы нет в памяти, то возникает страничное прерывание, в результате которого страница будет загружена в физическую память.





