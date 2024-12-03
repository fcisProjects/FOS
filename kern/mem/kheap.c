#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
//All pages in the given range should be allocated
//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
//Return:
//	On success: 0
//	Otherwise (if no memory OR initial size exceed the given limit): PANIC
int initialize_kheap_dynamic_allocator(uint32 daStart,
		uint32 initSizeToAllocate, uint32 daLimit) {
	//TODO: [PROJECT'24.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator
	// Write your code here, remove the panic and write your code
	//panic("initialize_kheap_dynamic_allocator() is not implemented yet...!!");
	if ((daStart + initSizeToAllocate) > daLimit
			|| (daLimit - daStart) < initSizeToAllocate) {
		panic("no memory or initial size exceed the given limit");
	}
	start = daStart;
	brk = daStart + initSizeToAllocate;
	end = daLimit;

	int numOfPages = ROUNDUP (initSizeToAllocate, PAGE_SIZE) / PAGE_SIZE;
	int numOfFrams = (KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE;

	for (int i = 0; i < numOfPages; ++i) {
		uint32 currAddress = daStart + i * PAGE_SIZE;
		struct FrameInfo *frameInfo = NULL;
		int res = allocate_frame(&frameInfo);
		if (res == E_NO_MEM) {
			panic("Failed to allocate frame");
		}
		frameInfo->va = currAddress;
		frames_info->numOfPages = numOfPages;
		uint32 *page_table = NULL;
		int x = get_page_table(ptr_page_directory, currAddress, &page_table);
		if (page_table == NULL) {
			create_page_table(ptr_page_directory, currAddress);
		}
		map_frame(ptr_page_directory, frameInfo, currAddress,
		PERM_WRITEABLE | PERM_PRESENT);
	}

	initialize_dynamic_allocator(daStart, initSizeToAllocate);

	return 0;

}

void* sbrk(int numOfPages) {
	/* numOfPages > 0: move the segment break of the kernel to increase the size of its heap by the given numOfPages,
	 * 				you should allocate pages and map them into the kernel virtual address space,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * numOfPages = 0: just return the current position of the segment break
	 *
	 * NOTES:
	 * 	1) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, kernel should panic(...)
	 */

	//MS2: COMMENT THIS LINE BEFORE START CODING====
	//return (void*)-1 ;
	//====================================================
	//[PROJECT'24.MS2] Implement this function
	// Write your code here, remove the panic and write your code
	//panic("sbrk() is not implemented yet...!!");

	if (numOfPages == 0) {
		//cprintf("0\n");
		return (void*) brk;
	}

	uint32 sizeToAllocate = numOfPages * PAGE_SIZE;

	if (brk + sizeToAllocate > end) {
		//cprintf("1\n");
		return (void*) -1;
	}

	//cprintf("2\n");
	uint32 oldBrk = brk;
	brk += sizeToAllocate;

	for (int i = 0; i < numOfPages; i++) {

		uint32 currAddress = oldBrk + i * PAGE_SIZE;

		struct FrameInfo *frameInfo;

		int res = allocate_frame(&frameInfo);
		//cprintf("3\n");
		if (res != 0) {
			panic("Failed to allocate frame in sbrk");
		}

		uint32* page_table;
		if (get_page_table(ptr_page_directory, currAddress,
				&page_table) == TABLE_NOT_EXIST) {
			create_page_table(ptr_page_directory, currAddress);
		}

		map_frame(ptr_page_directory, frameInfo, currAddress,
		PERM_WRITEABLE | PERM_PRESENT);
		frameInfo->va = currAddress;
		//cprintf("4\n");
	}

	uint32*endblock_ptr = (void*) brk - sizeof(int);
	*endblock_ptr = 1;
	uint32*oldendblock_ptr = (void*) oldBrk - sizeof(int);
	*oldendblock_ptr = 0;
	//cprintf("5\n");
	return (void*) oldBrk;
}

//TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator

void* kmalloc(unsigned int size) {
//[PROJECT'24.MS2] Implement this function
// Write your code here, remove the panic and write your code
//kpanic_into_prompt("kmalloc() is not implemented yet...!!");

// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy
	isKHeapPlacementStrategyFIRSTFIT();

	//cprintf("in kmalloc\n");

	if (size > DYN_ALLOC_MAX_SIZE) {
		return NULL;
	} else if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
//		cprintf("will alloc in dynamic\n");
		return alloc_block_FF(size);
	}
	uint32 numPages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	//cprintf("num of pages %d\n", numPages);
	uint32 baseVA = end + PAGE_SIZE;
	uint32 allocatedVA = 0;
	uint32 bool = 0;
	int counter = 0;
	uint32 start_add;
	uint32 iterator;
	for (uint32 va = baseVA; va < KERNEL_HEAP_MAX; va += PAGE_SIZE) {

		iterator = va;
		for (int i = 0; i < numPages; ++i) {
			uint32* ptr_table = NULL;
			struct FrameInfo* frame = get_frame_info(ptr_page_directory,
					iterator, &ptr_table);
			if (frame == NULL) {	// free frame
				counter++;
			} else {
				va = iterator;
				counter = 0;
				break;
			}
			iterator = iterator + PAGE_SIZE;
		}

		if (counter == numPages) {
			start_add = va;
			allocatedVA = va;
			break;
		}

	}
	//cprintf("find pages to allocate \n");
	//for (uint32 va = start_add; va < KERNEL_HEAP_MAX; va += PAGE_SIZE){

	if (counter == numPages) {
//		cprintf(
//				" the num of the pages in kamlloc    %d  --------------------------> \n",
//				numPages);

		for (int x = 0; x < numPages; ++x) {
			struct FrameInfo* new_frame;
			int allocResult = allocate_frame(&new_frame);

			if (allocResult != 0) {
				cprintf("kmalloc: Out of memory during allocation\n");
				return NULL;
			}

			int mapResult = map_frame(ptr_page_directory, new_frame, start_add,
			PERM_USED | PERM_WRITEABLE | PERM_PRESENT);

			new_frame->va = start_add;

//			cprintf(
//					" the index in kmalloc allocated %d -------------- --------------------------> \n",
//					x);

			if (mapResult != 0) {
//				cprintf("kmalloc: Failed to map frame at VA %x\n", start_add);
				return NULL;
			}
			start_add = start_add + PAGE_SIZE;
		}
		//	cprintf("kmalloc done\n");
		uint32* ptr_table = NULL;
		struct FrameInfo* new_frame = get_frame_info(ptr_page_directory,
				allocatedVA, &ptr_table);
		new_frame->va = allocatedVA;
		new_frame->numOfPages = numPages;
//		cprintf(" in kmalloc va %x\n",allocatedVA);
//		cprintf("p a %x\n",kheap_physical_address(allocatedVA));
		return (void*) allocatedVA;

	}
	return NULL;
}

void kfree(void* virtual_address) {
//	cprintf("in kfree\n");
//TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
// Write your code here, remove the panic and write your code
//panic("kfree() is not implemented yet...!!");
//cprintf("will free va %x",virtual_address);
	if (virtual_address == 0) {
		//cprintf("size is zero \n");
		return;
		free_block(virtual_address);
	}
	if (virtual_address >= (void*) KERNEL_HEAP_START
			&& virtual_address < (void*) end) {		// hard limit
			//	cprintf("will call free block allocator \n");
		free_block(virtual_address);
	}

	else if (virtual_address >= (void*) end + PAGE_SIZE
			&& virtual_address < (void*) KERNEL_HEAP_MAX) {
		//	cprintf("in page free\n");

		uint32* ptr_table = NULL;
		struct FrameInfo* new_frame = get_frame_info(ptr_page_directory,
				(uint32) virtual_address, &ptr_table);
		int numOfPages = new_frame->numOfPages;
		for (int i = 0; i < numOfPages; i++) {
			uint32* ptr_table = NULL;
			struct FrameInfo* new_frame = get_frame_info(ptr_page_directory,
					(uint32) virtual_address + i * PAGE_SIZE, &ptr_table);
			unmap_frame(ptr_page_directory,
					(uint32) virtual_address + i * PAGE_SIZE);
			new_frame->va = 0;
			new_frame->numOfPages = 0;

		}

	} else {
		panic("kfree() invalid address...!!");
	}

}

unsigned int kheap_physical_address(unsigned int virtual_address) {
//TODO: [PROJECT'24.MS2 - #05] [1] KERNEL HEAP - kheap_physical_address
// Write your code here, remove the panic and write your code
//panic("kheap_physical_address() is not implemented yet...!!");

//return the physical address corresponding to given virtual_address
//refer to the project presentation and documentation for details

//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

	uint32 *ptr_page_table;

	int ret = get_page_table(ptr_page_directory, virtual_address,
			&ptr_page_table);

	if (ret == 0) {

		uint32 page_table_entry = ptr_page_table[PTX(virtual_address)];
		if (page_table_entry == 0)
			return 0;
		// frame number
		page_table_entry = (page_table_entry >> 12) << 12;
		uint32 offset = virtual_address & 0xfff;

		//cprintf("in fun pa %x\n",page_table_entry );
		return page_table_entry + offset;
		//	return (ptr_page_table[PTX(virtual_address)] & 0xFFFFF000) + (virtual_address & 0x00000FFF);
	} else {
		//cprintf("in fun pa %x\n",0x0);
		return -1;
	}

}

unsigned int kheap_virtual_address(unsigned int physical_address) {
//TODO: [PROJECT'24.MS2 - #06] [1] KERNEL HEAP - kheap_virtual_address
// Write your code here, remove the panic and write your code
//panic("kheap_virtual_address() is not implemented yet...!!");

//return the virtual address corresponding to given physical_address
//refer to the project presentation and documentation for details

//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
//	cprintf("in phys\n");
//	cprintf("the phys add %x",physical_address);

	struct FrameInfo *f = to_frame_info(physical_address);
	//	return (f->va &0xFFFFF000)+PGOFF(physical_address);
	if (f->va == 0) {
		return 0;
	}
	return (f->va & 0xFFFFF000) + (physical_address & 0x00000FFF);
}
//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, if moved to another loc: the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size) {
	//TODO: [PROJECT'24.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc
	// Write your code here, remove the panic and write your code
	//	return NULL;
	//	panic("krealloc() is not implemented yet...!!");

	if (virtual_address == NULL)
		return kmalloc(new_size);
	if (new_size == 0) {
		kfree(virtual_address);
		return (void *) NULL;
	}
	if (virtual_address >= (void*) KERNEL_HEAP_START
			&& virtual_address < (void*) end) {

		if ((new_size + 8) > DYN_ALLOC_MAX_BLOCK_SIZE) {
			free_block(virtual_address);
			return kmalloc(new_size);
		} else {
			return realloc_block_FF(virtual_address, new_size);
		}

	} else if (virtual_address >= (void*) end + PAGE_SIZE
			&& virtual_address < (void*) KERNEL_HEAP_MAX) {

		if ((new_size + 8) <= DYN_ALLOC_MAX_BLOCK_SIZE) {
			kfree(virtual_address);
			return realloc_block_FF(virtual_address, new_size);
		} else {
			kfree(virtual_address);
			return kmalloc(new_size);
		}

	}
return (void*)NULL;
}
