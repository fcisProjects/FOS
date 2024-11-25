#include <inc/lib.h>

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment) {
//	cprintf("in user sbrk");
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================

struct user_heap_info {
	uint32 address;

	uint32 numOfPages;
};

struct user_heap_info allocPages[((USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE)];

void* malloc(uint32 size) {
	//cprintf("in malloc \n");

	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0)
		return NULL;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #12] [3] USER HEAP [USER SIDE] - malloc()
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");
	sys_isUHeapPlacementStrategyFIRSTFIT();

	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
		return alloc_block_FF(size);
	} else if (size > DYN_ALLOC_MAX_SIZE) {
		return NULL;
	} else {

		uint32 numPages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
		uint32 baseVA = myEnv->end + PAGE_SIZE;
		uint32 counter = 0, start_add = 0;

		for (uint32 va = baseVA; va < USER_HEAP_MAX; va += PAGE_SIZE) {
			if (allocPages[(va - USER_HEAP_START) / PAGE_SIZE].address == 0) {
				counter++;
			} else {
				counter = 0;
				va =
						allocPages[(va - USER_HEAP_START) / PAGE_SIZE].address
								+ allocPages[(va - USER_HEAP_START) / PAGE_SIZE].numOfPages
										* PAGE_SIZE - PAGE_SIZE;
			}

			if (counter == numPages) {

				start_add = va - (numPages - 1) * PAGE_SIZE;
				break;
			}
		}

		if (start_add != 0) {
			//cprintf("start add %p\n", start_add);
			sys_allocate_user_mem(start_add, numPages * PAGE_SIZE);
			allocPages[(start_add - USER_HEAP_START) / PAGE_SIZE].address =
					start_add;
			allocPages[(start_add - USER_HEAP_START) / PAGE_SIZE].numOfPages =
					numPages;

			return (void*) start_add;
		}
//		uint32 numPages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
//
//		uint32 baseVA = myEnv->end+PAGE_SIZE;
//
//		int counter = 0;
//		uint32 start_add;
//		uint32 i=0;
//
//
//
//		for (uint32 va = baseVA; va < USER_HEAP_MAX; va += PAGE_SIZE) {
//			cprintf("1 \n");
//			if ( allocPages[USER_HEAP_START-baseVA].address!=va)
//			{
//				cprintf("2 \n");
//				counter++;
//			}
//			else
//			{
//				counter=0;
//				va = allocPages[va-baseVA].address+allocPages[va-baseVA].size*PAGE_SIZE;
//				i++;
//				break;
//			}
//
//			if (counter == numPages) {
//				cprintf("1 \n");
//				allocPages[va-baseVA].address=va;
//				allocPages[va-baseVA].size=size;
//				cprintf("2 \n");
//				start_add = va;
//				break;
//			}
//		}
//		cprintf(" start_add %x  \n",start_add);
//		sys_allocate_user_mem(baseVA,size);

	}
	return NULL;
}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address) {
	//TODO: [PROJECT'24.MS2 - #14] [3] USER HEAP [USER SIDE] - free()
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");
	if (virtual_address == 0) {

		return;
	}
	if (virtual_address >= (void*) myEnv->start
			&& virtual_address < (void*) myEnv->end) {		// hard limit
			//	cprintf("will call free block allocator \n");
		free_block(virtual_address);
	} else if (virtual_address >= (void*) myEnv->end + PAGE_SIZE
			&& virtual_address < (void*) USER_HEAP_MAX) {
		uint32 size = allocPages[((uint32) virtual_address - USER_HEAP_START)
				/ PAGE_SIZE].numOfPages * PAGE_SIZE;

		sys_free_user_mem((uint32) virtual_address, size);
		allocPages[((uint32) virtual_address - USER_HEAP_START) / PAGE_SIZE].address =
				0;
		allocPages[((uint32) virtual_address - USER_HEAP_START) / PAGE_SIZE].numOfPages =
				0;
	} else {
		panic("free() invalid address...!!");
	}

}

//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable) {
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0)
		return NULL;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");

	/*uint32 numPages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	cprintf(" the total frames allocated %d \n", numPages);

	uint32 baseVA = myEnv->end + PAGE_SIZE;
	uint32 counter = 0, start_add = 0;

	for (uint32 va = baseVA; va < USER_HEAP_MAX; va += PAGE_SIZE) {
		if (allocPages[(va - USER_HEAP_START) / PAGE_SIZE].address == 0) {
			counter++;
		} else {
			counter = 0;
			va = allocPages[(va - USER_HEAP_START) / PAGE_SIZE].address
					+ allocPages[(va - USER_HEAP_START) / PAGE_SIZE].numOfPages
							* PAGE_SIZE - PAGE_SIZE;
		}

		if (counter == numPages) {

			start_add = va - (numPages - 1) * PAGE_SIZE;
			break;
		}
	}

	if (start_add != 0) {
		int ret = sys_createSharedObject(sharedVarName, size, isWritable,
				(void*) start_add);
		allocPages[(start_add - USER_HEAP_START) / PAGE_SIZE].address =
				start_add;
		allocPages[(start_add - USER_HEAP_START) / PAGE_SIZE].numOfPages =
				numPages;
		if (ret == E_NO_SHARE || ret == E_SHARED_MEM_EXISTS) {
			return NULL;
		}
		return (void*) start_add;
	}*/

	void* ptr = malloc(ROUNDUP(size, PAGE_SIZE));
	if (ptr != NULL) {
		int ret = sys_createSharedObject(sharedVarName, size, isWritable, ptr);
		if (ret == E_NO_SHARE || ret == E_SHARED_MEM_EXISTS) {
			return NULL;
		} else {
			return ptr;
		}
	}
	return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName) {
	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	// Write your code here, remove the panic and write your code
	//panic("sget() is not implemented yet...!!");
	uint32 size = sys_getSizeOfSharedObject(ownerEnvID, sharedVarName);
	if (size == 0 || size == E_SHARED_MEM_NOT_EXISTS)
		return NULL;

	void* ptr = malloc(ROUNDUP(size, PAGE_SIZE));
	if (ptr == NULL)
		return NULL;
	else {
		uint32 id = sys_getSharedObject(ownerEnvID, sharedVarName, ptr);
		if (id != E_SHARED_MEM_NOT_EXISTS) {
			return ptr;
		}
	}
	return NULL;
}
//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address) {
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [USER SIDE] - sfree()
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}

//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size) {
	//[PROJECT]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}

//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize) {
	panic("Not Implemented");

}
void shrink(uint32 newSize) {
	panic("Not Implemented");

}
void freeHeap(void* virtual_address) {
	panic("Not Implemented");

}
