# 内存管理

## kmalloc

提供内核空间的动态内存分配功能，代替直接调用get_free_page等接口的方式，大大简化内核程序中的内存申请

`使用场景`

- 内核数据结构的动态创建,如信号量、队列等
- 设备驱动程序的内存申请
- 临时性数据结构或缓冲区的分配

`实现原理`

kmalloc数组指向不同大小的内存池，内存池通过linked list组织空闲内存块

定义kmalloc数组指向每个大小分配的内存池, 使用宏SWITCH_ADDRESS_SPACE切换到内核地址空间

根据大小调用get_free_pages或get_free_page分配

用完后调用free_pages或free_page释放

## vmalloc

为内核提供可以分配大于最大物理内存的连续虚拟内存区域，通过页表映射实现虚拟连续,实际物理不连续

`使用场景`

- 大规模内核数据结构的动态创建,大小超过物理内存
- 需要连续虚拟地址的大内存块
- 映射硬件设备的内存到内核虚拟地址空间

`实现原理`

VMALLOC_RESERVE 记录vmalloc区信息。vm_struct 描述一块vmlloc区。vmap_area 全局的vmalloc区域。

- 定义全局描述符VMALLOC_RESERVE记录vmalloc区信息
- 初始化时根据物理内存调整vmalloc区大小
- vmalloc函数申请地址,获取空闲页并更新页表
- vfree函数释放地址,回收页表和页面