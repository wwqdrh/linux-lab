# usage

## versions

- 0.00
- 0.01
- 0.11
- 0.12
- 0.99
- ...

## dev

> 一些例子依赖minix内核模块，但是docker提供了linux内核中不包含，所以建议在虚拟机中使用ubuntu镜像编译而不是使用docker

`docker环境`

> ubuntu:22.10里的镜像源已经broken了

参考下面路径，`tools/docker/Dockerfile`

```bash
docker build -t [name]:latest .

# 如果有代理，加上 --build-arg HTTP_PROXY=http://192.168.0.105:7890 --build-arg HTTPS_PROXY=http://192.168.0.105:7890

docker run -itd --name ubuntu --network host -v $(pwd)"/0.11":/0.11 [name]:latest bash

docker exec -it ubuntu bash
```

## error

ld -r -o kernel.o sched.o system_call.o traps.o asm.o fork.o panic.o printk.o vsprintf.o sys.o exit.o signal.o mktime.o
ld: Relocatable linking with relocations from format elf32-i386 (sched.o) to format elf64-x86-64 (kernel.o) is not supported

make[1]: *** [kernel.o] Error 1
解决办法，在x86-64上链接出x86 文件，添加 -m elf_i386 选项

```
ld -m elf_i386 -r -o kernel.o sched.o system_call.o traps.o asm.o fork.o panic.o printk.o vsprintf.o sys.o exit.o signal.o mktime.o

ld -m elf_i386fs -r -o mm.o memory.o page.o

ld -m elf_i386 -r -o fs.o open.o read_write.o inode.o file_table.o buffer.o super.o \
	block_dev.o char_dev.o file_dev.o stat.o exec.o pipe.o namei.o \
	bitmap.o fcntl.o ioctl.o truncate.o
```

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
