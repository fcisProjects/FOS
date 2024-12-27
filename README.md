# OS'24 Project

## Overview

The **OS'24 Project** is a comprehensive operating system designed to implement key aspects of modern OS functionality. The project is structured into three milestones, each building upon the previous to progressively add features like dynamic memory allocation, kernel and user heaps, shared memory, fault handling, semaphores, and advanced CPU scheduling.

---

## Milestones and Features

### Milestone 1: Preprocessing
This milestone establishes the foundation with basic system operations.

- **Command Prompt**  
  - `process_command()`: Parses and executes user input commands.  
  - Handles linked list-based storage of command details.  

- **System Calls**  
  - Kernel-level API to handle privileged instructions.  
  - Includes parameter validation for security and stability.  

- **Dynamic Allocator**  
  - Explicit Free List for memory allocation and deallocation.  
  - Functions:
    - `initialize_dynamic_allocator()`
    - `alloc_block_FF()` (First Fit allocation)
    - `free_block()`
    - `realloc_block_FF()`  
  - **Bonus**: Best Fit Allocation implemented via `alloc_block_BF()`.  

- **Locks**  
  - SpinLock and SleepLock for process synchronization.  

---

### Milestone 2: Memory Management
Focused on dynamic memory handling for both kernel and user spaces.

- **Kernel Heap Management**  
  - `initialize_kheap_dynamic_allocator()`: Setup for kernel memory.  
  - `kmalloc()`: Allocates kernel space using block or page allocator.  
  - `kfree()`: Releases allocated kernel memory.  
  - `kheap_physical_address()` / `kheap_virtual_address()`: Efficient address resolution.  

- **Fault Handling (Phase I)**  
  - Managed page faults for kernel and user heap allocations.  

- **User Heap Management**  
  - `malloc() / free()`: User-level memory management functions.  
  - `allocate_user_mem() / free_user_mem()`: Kernel-level backing.  

- **Shared Memory**  
  - `smalloc() / sget()`: Shared memory creation and retrieval for inter-process communication.  

---

### Milestone 3: CPU Management
This milestone introduces advanced CPU functionalities.

- **Fault Handler II: Page Replacement**  
  - Nth Chance Clock Algorithm for page replacement.  
  - Enhanced memory management using modified replacement strategies.  

- **User-Level Semaphores**  
  - Synchronization mechanisms with `semWait()` and `semSignal()`.  
  - Shared semaphore creation and retrieval using `create_semaphore()` and `get_semaphore()`.  

- **Priority Round Robin Scheduler**  
  - Dynamic priority management with `sys_env_set_priority()` and starvation handling.  
  - Preemptive scheduling with priority promotion based on starvation thresholds.  

---
### Project team contributors 
Abanoub Saweris 
Yousef Osama 
Hager Hessin 
Hager Sherif 
Youstina Mitcho
Micheal Nabil

