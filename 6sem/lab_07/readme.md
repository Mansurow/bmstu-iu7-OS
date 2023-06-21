# Proc. Часть 2. Sequence Files.

## Условие

## Процесс
- Расширенный seq_file (Итератор)

```bash
$ make

$ sudo insmod seq_file.ko

$ echo "aaaaa" > /proc/seqfiles/seqfile
$ echo "bbbbb" > /proc/seqfiles/seqfile
$ echo "ccccc" > /proc/seqfiles/seqfile

$ cat  /proc/seqfiles/seqfile
aaaaa
bbbbb
ccccc

$ cat  /proc/seqfiles/seqfile

$ sudo rmmod seq_file.ko
```

Syslog:
```
$ dmesg | grep +

[14039.027047] +: seq_file: Module loaded.
[14076.858478] +: seq_file: open seq_file
[14076.858521] +: seq_file: write
[14082.931275] +: seq_file: open seq_file
[14082.931317] +: seq_file: write
[14090.428232] +: seq_file: open seq_file
[14090.428263] +: seq_file: write
[14098.242823] +: seq_file: open seq_file
[14098.242859] +: seq_file: read
[14098.242866] +: seq_file: start
[14098.242869] +: seq_file: show, 00000000681b90d3
[14098.242877] +: seq_file: next 00000000681b90d3
[14098.242881] +: seq_file: stop 0000000000000000
[14098.242919] +: seq_file: read
[14098.242923] +: seq_file: start
[14098.242925] +: seq_file: stop 0000000000000000
[14115.725959] +: seq_file: unloaded
```

- Single интерфейс seq_file

```bash
$ make

$ sudo insmod single_file.ko

$ echo "aaaaa" > /proc/seqfiles/seqfile
$ echo "bbbbb" > /proc/seqfiles/seqfile
$ echo "ccccc" > /proc/seqfiles/seqfile

$ cat  /proc/seqfiles/seqfile
aaaaa
bbbbb
ccccc

$ cat  /proc/seqfiles/seqfile

$ sudo rmmod single_file.ko
```

Syslog:
```
$ dmesg | grep +
[14481.116361] + module loaded!
[14545.164598] + module unloaded!
[14585.893818] + module loaded!
[14605.123913] + open() called
[14605.123957] + write() called
[14605.123970] + release() called
[14609.624198] + open() called
[14609.624232] + write() called
[14609.624244] + release() called
[14614.163976] + open() called
[14614.164008] + write() called
[14614.164019] + release() called
[14624.051719] + open() called
[14624.051757] + read() called
[14624.051767] + show() was called
[14624.051797] + read() called
[14624.051816] + release() called
[14626.475076] + open() called
[14626.475108] + read() called
[14626.475116] + show() was called
[14626.475138] + release() called
[14654.341576] + module unloaded!
```

**Примечание:** Single - упрощенный интерфейс для seq_files. В seq_files (итераторе) используется инициализация структруы struct seq_operations и регистрации функций stop,start,next, которые должны быть объявлены и описаны перед регистрацией; а в single files данные функции регистрируется путем вызовом `single_open()` в которую передается собственная функция `show`.

## Необходимая информация

В данной программе 6 точек входа:
- init и exit, которые вызываются при загрузки (insmod) и выгрузки (rmmod) модуля соотвественно;
- read, которая вызывается после открытия файла (open) для чтения; 
- write, которая вызывается после открытия файла (open) для записи;
- open, которая вызыывается при каждом чтении или записи для открытия файла, например команда `cat` или `echo`;
- release, которая вызывается пр каждом чтении или записи для закрытия файла;

которые регистрируются в структуре `struct proc_ops`.


В собственных функцияx `read` и `write`, используются функции `copy_from_user` и `copy_to_user`, которые используются по причине того, что ядра оперирует физическими адрессами, а у процесса виртуальное адресное пространство.

**Примечание:** При этом у виртуального адресного пространтсва есть недостаток - когда ядро обращается к буферу пользователя, страницы с ним может не быть в памяти. Если страницы нет в памяти, то возникает страничное прерывание, в результате которого страница будет загружена в физическую память.


[Использование файлов-последовательностей ядра Linux](https://habr.com/ru/articles/358184/)