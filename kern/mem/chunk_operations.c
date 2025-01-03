/*
 * chunk_operations.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include <kern/trap/fault_handler.h>
#include <kern/disk/pagefile_manager.h>
#include <kern/proc/user_environment.h>
#include "kheap.h"
#include "memory_manager.h"
#include <inc/queue.h>

//extern void inctst();

/******************************/
/*[1] RAM CHUNKS MANIPULATION */
/******************************/

//===============================
// 1) CUT-PASTE PAGES IN RAM:
//===============================
//This function should cut-paste the given number of pages from source_va to dest_va on the given page_directory
//	If the page table at any destination page in the range is not exist, it should create it
//	If ANY of the destination pages exists, deny the entire process and return -1. Otherwise, cut-paste the number of pages and return 0
//	ALL 12 permission bits of the destination should be TYPICAL to those of the source
//	The given addresses may be not aligned on 4 KB
int cut_paste_pages(uint32* page_directory, uint32 source_va, uint32 dest_va,
		uint32 num_of_pages) {
	//[PROJECT] [CHUNK OPERATIONS] cut_paste_pages
	// Write your code here, remove the panic and write your code
	panic("cut_paste_pages() is not implemented yet...!!");
}

//===============================
// 2) COPY-PASTE RANGE IN RAM:
//===============================
//This function should copy-paste the given size from source_va to dest_va on the given page_directory
//	Ranges DO NOT overlapped.
//	If ANY of the destination pages exists with READ ONLY permission, deny the entire process and return -1.
//	If the page table at any destination page in the range is not exist, it should create it
//	If ANY of the destination pages doesn't exist, create it with the following permissions then copy.
//	Otherwise, just copy!
//		1. WRITABLE permission
//		2. USER/SUPERVISOR permission must be SAME as the one of the source
//	The given range(s) may be not aligned on 4 KB
int copy_paste_chunk(uint32* page_directory, uint32 source_va, uint32 dest_va,
		uint32 size) {
	//[PROJECT] [CHUNK OPERATIONS] copy_paste_chunk
	// Write your code here, remove the //panic and write your code
	panic("copy_paste_chunk() is not implemented yet...!!");
}

//===============================
// 3) SHARE RANGE IN RAM:
//===============================
//This function should copy-paste the given size from source_va to dest_va on the given page_directory
//	Ranges DO NOT overlapped.
//	It should set the permissions of the second range by the given perms
//	If ANY of the destination pages exists, deny the entire process and return -1. Otherwise, share the required range and return 0
//	If the page table at any destination page in the range is not exist, it should create it
//	The given range(s) may be not aligned on 4 KB
int share_chunk(uint32* page_directory, uint32 source_va, uint32 dest_va,
		uint32 size, uint32 perms) {
	//[PROJECT] [CHUNK OPERATIONS] share_chunk
	// Write your code here, remove the //panic and write your code
	panic("share_chunk() is not implemented yet...!!");
}

//===============================
// 4) ALLOCATE CHUNK IN RAM:
//===============================
//This function should allocate the given virtual range [<va>, <va> + <size>) in the given address space  <page_directory> with the given permissions <perms>.
//	If ANY of the destination pages exists, deny the entire process and return -1. Otherwise, allocate the required range and return 0
//	If the page table at any destination page in the range is not exist, it should create it
//	Allocation should be aligned on page boundary. However, the given range may be not aligned.
int allocate_chunk(uint32* page_directory, uint32 va, uint32 size, uint32 perms) {
	//[PROJECT] [CHUNK OPERATIONS] allocate_chunk
	// Write your code here, remove the //panic and write your code
	panic("allocate_chunk() is not implemented yet...!!");
}

//=====================================
// 5) CALCULATE ALLOCATED SPACE IN RAM:
//=====================================
void calculate_allocated_space(uint32* page_directory, uint32 sva, uint32 eva,
		uint32 *num_tables, uint32 *num_pages) {
	//[PROJECT] [CHUNK OPERATIONS] calculate_allocated_space
	// Write your code here, remove the panic and write your code
	panic("calculate_allocated_space() is not implemented yet...!!");
}

//=====================================
// 6) CALCULATE REQUIRED FRAMES IN RAM:
//=====================================
//This function should calculate the required number of pages for allocating and mapping the given range [start va, start va + size) (either for the pages themselves or for the page tables required for mapping)
//	Pages and/or page tables that are already exist in the range SHOULD NOT be counted.
//	The given range(s) may be not aligned on 4 KB
uint32 calculate_required_frames(uint32* page_directory, uint32 sva,
		uint32 size) {
	//[PROJECT] [CHUNK OPERATIONS] calculate_required_frames
	// Write your code here, remove the panic and write your code
	panic("calculate_required_frames() is not implemented yet...!!");
}

//=================================================================================//
//===========================END RAM CHUNKS MANIPULATION ==========================//
//=================================================================================//

/*******************************/
/*[2] USER CHUNKS MANIPULATION */
/*******************************/

//======================================================
/// functions used for USER HEAP (malloc, free, ...)
//======================================================
//=====================================
/* DYNAMIC ALLOCATOR SYSTEM CALLS */
//=====================================
void* sys_sbrk(int numOfPages) {
	/* numOfPages > 0: move the segment break of the current user program to increase the size of its heap
	 * 				by the given number of pages. You should allocate NOTHING,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * numOfPages = 0: just return the current position of the segment break
	 *
	 * NOTES:
	 * 	1) As in real OS, allocate pages lazily. While sbrk moves the segment break, pages are not allocated
	 * 		until the user program actually tries to access data in its heap (i.e. will be allocated via the fault handler).
	 * 	2) Allocating additional pages for a process� heap will fail if, for example, the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sys_sbrk fails, the net effect should
	 * 		be that sys_sbrk returns (void*) -1 and that the segment break and the process heap are unaffected.
	 * 		You might have to undo any operations you have done so far in this case.
	 */

	//TODO: [PROJECT'24.MS2 - #11] [3] USER HEAP - sys_sbrk
	/*====================================*/
	/*Remove this line before start coding*/
	//return (void*)-1 ;
	/*====================================*/


	struct Env* env = get_cpu_proc();
	//env_free(env);
//	cprintf("finish");
//	return (void*)-1;

	if (numOfPages == 0) {
		return (void*) env->brk;
	}

	uint32 sizeToAllocate = numOfPages * PAGE_SIZE;

	if (env->brk + sizeToAllocate > env->end) {
//		cprintf("in sys sbrk exceed hard limit \n");
//		int x=-1;
//		cprintf("the value is %x\n", (void *)(unsigned int)(-1));
//		cprintf("the value is %p\n", (void *)(long)(-1));

		return ((void*)-1);

	}

	uint32 oldBrk = env->brk;
	env->brk += sizeToAllocate;

//
//	cprintf("old brk %p\n",oldBrk);
//	cprintf("new brk %p\n",env->brk);
//	cprintf("sizeToAllocate %d\n",sizeToAllocate);
	allocate_user_mem(env,oldBrk,sizeToAllocate);


	return (void*) oldBrk;
}


struct ws_info {
    uint32 virtual_address;
};

struct ws_info ws_table[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE];



//=====================================
// 1) ALLOCATE USER MEMORY:
//=====================================
void allocate_user_mem(struct Env* e, uint32 virtual_address, uint32 size) {
	/*====================================/
	 /Remove this line before start coding/
	 //	inctst();
	 //	return;
	 /====================================*/

	//TODO: [PROJECT'24.MS2 - #13] [3] USER HEAP [KERNEL SIDE] - allocate_user_mem()
	// Write your code here, remove the panic and write your code
	//panic("allocate_user_mem() is not implemented yet...!!");
//	cprintf("in allocate_user_mem\n");
	uint32 perm_mark = 0x400;

	uint32 numOfPages = ROUNDUP(size,PAGE_SIZE) / PAGE_SIZE;
//	cprintf("sizeToAllocate %d\n",numOfPages);
	for (uint32 i = 0; i < numOfPages; i++) {
		uint32 currAddress = virtual_address + i * PAGE_SIZE;
		uint32 *page_table;

		int x = get_page_table(e->env_page_directory, currAddress, &page_table);

		if (x == TABLE_NOT_EXIST) {

			page_table = (uint32 *) create_page_table(e->env_page_directory,
					currAddress);

		}

		page_table[PTX(virtual_address+i*PAGE_SIZE)] = page_table[PTX(
				currAddress)] | (perm_mark);

	}
	/*uint32 q=0x800fe000;
	 if (e->brk == q) {
	 for (uint32 i = 0; i < numOfPages; i++) {
	 uint32 *page_table;

	 int x = get_page_table(e->env_page_directory,
	 virtual_address + i * PAGE_SIZE, &page_table);

	 cprintf("addd %p\n", virtual_address + i * PAGE_SIZE);

	 cprintf("entryyy %d \n",
	 page_table[PTX(virtual_address+i*PAGE_SIZE)]);

	 }
	 }*/
}
//=====================================
// 2) FREE USER MEMORY:
//=====================================

void free_user_mem(struct Env* e, uint32 virtual_address, uint32 size) {
	/*====================================/
	 /Remove this line before start coding/
	 //	inctst();
	 //	return;
	 /====================================*/

	//TODO: [PROJECT'24.MS2 - #15] [3] USER HEAP [KERNEL SIDE] - free_user_mem
	// Write your code here, remove the panic and write your code
	//panic("free_user_mem() is not implemented yet...!!");

//	cprintf("the size of ws is %d\n", LIST_SIZE(&e->page_WS_list));
//	env_page_ws_print(e);
//	e->page_last_WS_element = LIST_LAST(&e->page_WS_list);
//	for (int i = 0; i <= 270; i++) {
//		e->page_last_WS_element = LIST_NEXT(e->page_last_WS_element);
//	}

//	env_page_ws_print(e);
	/// must removeeee

	uint32 pages_num = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	uint32 perm_mark = 0x400;
	uint32 bool = 0, bool2 = 0;
	if (e->page_last_WS_element != NULL) {
		bool = 1;
	}

	for (int i = 0; i < pages_num; ++i) {
		uint32 *page_table;
		int x = get_page_table(e->env_page_directory, virtual_address,
				&page_table);
		if (page_table != NULL) {
			page_table[PTX(virtual_address)] = page_table[PTX(virtual_address)]
					& (~perm_mark);
			//virtual_address = virtual_address & (~perm_mark);
			pf_remove_env_page(e, virtual_address);
			struct WorkingSetElement* w;
			LIST_FOREACH(w,&(e->page_WS_list))
			{
				if (ROUNDDOWN(w->virtual_address,
						PAGE_SIZE) == ROUNDDOWN(virtual_address,PAGE_SIZE)) {
					if (w == e->page_last_WS_element) {
						if (w == LIST_LAST(&e->page_WS_list)) {
							e->page_last_WS_element = LIST_FIRST(
									&e->page_WS_list);
						} else {
							e->page_last_WS_element = LIST_NEXT(w);
						}
					}
					env_page_ws_invalidate(e, virtual_address);
					bool2 = 1;
				}
			}

			virtual_address += PAGE_SIZE;
		}
	}
//	env_page_ws_invalidate(e, 0x82d087fe);

//	for (uint32 i = 0; i < pages_num; ++i) {
//		uint32 current_va = virtual_address + i * PAGE_SIZE;
//
//		uint32 *page_table;
//		int ret = get_page_table(e->env_page_directory, current_va,
//				&page_table);
//		if (ret == TABLE_NOT_EXIST || page_table == NULL) {
//			continue;
//		}
//
//		page_table[PTX(current_va)] &= (~perm_mark);
//
//		pf_remove_env_page(e, current_va);
////		uint32 index = (current_va - USER_HEAP_START) / PAGE_SIZE;
////		if (ws_table[index].virtual_address == current_va) {
////
////			unmap_frame(e->env_page_directory, current_va);
////
////			env_page_ws_invalidate(e, current_va);
////
////			ws_table[index].virtual_address = 0;
////		}
//
//		struct WorkingSetElement *w = NULL;
//		LIST_FOREACH(w,&e->page_WS_list)
//		{
//			if (w->virtual_address == current_va) {
//				cprintf("------------------------------------1\n\n");
//				if (w == e->page_last_WS_element) {
//					if (w == LIST_LAST(&e->page_WS_list)) {
//						e->page_last_WS_element = LIST_FIRST(&e->page_WS_list);
//					} else {
//						e->page_last_WS_element = LIST_NEXT(w);
//					}
//				}
//				env_page_ws_invalidate(e, current_va);
//				bool2 = 1;
//			}
//		}
//	}
	if (bool && bool2) {
		while (LIST_FIRST(&e->page_WS_list) != e->page_last_WS_element) {
			struct WorkingSetElement* w = LIST_FIRST(&e->page_WS_list);

			LIST_REMOVE(&e->page_WS_list, w);

			LIST_INSERT_TAIL(&e->page_WS_list, w);

//		    env_page_ws_print(e);
		}

		e->page_last_WS_element=NULL;

	}
//	env_page_ws_print(e);
}


//=====================================
// 2) FREE USER MEMORY (BUFFERING):
//=====================================
void __free_user_mem_with_buffering(struct Env* e, uint32 virtual_address,
		uint32 size) {
	// your code is here, remove the panic and write your code
	panic("__free_user_mem_with_buffering() is not implemented yet...!!");
}

//=====================================
// 3) MOVE USER MEMORY:
//=====================================
void move_user_mem(struct Env* e, uint32 src_virtual_address,
		uint32 dst_virtual_address, uint32 size) {
	//[PROJECT] [USER HEAP - KERNEL SIDE] move_user_mem
	//your code is here, remove the panic and write your code
	panic("move_user_mem() is not implemented yet...!!");
}

//=================================================================================//
//========================== END USER CHUNKS MANIPULATION =========================//
//=================================================================================//

