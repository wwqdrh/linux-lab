# 环境搭建

`ubuntu22`

```bash
apt-get update && apt-get install -y wget gcc-multilib bin86 make bear

wget http://old-releases.ubuntu.com/ubuntu/pool/main/g/gcc-3.4/cpp-3.4_3.4.6-6ubuntu2_amd64.deb

wget http://old-releases.ubuntu.com/ubuntu/pool/main/g/gcc-3.4/gcc-3.4-base_3.4.6-6ubuntu2_amd64.deb

wget http://old-releases.ubuntu.com/ubuntu/pool/main/g/gcc-3.4/gcc-3.4_3.4.6-6ubuntu2_amd64.deb

dpkg -i gcc-3.4-base_3.4.6-6ubuntu2_amd64.deb

dpkg -i cpp-3.4_3.4.6-6ubuntu2_amd64.deb

dpkg -i gcc-3.4_3.4.6-6ubuntu2_amd64.deb
```

# 编译

> 使用bear构建可以获取compile_commands.json文件，用于代码跳转

```bash
git clone https://github.com/wwqdrh/linux-lab.git

bear -- make
```

# 运行

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