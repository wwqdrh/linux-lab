/*
 *  linux/kernel/asm.s
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * asm.s contains the low-level code for most hardware faults.
 * page_exception is handled by the mm, so that isn't here. This
 * file also handles (hopefully) fpu-exceptions due to TS-bit, as
 * the fpu must be properly saved/resored. This hasn't been tested.
 */

# 下面的全局函数名的实现在traps.c中
.globl divide_error,debug,nmi,int3,overflow,bounds,invalid_op
.globl double_fault,coprocessor_segment_overrun
.globl invalid_TSS,segment_not_present,stack_segment
.globl general_protection,coprocessor_error,irq13,reserved

# 在执行DIV或IDIV指令时，如果除数是零，CPU会产生这个异常
# 当EAX(或AX、AL)容纳不了一个合法除操作的时候会产生这个异常
divide_error:
	pushl $do_divide_error # 将要调用的函数地址入栈
no_error_code:
	xchgl %eax,(%esp)
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %edi
	pushl %esi
	pushl %ebp
	push %ds # 16位的寄存器入栈后也要占用4个字节
	push %es
	push %fs
	pushl $0		# 将数值0作为出错码入栈
	lea 44(%esp),%edx # 取堆栈中原调用返回地址处堆栈指针位置，并压入堆栈
	pushl %edx
	movl $0x10,%edx # 初始化段寄存器ds、es、fs，加载内核数据段选择符
	mov %dx,%ds
	mov %dx,%es
	mov %dx,%fs
	call *%eax # *号表示调用操作数指定地址处的函数，称为间接调用
	addl $8,%esp # 将堆栈指针加8相当于执行两次pop操作，弹出最后入堆栈的两个C函数参数
	pop %fs
	pop %es
	pop %ds
	popl %ebp
	popl %esi
	popl %edi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	iret

# int1
debug:
	pushl $do_int3		# _do_debug
	jmp no_error_code

# int2 非屏蔽中断调用入口点
nmi:
	pushl $do_nmi
	jmp no_error_code

# in3 断点指令引起中断的入口点
int3:
	pushl $do_int3
	jmp no_error_code

# int4，检查溢出
overflow:
	pushl $do_overflow
	jmp no_error_code

# int5（3个操作数，检查第一个是否在另外两个之间），边界检查出错
bounds:
	pushl $do_bounds
	jmp no_error_code

# int6 无效操作指令
invalid_op:
	pushl $do_invalid_op
	jmp no_error_code

# int9 协处理器段超出
coprocessor_segment_overrun:
	pushl $do_coprocessor_segment_overrun
	jmp no_error_code

# int15 保留中断
reserved:
	pushl $do_reserved
	jmp no_error_code

# int45 数学协处理器硬件中断
irq13:
	pushl %eax
	xorb %al,%al
	outb %al,$0xF0
	movb $0x20,%al
	outb %al,$0x20 # 向8259主中断控制芯片发送EOI(中断结束)信号
	jmp 1f # 这两个跳转指令起延时作用
1:	jmp 1f
1:	outb %al,$0xA0 # 向8259从中断控制芯片发送EOI(中断结束)信号
	popl %eax
	jmp coprocessor_error

# int8 双出错故障
double_fault:
	pushl $do_double_fault
error_code:
	xchgl %eax,4(%esp)		# 保存eax的值到堆栈上
	xchgl %ebx,(%esp)		# 保存ebx的值到堆栈上
	pushl %ecx
	pushl %edx
	pushl %edi
	pushl %esi
	pushl %ebp
	push %ds
	push %es
	push %fs
	pushl %eax			# 出错号入栈
	lea 44(%esp),%eax		# offset
	pushl %eax
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	call *%ebx
	addl $8,%esp
	pop %fs
	pop %es
	pop %ds
	popl %ebp
	popl %esi
	popl %edi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	iret

# int10 无效的任务状态段
invalid_TSS:
	pushl $do_invalid_TSS
	jmp error_code

# int11 段不存在
segment_not_present:
	pushl $do_segment_not_present
	jmp error_code

# int12 堆栈段错误
stack_segment:
	pushl $do_stack_segment
	jmp error_code

# int13 一般保护性出错，缺省异常我在这处理
general_protection:
	pushl $do_general_protection
	jmp error_code

