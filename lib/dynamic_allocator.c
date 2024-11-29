/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
__inline__ uint32 get_block_size(void* va) {
	uint32 *curBlkMetaData = ((uint32 *) va - 1);
	return (*curBlkMetaData) & ~(0x1);
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
__inline__ int8 is_free_block(void* va) {
	uint32 *curBlkMetaData = ((uint32 *) va - 1);
	return (~(*curBlkMetaData) & 0x1);
}

//===========================
// 3) ALLOCATE BLOCK:
//===========================

void *alloc_block(uint32 size, int ALLOC_STRATEGY) {
	void *va = NULL;
	switch (ALLOC_STRATEGY) {
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list) {
	cprintf("=========================================\n");
	struct BlockElement* blk;
	cprintf("\nDynAlloc Blocks List:\n");
	cprintf("size of list is %d\n", LIST_SIZE(&list));
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", get_block_size(blk),
				is_free_block(blk));
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart,
		uint32 initSizeOfAllocatedSpace) {

	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (initSizeOfAllocatedSpace % 2 != 0)
			initSizeOfAllocatedSpace++; //ensure it's multiple of 2
		if (initSizeOfAllocatedSpace == 0)
			return;
		is_initialized = 1;
	}

	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #04] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("initialize_dynamic_allocator is not implemented yet");
	//Your Code is Here...

	uint32 beg = 1;
	uint32 end = 1;

	uint32* ptr_beg = (uint32*) daStart;
	uint32* ptr_end = (uint32*) (daStart + initSizeOfAllocatedSpace
			- sizeof(uint32));

	*ptr_beg = beg;
	*ptr_end = end;

	LIST_INIT(&freeBlocksList);

	uint32* ptr_header = (uint32*) (daStart + sizeof(uint32));
	uint32* ptr_footer = (uint32*) (daStart + initSizeOfAllocatedSpace
			- sizeof(uint32) - sizeof(uint32));

	uint32 header = initSizeOfAllocatedSpace - 2 * sizeof(uint32);
	uint32 footer = header;

	*ptr_header = header;
	*ptr_footer = footer;

	struct BlockElement* element = (struct BlockElement*) (daStart
			+ 2 * sizeof(uint32));

	LIST_INSERT_HEAD(&freeBlocksList, element);
	cprintf("initialize_dynamic_allocator------------------\n");

}
//==================================
// [2] SET BLOCK HEADER & FOOTER:
//==================================
void set_block_data(void* va, uint32 totalSize, bool isAllocated) {
	//TODO: [PROJECT'24.MS1 - #05] [3] DYNAMIC ALLOCATOR - set_block_data
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("set_block_data is not implemented yet");
	//Your Code is Here...
	//cprintf("in set_block_data \n");
	uint32* header = (uint32*) (va - sizeof(uint32));

	uint32* footer = (uint32*) (va + totalSize - 2 * sizeof(uint32));

	if (isAllocated) {
		totalSize |= 0x1;
	}

	*header = totalSize;

	*footer = totalSize;

////
//	cprintf("in set the header %x\n", header);
//	cprintf("in set the footer %x\n", footer);

	if (!isAllocated) {
		struct BlockElement* element = (struct BlockElement*) va;
		struct BlockElement* s;
		struct BlockElement* maxAddr = NULL;

		LIST_FOREACH(s, &freeBlocksList)
		{
			if (element > s) {
				maxAddr = s;
			} else {
				break;
			}
		}

		if (maxAddr) {
			LIST_INSERT_AFTER(&freeBlocksList, maxAddr, element);
		} else {
			LIST_INSERT_HEAD(&freeBlocksList, element);
		}
	}
}

//=========================================
// [3] ALLOCATE BLOCK BY FIRST FIT:
//=========================================

void *alloc_block_FF(uint32 size) {
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (size % 2 != 0)
			size++;	//ensure that the size is even (to use LSB as allocation flag)
		if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
			size = DYN_ALLOC_MIN_BLOCK_SIZE;
		if (!is_initialized) {
			uint32 required_size = size + 2 * sizeof(int) /*header & footer*/
			+ 2 * sizeof(int) /*da begin & end*/;
			uint32 da_start = (uint32) sbrk(
			ROUNDUP(required_size, PAGE_SIZE) / PAGE_SIZE);
			uint32 da_break = (uint32) sbrk(0);
			initialize_dynamic_allocator(da_start, da_break - da_start);
		}
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #06] [3] DYNAMIC ALLOCATOR - alloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("alloc_block_FF is not implemented yet");
	//Your Code is Here...
//	cprintf("in alloc block ff\n");
//	print_blocks_list(freeBlocksList);

	struct BlockElement *element;
	LIST_FOREACH(element,&freeBlocksList)
	{

		uint32 free_block_size = get_block_size(element);
		if (size + 8 <= free_block_size) {
			if (free_block_size - (size + 2 * sizeof(uint32)) >= 16) {
//				cprintf("there is space and free\n");

				struct BlockElement *newelement =
						(struct BlockElement*) ((uint8*) element + size
								+ 2 * sizeof(uint32));
				LIST_REMOVE(&freeBlocksList, element);
				set_block_data(element, size + 8, 1);
				set_block_data(newelement,
						free_block_size - (size + 2 * sizeof(uint32)), 0);
				return element;
			} else if (free_block_size - (size + 2 * sizeof(uint32)) < 16) {
//				cprintf("there is space and not free\n");
				LIST_REMOVE(&freeBlocksList, element);
				set_block_data(element, free_block_size, 1);
				return element;
			}
		}
	}
//	cprintf("will calll sbrk in block ff\n");
	uint32*oldbrk = sbrk(1);

	if (oldbrk == (void *)-1) {
//		cprintf("00000\n");
		return NULL;
	} else {

		//print_blocks_list(freeBlocksList);

		struct BlockElement* last_elem = LIST_LAST(&freeBlocksList);
		uint32 prev_size = 0;
		bool prev_alloc;
		if (last_elem == NULL) {
			prev_alloc = 0;
		} else {
			prev_size = get_block_size(last_elem);
			prev_alloc = is_free_block(last_elem);

		}


		if (prev_alloc && ((char*)last_elem + prev_size == (char*)oldbrk)) {

//			cprintf("case B (merge)\n");
			LIST_REMOVE(&freeBlocksList, last_elem);

			set_block_data(last_elem, prev_size + PAGE_SIZE, 0);
			return alloc_block_FF(size);

		} else {
			//cprintf("case A\n");
			struct BlockElement* newblock = (struct BlockElement*) oldbrk;
			set_block_data(newblock, PAGE_SIZE, 0);

			return alloc_block_FF(size);

		}

//		print_blocks_list(freeBlocksList);

	}

	return NULL;
}

//=========================================
// [4] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size) {
	//TODO: [PROJECT'24.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("alloc_block_BF is not implemented yet");
	//Your Code is Here...
	{
		if (size % 2 != 0)
			size++;
		if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
			size = DYN_ALLOC_MIN_BLOCK_SIZE;
		if (!is_initialized) {
			uint32 required_size = size + 2 * sizeof(int) + 2 * sizeof(int);
			uint32 da_start = (uint32) sbrk(
			ROUNDUP(required_size, PAGE_SIZE) / PAGE_SIZE);
			uint32 da_break = (uint32) sbrk(0);
			initialize_dynamic_allocator(da_start, da_break - da_start);
		}
	}

	struct BlockElement *best_fit = NULL;
	uint32 best_fit_size = 0xffffffff;

	struct BlockElement *it;

	uint32 meta_size = 2 * sizeof(uint32);
	uint32 size_with_meta = (size + meta_size);

	LIST_FOREACH(it, &freeBlocksList)
	{
		uint32 current_size = get_block_size(it);
		if (current_size >= size_with_meta) {
			if (current_size < best_fit_size) {
				best_fit = it;
				best_fit_size = current_size;
			}
		}
	}

	if (best_fit == NULL) {
		sbrk(0);
		return NULL;
	}

	if (best_fit_size - size_with_meta >= 16) {

		struct BlockElement *newelement =
				(struct BlockElement*) ((uint8*) best_fit + size_with_meta);

		LIST_REMOVE(&freeBlocksList, best_fit);
		set_block_data(best_fit, size + 8, 1);
		set_block_data(newelement, best_fit_size - size_with_meta, 0);

	} else {

		LIST_REMOVE(&freeBlocksList, best_fit);
		set_block_data(best_fit, best_fit_size, 1);
	}

	return (void *) best_fit;

}

//===================================================
// [5] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void* va) {
	//TODO: [PROJECT'24.MS1 - #07] [3] DYNAMIC ALLOCATOR - free_block
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("free_block is not implemented yet");
	//Your Code is Here...

	cprintf("in free block\n");

	void* prev = (char*) va - sizeof(uint32);
	uint32 prev_size = get_block_size(prev);
	bool prev_alloc = is_free_block(prev);

	uint32 cur_size = get_block_size(va);
	//extraaaa
	uint32*oldbrk = sbrk(0);
	//cprintf("leh next or not %p,   %p", va + cur_size, oldbrk);

	void* next;
	uint32 next_size;
	bool next_alloc;
	if ((uint32) oldbrk == (uint32) va + cur_size) // no next
	{
		next_alloc=0;
	}
	else
	{

		next = (char*) va + cur_size;
		next_size = get_block_size(next);
		next_alloc = is_free_block(next);
	}


	if (prev_alloc && next_alloc) {
		struct BlockElement *prevelement = (struct BlockElement*) ((char*) va
				- prev_size);
		struct BlockElement *nextelement = (struct BlockElement*) (next);
		LIST_REMOVE(&freeBlocksList, prevelement);
		LIST_REMOVE(&freeBlocksList, nextelement);

		set_block_data(prevelement, prev_size + cur_size + next_size, 0);

	} else if (prev_alloc && !next_alloc) {
		cprintf("in free block -----------------1 \n");
		struct BlockElement *element = (struct BlockElement*) ((char*) va
				- prev_size);
		LIST_REMOVE(&freeBlocksList, element);
		set_block_data(element, prev_size + cur_size, 0);

	} else if (!prev_alloc && next_alloc) {
		struct BlockElement *element = (struct BlockElement*) (next);
		LIST_REMOVE(&freeBlocksList, element);
		set_block_data(va, next_size + cur_size, 0);

	} else {
		set_block_data(va, cur_size, 0);
	}
}

//=========================================
// [6] REALLOCATE BLOCK BY FIRST FIT:
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'24.MS1 - #08] [3] DYNAMIC ALLOCATOR - realloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("realloc_block_FF is not implemented yet");
	//Your Code is Here...


	uint32 current_size = get_block_size(va);
	if (new_size == 0 && va != (void*) NULL) {
		free_block(va);
		return (void *) NULL;
	} else if (new_size == current_size && va != (void *) NULL) {
		return va;
	} else if (va == (void*) NULL && new_size > 0) {
		return alloc_block_FF(new_size);
	} else if (va == (void*) NULL && new_size == 0) {
		struct BlockElement *element;
		return (void *) NULL;
	}

	if (new_size % 2 != 0)
		new_size++;

	struct BlockElement *va_next = (struct BlockElement*) (va
			+ get_block_size(va));

	if (new_size + 8 > get_block_size(va)) {

		int8 free = is_free_block((void *) ((va_next)));
		if (free == 0) {
			//void *newVA = alloc_block_FF(new_size);
			//memcpy(newVA, va, current_size - 8);

			//free_block(va);

			return va;

		} else {

			if (get_block_size((va_next))
					< ((new_size + 8) - get_block_size(va))) {

				struct BlockElement *element;
				void *newVA = alloc_block_FF(new_size);

				memcpy(newVA, va, current_size - 8);

				free_block(va);

				return newVA;

			} else if (get_block_size((va_next))
					- ((new_size + 8) - get_block_size(va)) < 16) {

				set_block_data(va, new_size + 8, 1);
				LIST_REMOVE(&freeBlocksList, va_next);

				return va;
			}

			else

			{
				void * nextVA = (void *) ((uint32) va + (get_block_size(va)));

				uint32 size_of_next_block = get_block_size(nextVA);

				uint32 oldsize = get_block_size(va);

				struct BlockElement * old_block =
						(struct BlockElement *) (uint8*) va;

				LIST_REMOVE(&freeBlocksList, va_next);
				print_blocks_list(freeBlocksList);
				set_block_data(va, new_size + 8, 1);

				nextVA = (void *) ((uint32) va + new_size + 8);

				set_block_data(nextVA,
						((size_of_next_block + oldsize) - (new_size + 8)), 0);

				//print_blocks_list(freeBlocksList);

				return va;
			}
		}

	} else {
		uint32 size_of_block = get_block_size(va);
		uint32 size_of_new_block = new_size + 8;
		struct BlockElement *header_next3 = (struct BlockElement*) ((uint8 *) va
				+ size_of_new_block);

		if (size_of_block - size_of_new_block >= 16) {

			struct BlockElement * old_block =
					(struct BlockElement *) ((uint8*) va);

			set_block_data(va, size_of_new_block, 1);
			struct BlockElement* nextFreeBlock;
			bool flag=0;
			uint32 sizeOfNextFreeBlock;
			LIST_FOREACH(nextFreeBlock,&freeBlocksList){

				if((uint32)(nextFreeBlock) == (uint32)va + size_of_block){
					flag=1;
					sizeOfNextFreeBlock = get_block_size(nextFreeBlock);
					LIST_REMOVE(&freeBlocksList,nextFreeBlock);
					break;
				}
			}

			if(flag == 1){
				set_block_data(va + new_size + 8,(size_of_block - size_of_new_block) + sizeOfNextFreeBlock, 0);

			}
			else{
				set_block_data(va + new_size + 8, size_of_block - new_size - 8, 0);
			}

			return va;

		} else {

			struct BlockElement *header_next4 =
					(struct BlockElement*) (uint8 *) va + new_size + 8;

			struct BlockElement* nextFreeBlock;
			bool flag = 0;
			uint32 sizeOfNextFreeBlock;
			LIST_FOREACH(nextFreeBlock,&freeBlocksList)
			{

				if ((uint32) (nextFreeBlock) == (uint32) va + size_of_block) {
					flag = 1;
					sizeOfNextFreeBlock = get_block_size(nextFreeBlock);
					LIST_REMOVE(&freeBlocksList, nextFreeBlock);
					break;
				}
			}

			if (flag == 1) {
				set_block_data(va, new_size + 8, 1);
				cprintf("in coalesce/n");
				set_block_data(va + new_size + 8, (size_of_block - new_size + 8) + sizeOfNextFreeBlock, 0);
			}

			return va;

		}
		return (void *) NULL;
	}
	return (void *) NULL;

}

/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
//=========================================
// [7] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size) {
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [8] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size) {
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}
