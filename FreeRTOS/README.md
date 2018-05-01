# FreeRTOS (V8.2.1) port for the nRF51 MCUs using the GCC ARM Toolchain

Please comment, ask questions, provide patches etc. mainly in the FreeRTOS forum as instructed in
 http://www.freertos.org/RTOS-contributed-ports.html

 
 
 
1. Memory Usage 
FreeRTOS has 5 implematations [heap_1, heap_2, heap_3, heap_4, heap_5]

Following below:
heap_1 - the very simplest, does not permit memory to be freed
heap_2 - permits memory to be freed, but not does coalescence adjacent free blocks.
heap_3 - simply wraps the standard malloc() and free() for thread safety
heap_4 - coalescences adjacent free blocks to avoid fragmentation. Includes absolute address placement option
heap_5 - as per heap_4, with the ability to span the heap across multiple non-adjacent memory areas


We apply heap_4.c

This scheme uses a first fit algorithm and, unlike scheme 2, it does combine adjacent free memory blocks into a single large block (it does include a coalescence algorithm).
The total amount of available heap space is set by configTOTAL_HEAP_SIZE - which is defined in FreeRTOSConfig.h. The configAPPLICATION_ALLOCATED_HEAP FreeRTOSConfig.h configuration constant is provided to allow the heap to be placed at a specific address in memory.

The xPortGetFreeHeapSize() API function returns the total amount of heap space that remains unallocated when the function is called, 
and the xPortGetMinimumEverFreeHeapSize() API function returns lowest amount of free heap space that has existed system the FreeRTOS application booted. 
Neither function provides information on how the unallocated memory is fragmented into smaller blocks.

This implementation:

Can be used even when the application repeatedly deletes tasks, queues, semaphores, mutexes, etc..
Is much less likely than the heap_2 implementation to result in a heap space that is badly fragmented into multiple small blocks - even when the memory being allocated and freed is of random size.
Is not deterministic - but is much more efficient that most standard C library malloc implementations.
heap_4.c is particularly useful for applications that want to use the portable layer memory allocation schemes directly in the application code (rather than just indirectly by calling API functions that themselves call pvPortMalloc() and vPortFree()).
