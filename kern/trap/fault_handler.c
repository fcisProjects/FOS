/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include <kern/cpu/sched.h>
#include <kern/cpu/cpu.h>
#include <kern/disk/pagefile_manager.h>
#include <kern/mem/memory_manager.h>

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE) {
	assert(
			LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE;
}
void setPageReplacmentAlgorithmCLOCK() {
	_PageRepAlgoType = PG_REP_CLOCK;
}
void setPageReplacmentAlgorithmFIFO() {
	_PageRepAlgoType = PG_REP_FIFO;
}
void setPageReplacmentAlgorithmModifiedCLOCK() {
	_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;
}
/*2018*/void setPageReplacmentAlgorithmDynamicLocal() {
	_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;
}
/*2021*/void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps) {
	_PageRepAlgoType = PG_REP_NchanceCLOCK;
	page_WS_max_sweeps = PageWSMaxSweeps;
}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE) {
	return _PageRepAlgoType == LRU_TYPE ? 1 : 0;
}
uint32 isPageReplacmentAlgorithmCLOCK() {
	if (_PageRepAlgoType == PG_REP_CLOCK)
		return 1;
	return 0;
}
uint32 isPageReplacmentAlgorithmFIFO() {
	if (_PageRepAlgoType == PG_REP_FIFO)
		return 1;
	return 0;
}
uint32 isPageReplacmentAlgorithmModifiedCLOCK() {
	if (_PageRepAlgoType == PG_REP_MODIFIEDCLOCK)
		return 1;
	return 0;
}
/*2018*/uint32 isPageReplacmentAlgorithmDynamicLocal() {
	if (_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL)
		return 1;
	return 0;
}
/*2021*/uint32 isPageReplacmentAlgorithmNchanceCLOCK() {
	if (_PageRepAlgoType == PG_REP_NchanceCLOCK)
		return 1;
	return 0;
}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt) {
	_EnableModifiedBuffer = enableIt;
}
uint8 isModifiedBufferEnabled() {
	return _EnableModifiedBuffer;
}

void enableBuffering(uint32 enableIt) {
	_EnableBuffering = enableIt;
}
uint8 isBufferingEnabled() {
	return _EnableBuffering;
}

void setModifiedBufferLength(uint32 length) {
	_ModifiedBufferLength = length;
}
uint32 getModifiedBufferLength() {
	return _ModifiedBufferLength;
}

//===============================
// FAULT HANDLERS
//===============================

//==================
// [1] MAIN HANDLER:
//==================
/*2022*/
uint32 last_eip = 0;
uint32 before_last_eip = 0;
uint32 last_fault_va = 0;
uint32 before_last_fault_va = 0;
int8 num_repeated_fault = 0;

struct Env* last_faulted_env = NULL;
void fault_handler(struct Trapframe *tf) {
	/******************************************************/
	// Read processor's CR2 register to find the faulting address
	uint32 fault_va = rcr2();
	//	cprintf("\n************Faulted VA = %x************\n", fault_va);
	//	print_trapframe(tf);
	/******************************************************/

	//If same fault va for 3 times, then panic
	//UPDATE: 3 FAULTS MUST come from the same environment (or the kernel)
	struct Env* cur_env = get_cpu_proc();
	if (last_fault_va == fault_va && last_faulted_env == cur_env) {
		num_repeated_fault++;
		if (num_repeated_fault == 3) {
			print_trapframe(tf);
			panic(
					"Failed to handle fault! fault @ at va = %x from eip = %x causes va (%x) to be faulted for 3 successive times\n",
					before_last_fault_va, before_last_eip, fault_va);
		}
	} else {
		before_last_fault_va = last_fault_va;
		before_last_eip = last_eip;
		num_repeated_fault = 0;
	}
	last_eip = (uint32) tf->tf_eip;
	last_fault_va = fault_va;
	last_faulted_env = cur_env;
	/******************************************************/
	//2017: Check stack overflow for Kernel
	int userTrap = 0;
	if ((tf->tf_cs & 3) == 3) {
		userTrap = 1;
	}
	if (!userTrap) {
		struct cpu* c = mycpu();
		//cprintf("trap from KERNEL\n");
		if (cur_env
				&& fault_va
						>= (uint32) cur_env->kstack&& fault_va < (uint32)cur_env->kstack + PAGE_SIZE)
			panic("User Kernel Stack: overflow exception!");
		else if (fault_va
				>= (uint32) c->stack&& fault_va < (uint32)c->stack + PAGE_SIZE)
			panic("Sched Kernel Stack of CPU #%d: overflow exception!",
					c - CPUS);
#if USE_KHEAP
		if (fault_va >= KERNEL_HEAP_MAX)
			panic("Kernel: heap overflow exception!");
#endif
	}
	//2017: Check stack underflow for User
	else {
		//cprintf("trap from USER\n");
		if (fault_va >= USTACKTOP && fault_va < USER_TOP)
			panic("User: stack underflow exception!");
	}

	//get a pointer to the environment that caused the fault at runtime
	//cprintf("curenv = %x\n", curenv);
	struct Env* faulted_env = cur_env;
	if (faulted_env == NULL) {
		print_trapframe(tf);
		panic("faulted env == NULL!");
	}
	//check the faulted address, is it a table or not ?
	//If the directory entry of the faulted address is NOT PRESENT then
	if ((faulted_env->env_page_directory[PDX(fault_va)] & PERM_PRESENT)
			!= PERM_PRESENT) {
		// we have a table fault =============================================================
		//		cprintf("[%s] user TABLE fault va %08x\n", curenv->prog_name, fault_va);
		//		print_trapframe(tf);

		faulted_env->tableFaultsCounter++;

		table_fault_handler(faulted_env, fault_va);
	} else {
		if (userTrap) {
			/*============================================================================================*/
			//TODO: [PROJECT'24.MS2 - #08] [2] FAULT HANDLER I - Check for invalid pointers
			//(e.g. pointing to unmarked user heap page, kernel or wrong access rights),
			//your code is here
			uint32 PERM_MARK = 0x400;
//			cprintf("in the validate pointer-------------------------------------------->  \n");

//			if (fault_va==0xffffffff)
//				return;
			uint32 *page;
			int y = get_page_table(faulted_env->env_page_directory, fault_va,
					&page);

//			cprintf("fault add is %p \n", fault_va);

//			cprintf("env ID is %d \n", faulted_env->env_id);
//			cprintf("perm mark is %p \n", PERM_MARK);
//			cprintf("the anding is %d \n", page[PTX(fault_va)] & PERM_MARK);
			if (page != NULL) {
				uint32 entry = page[PTX(fault_va)];
				if ((fault_va >= USER_HEAP_START && fault_va <= USER_HEAP_MAX)
						&& (!(entry & PERM_MARK))) {		//point to user heap
//					cprintf("if bit is not marked in heap \n");
					cprintf("willl exit 1 \n\n");
					env_exit();
				} else if ((entry & PERM_PRESENT) && (!(entry & PERM_USER))) {
//					cprintf("kearnal heap \n");
					cprintf("willl exit 2 \n\n");
					env_exit();
				} else if ((entry & PERM_PRESENT)
						&& (!(entry & PERM_WRITEABLE))) {
//					cprintf("read only  \n");
					cprintf("willl exit 3 \n\n");
					env_exit();
				}
			}

			/*============================================================================================*/
		}

		/*2022: Check if fault due to Access Rights */
		int perms = pt_get_page_permissions(faulted_env->env_page_directory,
				fault_va);
		if (perms & PERM_PRESENT)
			panic(
					"Page @va=%x is exist! page fault due to violation of ACCESS RIGHTS\n",
					fault_va);
		/*============================================================================================*/

		// we have normal page fault =============================================================
		faulted_env->pageFaultsCounter++;

		//		cprintf("[%08s] user PAGE fault va %08x\n", curenv->prog_name, fault_va);
		//		cprintf("\nPage working set BEFORE fault handler...\n");
		//		env_page_ws_print(curenv);

		if (isBufferingEnabled()) {
			__page_fault_handler_with_buffering(faulted_env, fault_va);
		} else {
			//page_fault_handler(faulted_env, fault_va);
			page_fault_handler(faulted_env, fault_va);
		}
		//		cprintf("\nPage working set AFTER fault handler...\n");
		//		env_page_ws_print(curenv);

	}

	/*************************************************************/
	//Refresh the TLB cache
	tlbflush();
	/*************************************************************/
}

//=========================
// [2] TABLE FAULT HANDLER:
//=========================
void table_fault_handler(struct Env * curenv, uint32 fault_va) {
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory,
				(uint32) fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}

//=========================
// [3] PAGE FAULT HANDLER:
//=========================
void page_fault_handler(struct Env * faulted_env, uint32 fault_va) {

	//cprintf("fault add is %p \n", fault_va);

#if USE_KHEAP
	struct WorkingSetElement *victimWSElement = NULL;
	uint32 wsSize = LIST_SIZE(&(faulted_env->page_WS_list));
#else
	int iWS =faulted_env->page_last_WS_index;
	uint32 wsSize = env_page_ws_get_size(faulted_env);
#endif
	if (isPageReplacmentAlgorithmNchanceCLOCK()) {

		if (wsSize < (faulted_env->page_WS_max_size)) {
			//cprintf("PLACEMENT=========================WS Size = %d\n", wsSize );
			//TODO: [PROJECT'24.MS2 - #09] [2] FAULT HANDLER I - Placement
			// Write your code here, remove the panic and write your code
			//panic("page_fault_handler().PLACEMENT is not implemented yet...!!");
//		cprintf("in the page fault \n");
			//int size_working_set = LIST_SIZE(&faulted_env->page_WS_list);

			struct FrameInfo *free_frame = NULL;
//		cprintf("in the page fault  if the wor set still valid \n");
			int r = allocate_frame(&free_frame);
			if (r != 0) {
//			cprintf("no cpace to allocate \n");
				cprintf("willl exit 4 \n\n");
				env_exit();
			}
			int ret = map_frame(faulted_env->env_page_directory, free_frame,
					fault_va,
					PERM_WRITEABLE | PERM_USER);
			if (ret != 0) {
//			cprintf("no space to map \n");
				cprintf("willl exit 5 \n\n");
				env_exit();
			}
			int ret2 = pf_read_env_page(faulted_env, (void*) fault_va);
			if (ret2 == E_PAGE_NOT_EXIST_IN_PF) {		// not in disk
//			cprintf("not in disk  \n");
				if (!((fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX)
						|| (fault_va >= USTACKBOTTOM && fault_va < USTACKTOP))) {
//				cprintf(" not also stack or heap   \n");
					//unmap_frame(faulted_env->env_page_directory, fault_va);
//				cprintf("fault add casting to int  %d \n", (int)fault_va);
//				cprintf("fault add is %p \n", fault_va);
					cprintf("fault add is %p \n", fault_va);
					cprintf("willl exit  6\n\n");
					env_exit();
				}

			}

//		cprintf("in the page fault  data in the disk or stack or heap\n");

			struct WorkingSetElement* new_elem =
					env_page_ws_list_create_element(faulted_env, fault_va);
//		cprintf("  in placment before push element \n");

//		cprintf("  in placment after get element\n");
			LIST_INSERT_TAIL(&faulted_env->page_WS_list, new_elem);
//		cprintf("  in placment after push in the wslist with size %d\n",
//				faulted_env->page_WS_max_size);
			//cprintf("  in placment after push in the list with size %d\n",);

			if (faulted_env->page_WS_max_size
					== LIST_SIZE(&faulted_env->page_WS_list)) {
				faulted_env->page_last_WS_element = LIST_FIRST(
						&faulted_env->page_WS_list);
				//faulted_env->page_last_WS_element++;

//			cprintf(" if list max \n");
			} else {
//			cprintf(" still have size \n");

				faulted_env->page_last_WS_element = NULL;
			}
			//	env_page_ws_print(faulted_env);

//			cprintf("  in placment after set last ele to null \n");
//
//			//refer to the project presentation and documentation for details
//
//			env_page_ws_print(faulted_env);
		}

		else {

			cprintf(
					"\n\nREPLACEMENT=========================WS Size = %d  , faulted addr %p \n\n",
					wsSize, fault_va);
//			env_page_ws_print(faulted_env);

			//refer to the project presentation and documentation for details
			//TODO: [PROJECT'24.MS3] [2] FAULT HANDLER II - Replacement
			// Write your code here, remove the panic and write your code
			//panic("page_fault_handler() Replacement is not implemented yet...!!");

			cprintf("page_WS_max_sweeps %d\n", page_WS_max_sweeps);
			if (page_WS_max_sweeps > 0) {
				cprintf(" -----------  +ve  \n");
				struct FrameInfo *f;

				while (1) {

//					cprintf("the index is %d\n",
//							faulted_env->page_last_WS_index);
//					env_page_ws_print(faulted_env);

					uint32 *pageTable;
					get_page_table(faulted_env->env_page_directory,
							faulted_env->page_last_WS_element->virtual_address,
							&pageTable);

					if (!(pageTable[PTX(
							faulted_env->page_last_WS_element->virtual_address)]
							& (PERM_USED))) {			// not used
						cprintf(" Not Used \n");
						faulted_env->page_last_WS_element->sweeps_counter++;

						if (faulted_env->page_last_WS_element->sweeps_counter
								== page_WS_max_sweeps) {
							cprintf(" will replace \n");
							// delete
							uint32 page_permissions =
									pt_get_page_permissions(
											faulted_env->env_page_directory,
											faulted_env->page_last_WS_element->virtual_address);
							if (page_permissions & PERM_MODIFIED) { // not used and modefied
								uint32* page;
								f =
										get_frame_info(
												faulted_env->env_page_directory,
												faulted_env->page_last_WS_element->virtual_address,
												&page);
								cprintf(
										" will update the disk...............................................................\n");
								int ret =
										pf_update_env_page(faulted_env,
												faulted_env->page_last_WS_element->virtual_address,
												f);

								if (ret == E_PAGE_NOT_EXIST_IN_PF) {
									cprintf(
											"\n---------------------E_PAGE_NOT_EXIST_IN_PF---------------------------\n")

											;
								}

//								env_page_ws_print(faulted_env);

							}
							uint32 * pa;
							struct FrameInfo *fault =
									get_frame_info(
											faulted_env->env_page_directory,
											faulted_env->page_last_WS_element->virtual_address,
											&pa);

							if (fault->references == 1) {
								int x = allocate_frame(&fault);
							}
							struct WorkingSetElement* next = LIST_NEXT(
									(faulted_env->page_last_WS_element));
							struct WorkingSetElement* ws_last =
									faulted_env->page_last_WS_element;
							env_page_ws_invalidate(faulted_env,
									faulted_env->page_last_WS_element->virtual_address);
//							cprintf(" after delete the element \n");
//							env_page_ws_print(faulted_env);
//							env_page_ws_print(faulted_env);
//							cprintf(" pf_read_env_page  \n");

							int ret = map_frame(faulted_env->env_page_directory,
									fault, fault_va,
									PERM_WRITEABLE | PERM_USER);
							if (ret != 0) {
								//			cprintf("no space to map \n");

								env_exit();
							}
							int ret2 = pf_read_env_page(faulted_env,
									(void*) fault_va);

							cprintf(" After pf_read_env_page \n");
//							env_page_ws_print(faulted_env);
							if (ret2 == E_PAGE_NOT_EXIST_IN_PF) {
								cprintf(
										"\n---------------------E_PAGE_NOT_EXIST_IN_PF---------------------------\n");

								if (!((fault_va >= USER_HEAP_START
										&& fault_va < USER_HEAP_MAX)
										|| (fault_va >= USTACKBOTTOM
												&& fault_va < USTACKTOP))) {
									cprintf("will exit ");
									env_exit(); // what ??????
								}
							}
//							env_page_ws_print(faulted_env);

							struct WorkingSetElement* new =
									env_page_ws_list_create_element(faulted_env,
											fault_va);
							if (next != NULL)
								LIST_INSERT_BEFORE(&faulted_env->page_WS_list,
										next, new);
							else {
								LIST_INSERT_TAIL(&faulted_env->page_WS_list,
										new);
							}
							cprintf(" After update the insertion \n");

//							struct WorkingSetElement* tail = LIST_LAST(
//									&faulted_env->page_WS_list);
//							if (tail == faulted_env->page_last_WS_element) {
//								faulted_env->page_last_WS_element = LIST_FIRST(
//										&faulted_env->page_WS_list);
//							} else {
//								faulted_env->page_last_WS_element = LIST_NEXT(
//										faulted_env->page_last_WS_element);
//							}
							faulted_env->page_last_WS_index =
									(faulted_env->page_last_WS_index + 1)
											% faulted_env->page_WS_max_size;
							if (faulted_env->page_last_WS_element == NULL)
								faulted_env->page_last_WS_element = LIST_FIRST(
										&faulted_env->page_WS_list);

//							faulted_env->page_last_WS_element =
//							struct WorkingSetElement *wse = NULL;
//
//							int i = 0, index = faulted_env->page_last_WS_index;
//							LIST_FOREACH(wse, &(faulted_env->page_WS_list))
//							{
//
//								if (wse->sweeps_counter == page_WS_max_sweeps)
//
//								{
//									LIST_REMOVE(&(faulted_env->page_WS_list),
//											wse);
//								}
//								if (i == index)
//									faulted_env->page_last_WS_element = wse;
//								i++;
//
//							}
							cprintf(" After replacement \n");

							env_page_ws_print(faulted_env);

							cprintf(" After replacement  with size in normal  %d\n",
									LIST_SIZE(&faulted_env->page_WS_list));

							break;
						}
					} else {
						cprintf("  Used \n");
						uint32 *pageTable;
						get_page_table(faulted_env->env_page_directory,
								faulted_env->page_last_WS_element->virtual_address,
								&pageTable);
						pageTable[PTX(
								faulted_env->page_last_WS_element->virtual_address)] =
								pageTable[PTX(
										faulted_env->page_last_WS_element->virtual_address)]
										& (~PERM_USED);
//						pd_set_table_unused(faulted_env->env_page_directory,
//								faulted_env->page_last_WS_element->virtual_address);

						faulted_env->page_last_WS_element->sweeps_counter = 0;

					}

					struct WorkingSetElement* tail = LIST_LAST(
							&faulted_env->page_WS_list);
					if (tail == faulted_env->page_last_WS_element) {
						faulted_env->page_last_WS_element = LIST_FIRST(
								&faulted_env->page_WS_list);
					} else {
						faulted_env->page_last_WS_element = LIST_NEXT(
								faulted_env->page_last_WS_element);
					}
					faulted_env->page_last_WS_index =
							(faulted_env->page_last_WS_index + 1)
									% faulted_env->page_WS_max_size;

					//-------------------------------------------------------->
					if (faulted_env->page_last_WS_element == NULL)
						faulted_env->page_last_WS_element = LIST_FIRST(
								&faulted_env->page_WS_list);

				}

			} else { // modefied
				cprintf(" ----------->size is -ve \n");
				env_page_ws_print(faulted_env);
				int numOfSweeps = page_WS_max_sweeps * -1;
				while (1) {
					env_page_ws_print(faulted_env);
					uint32 *pageTable;
					get_page_table(faulted_env->env_page_directory,
							faulted_env->page_last_WS_element->virtual_address,
							&pageTable);

					if (!(pageTable[PTX(
							faulted_env->page_last_WS_element->virtual_address)]
							& (PERM_USED))) { //not used
						faulted_env->page_last_WS_element->sweeps_counter++;

						uint32 page_permissions =
								pt_get_page_permissions(
										faulted_env->env_page_directory,
										faulted_env->page_last_WS_element->virtual_address);
						if (page_permissions & PERM_MODIFIED) { // modified

							if (faulted_env->page_last_WS_element->sweeps_counter
									== numOfSweeps + 1) {
								uint32* page;
								struct FrameInfo* f =
										get_frame_info(
												faulted_env->env_page_directory,
												faulted_env->page_last_WS_element->virtual_address,
												&page);
								pf_update_env_page(faulted_env,
										faulted_env->page_last_WS_element->virtual_address,
										f);
								uint32 * pa;
								struct FrameInfo *fault;
								int x = allocate_frame(&fault);

								struct WorkingSetElement* next = LIST_NEXT(
										(faulted_env->page_last_WS_element));
								struct WorkingSetElement* ws_last =
										faulted_env->page_last_WS_element;
								env_page_ws_invalidate(faulted_env,
										faulted_env->page_last_WS_element->virtual_address);
								//							cprintf(" after delete the element \n");
								//							env_page_ws_print(faulted_env);
								//							env_page_ws_print(faulted_env);
								//							cprintf(" pf_read_env_page  \n");

								int ret = map_frame(
										faulted_env->env_page_directory, fault,
										fault_va,
										PERM_WRITEABLE | PERM_USER);
								if (ret != 0) {
									//			cprintf("no space to map \n");

									env_exit();
								}

								// replace
								cprintf("before read \n\n");
								int ret2 = pf_read_env_page(faulted_env,
										(void*) fault_va);
								cprintf("After read \n\n");
								if (ret2 == E_PAGE_NOT_EXIST_IN_PF) {

									if (!((fault_va >= USER_HEAP_START
											&& fault_va < USER_HEAP_MAX)
											|| (fault_va >= USTACKBOTTOM
													&& fault_va < USTACKTOP))) {
										cprintf("the add is %x\n", fault_va);
										cprintf("willl exit \n\n");
										env_exit(); // what ??????
									}
								}
								struct WorkingSetElement* new =
										env_page_ws_list_create_element(
												faulted_env, fault_va);

								if (next != NULL)
									LIST_INSERT_BEFORE(
											&faulted_env->page_WS_list, next,
											new);
								else {
									LIST_INSERT_TAIL(&faulted_env->page_WS_list,
											new);
								}

								faulted_env->page_last_WS_index =
										(faulted_env->page_last_WS_index + 1)
												% faulted_env->page_WS_max_size;
								if (faulted_env->page_last_WS_element == NULL)
									faulted_env->page_last_WS_element =
											LIST_FIRST(
													&faulted_env->page_WS_list);
								env_page_ws_print(faulted_env);

								cprintf(" After replacement  with size  in modefied %d\n",
										LIST_SIZE(&faulted_env->page_WS_list));
								break;
							}

						} else //not modified
						{
							if (faulted_env->page_last_WS_element->sweeps_counter
									== numOfSweeps) {
								uint32 * pa;
								struct FrameInfo *fault;
								int x = allocate_frame(&fault);

								struct WorkingSetElement* next = LIST_NEXT(
										(faulted_env->page_last_WS_element));
								struct WorkingSetElement* ws_last =
										faulted_env->page_last_WS_element;
								env_page_ws_invalidate(faulted_env,
										faulted_env->page_last_WS_element->virtual_address);
								//							cprintf(" after delete the element \n");
								//							env_page_ws_print(faulted_env);
								//							env_page_ws_print(faulted_env);
								//							cprintf(" pf_read_env_page  \n");

								int ret = map_frame(
										faulted_env->env_page_directory, fault,
										fault_va,
										PERM_WRITEABLE | PERM_USER);
								if (ret != 0) {
									//			cprintf("no space to map \n");

									env_exit();
								}
								// replace
								cprintf("before read \n\n");
								int ret2 = pf_read_env_page(faulted_env,
										(void*) fault_va);
								cprintf("After read \n\n");

								if (ret2 == E_PAGE_NOT_EXIST_IN_PF) {

									if (!((fault_va >= USER_HEAP_START
											&& fault_va < USER_HEAP_MAX)
											|| (fault_va >= USTACKBOTTOM
													&& fault_va < USTACKTOP))) {
										cprintf("the add is %x\n", fault_va);
										cprintf("willl exit \n\n");
										env_exit(); // what ??????
									}
								}
								cprintf(".................1\n");
								struct WorkingSetElement* new =
										env_page_ws_list_create_element(
												faulted_env, fault_va);
								cprintf(".................2\n");

								cprintf(".................3\n");
								if (next != NULL)
									LIST_INSERT_BEFORE(
											&faulted_env->page_WS_list, next,
											new);
								else {
									LIST_INSERT_TAIL(&faulted_env->page_WS_list,
											new);
								}
								cprintf(".................4\n");
								faulted_env->page_last_WS_index =
										(faulted_env->page_last_WS_index + 1)
												% faulted_env->page_WS_max_size;
								cprintf(".................5\n");
								if (faulted_env->page_last_WS_element == NULL)
									faulted_env->page_last_WS_element =
											LIST_FIRST(
													&faulted_env->page_WS_list);
								cprintf(".................6\n");
								env_page_ws_print(faulted_env);
								break;

							}
						}
					} else {
						cprintf("  Used \n");
						uint32 *pageTable;
						get_page_table(faulted_env->env_page_directory,
								faulted_env->page_last_WS_element->virtual_address,
								&pageTable);
						pageTable[PTX(
								faulted_env->page_last_WS_element->virtual_address)] =
								pageTable[PTX(
										faulted_env->page_last_WS_element->virtual_address)]
										& (~PERM_USED);
						//						pd_set_table_unused(faulted_env->env_page_directory,
						//								faulted_env->page_last_WS_element->virtual_address);

						faulted_env->page_last_WS_element->sweeps_counter = 0;

					}

					// update last element
					struct WorkingSetElement* tail = LIST_LAST(
							&faulted_env->page_WS_list);
					if (tail == faulted_env->page_last_WS_element) {
						faulted_env->page_last_WS_element = LIST_FIRST(
								&faulted_env->page_WS_list);
					} else {
						faulted_env->page_last_WS_element = LIST_NEXT(
								faulted_env->page_last_WS_element);
					}
					faulted_env->page_last_WS_index =
							(faulted_env->page_last_WS_index + 1)
									% faulted_env->page_WS_max_size;

					//-------------------------------------------------------->
					if (faulted_env->page_last_WS_element == NULL)
						faulted_env->page_last_WS_element = LIST_FIRST(
								&faulted_env->page_WS_list);
				}

			}
		}
	}
}

void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va) {
	//[PROJECT] PAGE FAULT HANDLER WITH BUFFERING
	// your code is here, remove the panic and write your code
	panic("__page_fault_handler_with_buffering() is not implemented yet...!!");
}

