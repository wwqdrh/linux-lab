# usage

hdc-0.11.img是一个文件系统镜像，它的格式是Minix文件系统的镜像。Linux所有版本都支持这种格式的文件系统，所以可以直接在宿主Linux上通过mount命令访问此文件内的文件。这样可以达到宿主系统和bochs内运行的Linux 0.11之间交换文件的效果

## versions

> 源码归档路径: https://kernel.googlesource.com/pub/scm/linux/kernel/git/nico/archive/

|版本号|发布日期|说明|
|-|-|-|
|0.00|1991|内部版本|
|0.01|1991|第一个正式对外发布版本，多线程文件系统，分段和分页内存管理，还不包含软盘驱动程序|
|0.02|1991|内部版本|
|0.10|1991|增加了内存分配哭函数|
|0.11|1991|基本可以正常运行的内核版本|
|0.12|1992|增加了数学协同处理器的软件模拟程序，增加了作业控制，许虚拟控制台，文件符号链接和虚拟内存对换功能|
|0.95.x(0.13)|1992|加入虚拟文件系统支持，值包含MINIX文件系统，增加了登录功能，改善了软盘驱动程序和文件系统的性能，MINIX系统相同，支持CDROM|
|0.96.x|1992|加入unixsocket支持，增加了ext文件系统alpha测试程序，SCSI驱动加入内核，软盘类型自动识别，改善串行驱动，高速缓冲、内存管理性能，支持动态链接库|
|0.97.x|1992|增加了SCSI驱动支持，动态高速缓冲功能，msdos和ext文件系统支持，总线鼠标驱动程序，内核被映射到线性地址3GB处|
|0.98.x|1992|改善TCP/IP(0.8.1)网络的支持，重写内部管理部分。从0.98.4开始每个进程可同时打开256个文件（原来32个），并且进程的内核堆栈独立使用一个内存页面|
|0.99.x|1992|重新设计进程对内存的使用分配，每个进程有4G线性空间，改进网络代码，NFS支持|
|1.0|1994|第一个正式版本|

## use

```bash
docker run -d --name command -p 2002:22 -v $(pwd):/code hrdqww/dev-c
```

然后在vscode使用远程连接进入容器，`root@123456`

## dev

> 一些例子依赖minix内核模块，但是docker提供了linux内核中不包含，所以建议在虚拟机中使用ubuntu镜像编译而不是使用docker

`docker环境`

> ubuntu:22.10里的镜像源已经broken了

```bash
cd 0.11

docker build -t [name]:latest .

# 如果有代理，加上 --build-arg HTTP_PROXY=http://192.168.0.105:7890 --build-arg HTTPS_PROXY=http://192.168.0.105:7890

docker run -itd --name ubuntu --network host -v $(pwd)"/0.11":/0.11 [name]:latest bash

docker exec -it ubuntu bash

cd 0.11

make
```

如果需要代码跳转，清使用`bear -- make`, 将会生成compile_commands.json用于代码跳转，需要注意的是请自行将生成的内容中的文件路径改为绝对路径

## 汇编代码

- mov: 用于数据传送,如 mov eax, 100 将立即数100传送到eax寄存器，支持 8位、16位和 32位操作数的传送
- movl eax, [edx] ; 32位传送
- add/sub/mul/div: 用于整数运算,如 add eax, 10给eax寄存器增加10
- push/pop: 用于操作栈,入栈和出栈
- call/ret: 用于调用函数和从函数返回
- cmp: 用于比较两个操作数的大小
- test: 用于逻辑与操作,类似于C语言中的&操作
- jump: 无条件跳转指令,如 jmp label跳转到label位置
- je/jne/jg: 条件跳转指令,根据前一次比较结果跳转
- loop: 循环控制指令,配合cx寄存器实现循环
- lea: 加载有效地址,常用于获取变量地址，支持 16位和 32位地址计算
- leal: 仅用于 32位地址计算
- xchg: 交换两个寄存器的内容
- int: 生成软中断
- iret: 从中断返回
- nop: 空指令,什么都不做

## 示例

### Syscall -- 添加系统调用

A new demonstration is added: [Linux 0.11 Lab: Add a new syscall into Linux 0.11](http://showterm.io/4b628301d2d45936a7f8a)

  Host:

    $ patch -p1 < examples/syscall/syscall.patch
    $ make start-hd

  Emulator:

    $ cd examples/syscall/
    $ make
    as -o syscall.o syscall.s
    ld -o syscall syscall.o
    ./syscall
    Hello, Linux 0.11

**Notes** If not `examples/` found in default directory: `/usr/root` of Linux
          0.11, that means your host may not have minix fs kernel module, compile
          one yourself, or switch your host to Ubuntu.

### Linux 0.00

  Host:

    $ make boot-hd

  Emulator:

    $ cd examples/linux-0.00
    $ make
    $ sync

  Host:

    $ make linux-0.00

### Building Linux 0.11 in Linux 0.11

  Host:

    $ make boot-hd

  Emulator:

    $ cd examples/linux-0.11
    $ make
    $ sync

  Host:

    $ make hd-boot
