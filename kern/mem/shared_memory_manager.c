#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/queue.h>
#include <inc/environment_definitions.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/syscall.h>
#include "kheap.h"
#include "memory_manager.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
struct Share* get_share(int32 ownerID, char* name);

//===========================
// [1] INITIALIZE SHARES:
//===========================
//Initialize the list and the corresponding lock
void sharing_init() {
#if USE_KHEAP
	LIST_INIT(&AllShares.shares_list)
	;
	init_spinlock(&AllShares.shareslock, "shares lock");
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//==============================
// [2] Get Size of Share Object:
//==============================
int getSizeOfSharedObject(int32 ownerID, char* shareName) {
	//[PROJECT'24.MS2] DONE
	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	//
	struct Share* ptr_share = get_share(ownerID, shareName);
	if (ptr_share == NULL)
		return E_SHARED_MEM_NOT_EXISTS;
	else
		return ptr_share->size;

	return 0;
}

//===========================================================

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//===========================
//[1] Create frames_storage:
//===========================
// Create the frames_storage and initialize it by 0
inline struct FrameInfo** create_frames_storage(int numOfFrames) {
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_frames_storage()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_frames_storage is not implemented yet");
	//Your Code is Here...

//	cprintf(
//			" before kmalloc of the array of the pointer with size into frames  %d \n",
//			ROUNDUP((numOfFrames * sizeof(struct FrameInfo)),
//					PAGE_SIZE) / PAGE_SIZE);

	//struct FrameInfo** storage = kmalloc(0);

	struct FrameInfo** storage = kmalloc(
			(uint32) (numOfFrames * sizeof(uint32*)));
//
//	cprintf(" the size of the frame info %d \n", sizeof(struct FrameInfo));
//	cprintf(" the num of the frame info %d \n", numOfFrames);

	//cprintf(" the size of the frame info without size of va anf page size  %d \n",sizeof(struct FrameInfo)-(2*sizeof(uint32)));

	if (storage == NULL) {
		return NULL;
	}

	for (int i = 0; i < numOfFrames; ++i) {

		storage[i] = NULL;
	}
	//cprintf(" in create array \n ");
	return storage;
}
//=====================================

//[2] Alloc & Initialize Share Object:
//=====================================
//Allocates a new shared object and initialize its member
//It dynamically creates the "framesStorage"
//Return: allocatedObject (pointer to struct Share) passed by reference
struct Share* create_share(int32 ownerID, char* shareName, uint32 size,
		uint8 isWritable) {
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_share is not implemented yet");
	//Your Code is Here...
//	cprintf(" create the crated obj size in frame %d \n",
//	ROUNDUP((sizeof(struct Share)),PAGE_SIZE) / PAGE_SIZE);

	struct Share* shared = kmalloc(sizeof(struct Share));
	if (shared != NULL) {
		//cprintf(" in create share object and not null \n ");

//init
		shared->ID = (uint32) shared & 0x7FFFFFFF;
		//cprintf(" va in create obj fter mask %p \n ", shared->ID);

		shared->references = 1;

		shared->isWritable = isWritable;
		strcpy(shared->name, shareName);
		//cprintf(" name  in create obj = %s \n ", shared->name);

		shared->size = size;
		shared->ownerID = ownerID;

		int frames = ROUNDUP (size, PAGE_SIZE) / PAGE_SIZE;
		//cprintf(" number of frames passin to the creat array %d \n ", frames);

		shared->framesStorage = create_frames_storage(frames);
		if (shared->framesStorage == NULL) {
			kfree((void*) shared);
			return NULL;
		}
		//cprintf(" after call the create array in create opj  \n ");

		return shared;
	} else {
		return NULL;
	}

}
//=============================
// [3] Search for Share Object:
//=============================
//Search for the given shared object in the "shares_list"
//Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share* get_share(int32 ownerID, char* name) {
	//TODO: [PROJECT'24.MS2 - #17] [4] SHARED MEMORY - get_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_share is not implemented yet");
	//Your Code is Here...
	struct Share *shared;
	LIST_FOREACH(shared,&AllShares.shares_list)
	{
		if (ownerID == shared->ownerID && strcmp(name, shared->name) == 0) {
			return shared;
		}
	}
	return NULL;
}

//=========================
// [4] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size,
		uint8 isWritable, void* virtual_address) {
	//TODO: [PROJECT'24.MS2 - #19] [4] SHARED MEMORY [KERNEL SIDE] - createSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("createSharedObject is not implemented yet");
	//Your Code is Here...

	struct Env* myenv = get_cpu_proc(); //The calling environment
	struct Share* existingShare = get_share(ownerID, shareName);
	if (existingShare != NULL) {
		return E_SHARED_MEM_EXISTS;
	}

	struct Share* newShare = create_share(ownerID, shareName, size, isWritable);
	if (newShare == NULL) {
		return E_NO_SHARE;
	}

	uint32 numOfFrames = (ROUNDUP(size, PAGE_SIZE)) / PAGE_SIZE;

//	numOfFrames = numOfFrames
//			- (ROUNDUP((numOfFrames * sizeof(struct FrameInfo)), PAGE_SIZE)
//					/ PAGE_SIZE);
//
//	cprintf(
//			" the total frames allocated in smalloc after deleted the frames of the frame srotrage %d \n",
//			numOfFrames);

	newShare->size = numOfFrames * PAGE_SIZE;
	//cprintf(" the total frames  allocated in create in kernel  %d \n",
	//ROUNDUP(newShare->size , PAGE_SIZE) / PAGE_SIZE);

	for (uint32 i = 0; i < numOfFrames; i++) {

		struct FrameInfo *frameInfo = NULL;
		int res = allocate_frame(&frameInfo);
		if (res == E_NO_MEM) {
			//kfree(newShare->framesStorage[i]);
			kfree(newShare);
			return E_NO_SHARE;
		}

		uint32 currAddress = (uint32) (virtual_address + i * PAGE_SIZE);
		//uint32 *page;

		int ret = map_frame(myenv->env_page_directory, frameInfo, currAddress,
		PERM_WRITEABLE | PERM_USER);

		if (ret != 0) {
			//kfree(newShare->framesStorage[i]);
			kfree(newShare);

			//cprintf(" not map\n ");
			return E_NO_SHARE;
		}
		//cprintf(" the index %d  --------------------------> \n", i);

		newShare->framesStorage[i] = frameInfo;
	}
	LIST_INSERT_TAIL(&AllShares.shares_list, newShare);

	return newShare->ID;
}

//======================
// [5] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char* shareName, void* virtual_address) {
	//TODO: [PROJECT'24.MS2 - #21] [4] SHARED MEMORY [KERNEL SIDE] - getSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("getSharedObject is not implemented yet");
	//Your Code is Here...

	struct Env* myenv = get_cpu_proc(); //The calling environment
	acquire_spinlock(&AllShares.shareslock);
	struct Share *it = NULL, *s = NULL;
//	LIST_FOREACH(it,&AllShares.shares_list)
//	{
//		if (strcmp(shareName, it->name) == 0 && (ownerID == it->ownerID)) {
//			s = it;
//			break;
//		}
//	}
	s = get_share(ownerID, shareName);
	if (s == NULL) {
		release_spinlock(&AllShares.shareslock);
		return E_SHARED_MEM_NOT_EXISTS;
	}
	int numOfFrams = ROUNDUP(s->size,PAGE_SIZE) / PAGE_SIZE;
	for (int i = 0; i < numOfFrams; i++) {
		//struct FrameInfo** framesStorage;
		//struct FrameInfo *f = s->framesStorage[i];
		if (s->isWritable == 1) {
			int ret = map_frame(myenv->env_page_directory, s->framesStorage[i],
					(uint32) virtual_address + i * PAGE_SIZE,
					PERM_WRITEABLE | PERM_USER);
			//cprintf(" the index in sget  %d  --------------------------> \n",i);
			if (ret != 0) {
				release_spinlock(&AllShares.shareslock);
				return 0;
			}
		} else {
			int ret = map_frame(myenv->env_page_directory, s->framesStorage[i],
					(uint32) virtual_address + i * PAGE_SIZE, PERM_USER);

			if (ret != 0) {
				release_spinlock(&AllShares.shareslock);
				return 0;
			}
		}
	}
	s->references++;
	release_spinlock(&AllShares.shareslock);
	return s->ID;
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//==========================
// [B1] Delete Share Object:
//==========================
//delete the given shared object from the "shares_list"
//it should free its framesStorage and the share object itself
void free_share(struct Share* ptrShare) {
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - free_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("free_share is not implemented yet");
	//Your Code is Here...

}
//========================
// [B2] Free Share Object:
//========================
int freeSharedObject(int32 sharedObjectID, void *startVA) {
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - freeSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("freeSharedObject is not implemented yet");
	//Your Code is Here...

}
