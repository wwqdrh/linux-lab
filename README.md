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
