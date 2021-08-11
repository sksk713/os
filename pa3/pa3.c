/**********************************************************************
 * Copyright (c) 2020-2021
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "types.h"
#include "list_head.h"
#include "vm.h"

/**
 * Ready queue of the system
 */
extern struct list_head processes;

/**
 * Currently running process
 */
extern struct process *current;

/**
 * Page Table Base Register that MMU will walk through for address translation
 */
extern struct pagetable *ptbr;


/**
 * The number of mappings for each page frame. Can be used to determine how
 * many processes are using the page frames.
 */
extern unsigned int mapcounts[];


/**
 * alloc_page(@vpn, @rw)
 *
 * DESCRIPTION
 *   Allocate a page frame that is not allocated to any process, and map it
 *   to @vpn. When the system has multiple free pages, this function should
 *   allocate the page frame with the **smallest pfn**.
 *   You may construct the page table of the @current process. When the page
 *   is allocated with RW_WRITE flag, the page may be later accessed for writes.
 *   However, the pages populated with RW_READ only should not be accessed with
 *   RW_WRITE accesses.
 *
 * RETURN
 *   Return allocated page frame number.
 *   Return -1 if all page frames are allocated.
 */
unsigned int alloc_page(unsigned int vpn, unsigned int rw) {
	int outerpt_idx = vpn / NR_PTES_PER_PAGE;
	int pt_idx = vpn % NR_PTES_PER_PAGE;
	int pfn = 0;

	for (pfn; pfn < NR_PAGEFRAMES; pfn++) {
		if (mapcounts[pfn] == 0) {
			break;
		}
	}
	mapcounts[pfn]++;

	if (current->pagetable.outer_ptes[outerpt_idx] == NULL) {
		current->pagetable.outer_ptes[outerpt_idx] = malloc(sizeof(struct pte_directory));
	}

	current->pagetable.outer_ptes[outerpt_idx]->ptes[pt_idx].valid = true;
	current->pagetable.outer_ptes[outerpt_idx]->ptes[pt_idx].pfn = pfn;

	if (rw == RW_READ) {
		current->pagetable.outer_ptes[outerpt_idx]->ptes[pt_idx].writable = false;
	}
	else {
		current->pagetable.outer_ptes[outerpt_idx]->ptes[pt_idx].writable = RW_WRITE;
	}

	return pfn;
}


/**
 * free_page(@vpn)
 *
 * DESCRIPTION
 *   Deallocate the page from the current processor. Make sure that the fields
 *   for the corresponding PTE (valid, writable, pfn) is set @false or 0.
 *   Also, consider carefully for the case when a page is shared by two processes,
 *   and one process is to free the page.
 */
void free_page(unsigned int vpn) {
	int outerpt_idx = vpn / NR_PTES_PER_PAGE;
	int pt_idx = vpn % NR_PTES_PER_PAGE;
	int pfn = current->pagetable.outer_ptes[outerpt_idx]->ptes[pt_idx].pfn;

	current->pagetable.outer_ptes[outerpt_idx]->ptes[pt_idx].valid = false;
	current->pagetable.outer_ptes[outerpt_idx]->ptes[pt_idx].writable = false;
	mapcounts[pfn]--;

}


/**
 * handle_page_fault()
 *
 * DESCRIPTION
 *   Handle the page fault for accessing @vpn for @rw. This function is called
 *   by the framework when the __translate() for @vpn fails. This implies;
 *   0. page directory is invalid
 *   1. pte is invalid
 *   2. pte is not writable but @rw is for write
 *   This function should identify the situation, and do the copy-on-write if
 *   necessary.
 *
 * RETURN
 *   @true on successful fault handling
 *   @false otherwise
 */
bool handle_page_fault(unsigned int vpn, unsigned int rw) {	
    int outerpt_idx = vpn / NR_PTES_PER_PAGE;
	int pt_idx = vpn % NR_PTES_PER_PAGE;
	int pfn = current->pagetable.outer_ptes[outerpt_idx]->ptes[pt_idx].pfn;


//mapcount 2이상이면 1 줄이고 할당
    if (current->pagetable.outer_ptes[outerpt_idx]->ptes[pt_idx].private == RW_READ) {

		if (mapcounts[pfn] > 1) {
			mapcounts[pfn]--;
			alloc_page(vpn, RW_WRITE);

			return true;
		}

		else {
			current->pagetable.outer_ptes[outerpt_idx]->ptes[pt_idx].writable = RW_WRITE;

			return true;
		}
		
    }

	return false;
}


/**
 * switch_process()
 *
 * DESCRIPTION
 *   If there is a process with @pid in @processes, switch to the process.
 *   The @current process at the moment should be put into the @processes
 *   list, and @current should be replaced to the requested process.
 *   Make sure that the next process is unlinked from the @processes, and
 *   @ptbr is set properly.
 *
 *   If there is no process with @pid in the @processes list, fork a process
 *   from the @current. This implies the forked child process should have
 *   the identical page table entry 'values' to its parent's (i.e., @current)
 *   page table. 
 *   To implement the copy-on-write feature, you should manipulate the writable
 *   bit in PTE and mapcounts for shared pages. You may use pte->private for 
 *   storing some useful information :-)
 */
void switch_process(unsigned int pid) {
	struct process *temp = NULL;
    struct list_head *lh = NULL;

// 이미 존재
	list_for_each(lh, &processes) {
	    temp = list_entry(lh, struct process, list);

	    if (temp->pid == pid) {
			list_add_tail(&current->list,&processes);
			list_del_init(&current->list);

			current = temp;
			ptbr = &temp->pagetable;

			return;
	    }
	}

// 없으면
	struct process *child = malloc(sizeof(struct process));
	child->pid = pid;

	for (int i = 0; i < NR_PTES_PER_PAGE; i++) {
		
		// current
		// child
		if (current->pagetable.outer_ptes[i] != NULL) {
			child->pagetable.outer_ptes[i] = malloc(sizeof(struct pte_directory));
		
			//같은 곳 바라봄
			for (int j = 0; j < NR_PTES_PER_PAGE; j++) {
				if (current->pagetable.outer_ptes[i]->ptes[j].valid == true) {
					child->pagetable.outer_ptes[i]->ptes[j].valid = true;
					child->pagetable.outer_ptes[i]->ptes[j] = current->pagetable.outer_ptes[i]->ptes[j];

					mapcounts[current->pagetable.outer_ptes[i]->ptes[j].pfn]++;
				}

				//w 빼고 저장
				if (child->pagetable.outer_ptes[i]->ptes[j].writable != false) {
					child->pagetable.outer_ptes[i]->ptes[j].writable = false;
					child->pagetable.outer_ptes[i]->ptes[j].private = RW_READ;
					
					current->pagetable.outer_ptes[i]->ptes[j].writable = false;
					current->pagetable.outer_ptes[i]->ptes[j].private = RW_READ;
				}

			}	
		}
	}

	list_add_tail(&current->list,&processes);

	ptbr = &(child->pagetable);

	current = child;
}