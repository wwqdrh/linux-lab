# 整体流程

当用户空间执行fork函数时，会发生系统调用，对应的汇编代码如下

```s
.align 2
sys_fork:
	call find_empty_process
	testl %eax,%eax
	js 1f
	push %gs
	pushl %esi
	pushl %edi
	pushl %ebp
	pushl %eax
	call copy_process
	addl $20,%esp
1:	ret
```

首先执行find_empty_process寻找是否存在一个空闲的任务结构，如果存在，则将gs、esi、edi、ebp、eax寄存器的值压入调用栈，并且调用copy_process, 当执行完了之后，将esp栈顶指针往高地址移动20，即将前面压入栈中的参数弹出，回到函数调用之前

# 寻找空的任务

代码如下，主要就是在task列表中找到一个空的位置，并将该位置的索引返回出去

> NR_TASKS定义了最大的任务数为64

```c
int find_empty_process(void)
{
	int i;

	repeat:
		if ((++last_pid)<0) last_pid=1;
		for(i=0 ; i<NR_TASKS ; i++)
			if (task[i] && task[i]->pid == last_pid) goto repeat;
	for(i=1 ; i<NR_TASKS ; i++)
		if (!task[i])
			return i;
	return -EAGAIN;
}
```

# 复制任务

首先看copy_process的函数签名如下

```c
int copy_process(int nr,long ebp,long edi,long esi,long gs,long none,
		long ebx,long ecx,long edx,
		long fs,long es,long ds,
		long eip,long cs,long eflags,long esp,long ss)
```

结合前面的sys_fork汇编代码，会有一个疑惑，为什么sys_fork只压入了5个参数，而这里有一大堆呢

原因在于在进入系统调用, 触发了中断后，CPU 会自动帮我们做如下压栈操作, 并且system_call自动压入了一些参数

- ss：堆栈段
- esp：栈顶指针
- eflags：标志位
- cs：代码段
- eip：指令寄存器
- ds：数据段
- es：额外扩展段
- fs：栈帧段
- edx：数据寄存器
- ecx：数据寄存器
- ebx：数据寄存器
- eax：数据寄存器

在copy_process函数中主要的工作就是下面的部分，分配一个task_struct并复制一些状态信息，并且将该进程的tss信息以及ldt表存储在gdt对应的位置，方便通过gdt寻找这个进程的相关信息

```c
struct task_struct *p;
p = (struct task_struct *) get_free_page();
*p = *current;	/* NOTE! this doesn't copy the supervisor stack */
p->state = TASK_UNINTERRUPTIBLE;
// ...复制一些状态信息
set_tss_desc(gdt+(nr<<1)+FIRST_TSS_ENTRY,&(p->tss));
set_ldt_desc(gdt+(nr<<1)+FIRST_LDT_ENTRY,&(p->ldt));
```