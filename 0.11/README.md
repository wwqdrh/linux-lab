# 内存管理

<img src="./docs/image/内存布局.jpg"/>

> 主内存区就是当前的内存管理所管理的模块，如果系统中还存在RAM虚拟盘时，主内存区前段还要扣除虚拟盘所占的内存空间

分页管理方式。利用页目录和页表结构处理对内存的申请和释放操作

page.s仅包含内存页异常的中断处理过程，对缺页和页写保护的处理

```bash
.globl page_fault

page_fault:
	xchgl %eax,(%esp)
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	push %fs
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	mov %dx,%fs
	movl %cr2,%edx
	pushl %edx
	pushl %eax
	testl $1,%eax
	jne 1f
	call do_no_page
	jmp 2f
1:	call do_wp_page
2:	addl $8,%esp
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret
```

## 寻址

在0.11版本中，所有进程都是用一个页目录表，而每个进程都有自己的页表。

经过分段处理，内核代码和数据段长度是16MB，使用了4个页表，且直接位于页目录表后面，然后再经过分页机制变换，被直接一一对应的映射到16MB的物理内存上。

为了使用分页机制，一个32位的线性地址被分为三个部分，分别用来指定一个页目录项、一个页表项和对应物理内存页上的偏移地址

<img src="./docs/image/页表项结构.jpg"/>

- 页框地址：指定一页内存的物理起始地址
- 存在位P: 确定一个页表项是否可以用于地址转换（如果为0且发出地址转换，则会发出缺页中断异常）
- 已访问A
- 已修改D
- 读/写位
- 用户/超级用户位

<img src="./docs/image/地址转换.jpg"/>

一个系统中可以同时存在多个页目录表，而在某个时刻只有一个页目录表可用(由CPU寄存器CR3来确定)

## 写时拷贝

当进程A fork创建出一个子进程B时，B会直接以只读的方式共享进程A的物理页面，只有当A或者B需要执行写操作的时候，会产生int14中断，然后使用do_wp_page来尝试解决这个问题

对产生中断的这个页面取消共享，并且为写进程复制一新的物理页面，然后从异常处理函数中返回，就会重新执行写入操作指令
