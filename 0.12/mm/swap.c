/*
 *  linux/mm/swap.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * This file should contain most things doing the swapping from/to disk.
 * Started 18.12.91
 */

#include <string.h>

#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <linux/kernel.h>

#define SWAP_BITS (4096<<3)

#define bitop(name,op) \
static inline int name(char * addr,unsigned int nr) \
{ \
int __res; \
__asm__ __volatile__("bt" op " %1,%2; adcl $0,%0" \
:"=g" (__res) \
:"r" (nr),"m" (*(addr)),"0" (0)); \
return __res; \
}

bitop(bit,"")
bitop(setbit,"s")
bitop(clrbit,"r")

static char * swap_bitmap = NULL;
int SWAP_DEV = 0;

/*
 * We never page the pages in task[0] - kernel memory.
 * We page all other pages.
 */
#define FIRST_VM_PAGE (TASK_SIZE>>12)
#define LAST_VM_PAGE (1024*1024)
#define VM_PAGES (LAST_VM_PAGE - FIRST_VM_PAGE)

static int get_swap_page(void)
{
	int nr;

	if (!swap_bitmap)
		return 0;
	for (nr = 1; nr < 32768 ; nr++)
		if (clrbit(swap_bitmap,nr))
			return nr;
	return 0;
}

void swap_free(int swap_nr)
{
	if (!swap_nr)
		return;
	if (swap_bitmap && swap_nr < SWAP_BITS)
		if (!setbit(swap_bitmap,swap_nr))
			return;
	printk("Swap-space bad (swap_free())\n\r");
	return;
}

void swap_in(unsigned long *table_ptr)
{
	int swap_nr;
	unsigned long page;

	// 检查swap_bitmap是否初始化,如果没有则报错返回。
	if (!swap_bitmap) {
		printk("Trying to swap in without swap bit-map");
		return;
	}

	// 检查页表项的PRESENT位,如果页面已经在内存则报错返回。
	if (1 & *table_ptr) {
		printk("trying to swap in present page\n\r");
		return;
	}
	// 从页表项的高位获取swap区号swap_nr。
	swap_nr = *table_ptr >> 1;
	if (!swap_nr) {
		printk("No swap page in swap_in\n\r");
		return;
	}
	// 尝试从内存分配一页作为页面数据的存储。如果分配失败则内存不足,报错。
	if (!(page = get_free_page()))
		oom();
	// 根据swap区号,从swap区读取页面数据到分配的内存页上。
	read_swap_page(swap_nr, (char *) page);
	// 在swap位图中标记该swap区页框已经使用。如果已经被使用了则报错。
	if (setbit(swap_bitmap,swap_nr))
		printk("swapping in multiply from same page\n\r");
	// 更新页表项为内存入口,并设置脏位和访问权限。
	*table_ptr = page | (PAGE_DIRTY | 7);
}

// 主要检查页面是否在内核空间、是否正被占用、是否已经在swap区等,以确定是否可以swap出该页面。
int try_to_swap_out(unsigned long * table_ptr)
{
	unsigned long page;
	unsigned long swap_nr;

	// 获取页表项内容到page变量
	page = *table_ptr;

	// 检查PF_PRESENT标志是否设置,如果页不在内存则退出
	if (!(PAGE_PRESENT & page))
		return 0;
	// 检查页面是否在内核地址范围内,如果是则退出
	if (page - LOW_MEM > PAGING_MEMORY)
		return 0;
	// 如果页面被修改(PF_DIRTY set),则需要先写入swap区
	if (PAGE_DIRTY & page) {
		page &= 0xfffff000;
		if (mem_map[MAP_NR(page)] != 1)
			return 0;
		// 获取swap区未使用的页框号swap_nr，如果获取失败则退出
		if (!(swap_nr = get_swap_page()))
			return 0;
		// 更新页表项地址为swap入口,写入swap号和脏位
		*table_ptr = swap_nr<<1;
		// 刷新TLB
		invalidate();
		// 将页面数据写入swap区
		write_swap_page(swap_nr, (char *) page);
		// 释放原内存页框
		free_page(page);
		return 1;
	}
	// 对于未修改的页,直接释放内存页框,更新页表项无效
	*table_ptr = 0;
	invalidate();
	free_page(page);
	return 1;
}

/*
 * Ok, this has a rather intricate logic - the idea is to make good
 * and fast machine code. If we didn't worry about that, things would
 * be easier.
 */
int swap_out(void)
{
	// 通过dir_entry和page_entry两个索引遍历整个线性地址空间的页表。
	static int dir_entry = FIRST_VM_PAGE>>10;
	static int page_entry = -1;
	int counter = VM_PAGES;
	int pg_table;

	while (counter>0) {
		pg_table = pg_dir[dir_entry];
		if (pg_table & 1)
			break;
		counter -= 1024;
		dir_entry++;
		if (dir_entry >= 1024)
			dir_entry = FIRST_VM_PAGE>>10;
	}
	pg_table &= 0xfffff000;
	while (counter-- > 0) {
		page_entry++;
		if (page_entry >= 1024) {
			page_entry = 0;
		repeat:
			dir_entry++;
			if (dir_entry >= 1024)
				dir_entry = FIRST_VM_PAGE>>10;
			pg_table = pg_dir[dir_entry];
			if (!(pg_table&1))
				if ((counter -= 1024) > 0)
					goto repeat;
				else
					break;
			pg_table &= 0xfffff000;
		}
		// try_to_swap_out函数检查指定页表项是否可swap出,如果可以则执行swap out并返回1。
		if (try_to_swap_out(page_entry + (unsigned long *) pg_table))
			return 1;
	}
	// 如果整个地址空间的页面都不可swap出,则打印出错并返回0。
	printk("Out of swap-memory\n\r");
	return 0;
}

/*
 * Get physical address of first (actually last :-) free page, and mark it
 * used. If no free pages left, return 0.
 */
unsigned long get_free_page(void)
{
register unsigned long __res asm("ax");

repeat:
	__asm__("std ; repne ; scasb\n\t"
		"jne 1f\n\t"
		"movb $1,1(%%edi)\n\t"
		"sall $12,%%ecx\n\t"
		"addl %2,%%ecx\n\t"
		"movl %%ecx,%%edx\n\t"
		"movl $1024,%%ecx\n\t"
		"leal 4092(%%edx),%%edi\n\t"
		"rep ; stosl\n\t"
		"movl %%edx,%%eax\n"
		"1:"
		:"=a" (__res)
		:"0" (0),"i" (LOW_MEM),"c" (PAGING_PAGES),
		"D" (mem_map+PAGING_PAGES-1)
		:"di","cx","dx");
	if (__res >= HIGH_MEMORY)
		goto repeat;
	if (!__res && swap_out())
		goto repeat;
	return __res;
}

// 对swap设备进行完整性检查,并初始化了关键的数据结构swap_bitmap,为后续的swap出入提供支持
void init_swapping(void)
{
	extern int *blk_size[];
	int swap_size,i,j;

	// 检查是否定义了SWAP_DEV,如果没有则直接返回。SWAP_DEV指定了swap所使用的设备
	if (!SWAP_DEV)
		return;
	
	// 通过blk_size数组获取SWAP_DEV设备的大小,如果获取失败则返回。
	if (!blk_size[MAJOR(SWAP_DEV)]) {
		printk("Unable to get size of swap device\n\r");
		return;
	}

	// 计算出swap区的页框数量swap_size,如果太小则打印警告并返回。
	swap_size = blk_size[MAJOR(SWAP_DEV)][MINOR(SWAP_DEV)];
	if (!swap_size)
		return;
	if (swap_size < 100) {
		printk("Swap device too small (%d blocks)\n\r",swap_size);
		return;
	}

	// 分配一页内存用于表示swap位图swap_bitmap。如果分配失败则返回。
	swap_size >>= 2;
	if (swap_size > SWAP_BITS)
		swap_size = SWAP_BITS;
	swap_bitmap = (char *) get_free_page();
	if (!swap_bitmap) {
		printk("Unable to start swapping: out of memory :-)\n\r");
		return;
	}

	// 读取swap区的第一个页到swap_bitmap中,并检查签名"SWAP-SPACE"是否存在。如果不存在则释放位图内存并返回。
	read_swap_page(0,swap_bitmap);
	if (strncmp("SWAP-SPACE",swap_bitmap+4086,10)) {
		printk("Unable to find swap-space signature\n\r");
		free_page((long) swap_bitmap);
		swap_bitmap = NULL;
		return;
	}

	// 清零签名字段,并检查位图的每个比特位是否为1。如果是1表示对应的页框被使用了,有问题则释放位图内存并返回。
	memset(swap_bitmap+4086,0,10);
	for (i = 0 ; i < SWAP_BITS ; i++) {
		if (i == 1)
			i = swap_size;
		if (bit(swap_bitmap,i)) {
			printk("Bad swap-space bit-map\n\r");
			free_page((long) swap_bitmap);
			swap_bitmap = NULL;
			return;
		}
	}

	// 统计总共有多少页框可用于swap,即比特位为0的页框数量。如果没有任何可用页框则释放位图内存并返回。
	j = 0;
	for (i = 1 ; i < swap_size ; i++)
		if (bit(swap_bitmap,i))
			j++;
	if (!j) {
		free_page((long) swap_bitmap);
		swap_bitmap = NULL;
		return;
	}

	// 到此swap区和位图检查通过,打印有多少页框可用于swap。
	printk("Swap device ok: %d pages (%d bytes) swap-space\n\r",j,j*4096);
}
