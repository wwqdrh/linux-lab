# 内存管理

将一些宏定义移动到了`include/linux/sched.h`->`include/linux/mm.h`中

相较于0.11，多了虚拟内存交换的功能

新增了put_dirty_page函数

show_mem

## swap

当物理内存不够的时候，将部分暂未使用的内存页换到swap分区，从而释放物理内存以供其他进程使用。

在启动初期会根据swap分区大小来初始化swap缓冲区和swap哈希表。

增加了swap_info结构体来记录swap区信息。
增加swap_map_handle结构来表示swap条目。
在页表项中多了一个SWP_TYPE字段来表示是否在swap区。

- swap_free：释放swap区页框
- swap_in：将页读入到内存
- swap_out：将内存页swap出到swap区
- init_swapping