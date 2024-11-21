#include <inc/lib.h>

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size) {
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
		cprintf("block alloc\n");
		return alloc_block_FF(size);
	} else if (size > DYN_ALLOC_MAX_SIZE) {
		cprintf("greater than max size\n");
		return NULL;
	} else {
		uint32 numOfPages = ROUNDUP(size,PAGE_SIZE) / PAGE_SIZE;
		uint32 baseVA = myEnv->end + PAGE_SIZE;
		uint32 allocatedVA = 0;
		uint32 bool = 0;
		int counter = 0;
		uint32 start_add;
		uint32 iterator;
		cprintf("initialzations\n");
		for (uint32 va = baseVA; va < USER_HEAP_MAX; va += PAGE_SIZE) {
			//cprintf("outerloop\n");
			iterator = va;
			for (int i = 0; i < numOfPages; ++i) {
				//cprintf("inner loop start\n");
				uint32* ptr_table = NULL;
				//cprintf("before sys_is_frame_free\n");
				uint32 framee = sys_is_frame_free(iterator);
				//cprintf("after sys_is_frame_free\n");
				//get_page_table(myEnv->env_page_directory, va, &ptr_table);
				//cprintf("frame: %d\n",framee);

				if (framee == 1) {
					// free frame
					//cprintf("added to free frame counter\n");
					counter++;
				} else {
					//cprintf("not enough space break\n");
					va = iterator;
					counter = 0;
					break;
				}
				iterator = iterator + PAGE_SIZE;
			}

			if (counter == numOfPages) {
				//cprintf("enough frame exist at start va: %x\n", va);
				start_add = va;
				allocatedVA = va;
				break;
			}

		}
		//cprintf("call sys allocate user mem\n");
		//cprintf("enough frame exist at start va: %x\n", start_add);
		sys_allocate_user_mem(start_add, size);
		//cprintf("after call sys allocate user mem\n");
		return (void*)start_add;

	}
	return NULL;
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and	sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy
}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #14] [3] USER HEAP [USER SIDE] - free()
	// Write your code here, remove the panic and write your code
	panic("free() is not implemented yet...!!");
}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
	// Write your code here, remove the panic and write your code
	panic("smalloc() is not implemented yet...!!");
	return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");
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

void sfree(void* virtual_address)
{
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
void *realloc(void *virtual_address, uint32 new_size)
{
	//[PROJECT]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
