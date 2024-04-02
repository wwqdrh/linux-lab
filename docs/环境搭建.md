# 环境搭建

`ubuntu22`

```bash
apt-get update && apt-get install -y wget gcc-multilib bin86 make bear qemu-system

wget http://old-releases.ubuntu.com/ubuntu/pool/main/g/gcc-3.4/cpp-3.4_3.4.6-6ubuntu2_amd64.deb && sudo dpkg -i cpp-3.4_3.4.6-6ubuntu2_amd64.deb

wget http://old-releases.ubuntu.com/ubuntu/pool/main/g/gcc-3.4/gcc-3.4-base_3.4.6-6ubuntu2_amd64.deb && sudo dpkg -i gcc-3.4-base_3.4.6-6ubuntu2_amd64.deb

wget http://old-releases.ubuntu.com/ubuntu/pool/main/g/gcc-3.4/gcc-3.4_3.4.6-6ubuntu2_amd64.deb && sudo dpkg -i gcc-3.4_3.4.6-6ubuntu2_amd64.deb
```

# 编译

> 使用bear构建可以获取compile_commands.json文件，用于代码跳转

```bash
git clone -b v0.11 https://github.com/wwqdrh/linux-lab.git

cd linux-lab

mkdir rootfs

wget -O rootfs/hdc-0.11.img https://github.com/wwqdrh/linux-lab/releases/download/v0.11/hdc-0.11.img

bear -- make
```

# 运行

由于一般ubuntu-server没有图形界面无法使用gtk，所以在使用qemu-system进行启动镜像时采用vnc, 启动后，需要使用vnc客户端链接该主机

```bash
make start
```

# 调试

```bash
gdb -q tools/system

# 进入gdb命令
$ break main
$ target remote :1234
$ s
```