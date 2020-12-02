#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {

//you write this
char exist = 0;          //This will check if the ID exists 
int i = 0; 
//acquire lock for shm table
acquire(&(shm_table.lock));
//Case 1: ID exists
//We loop through the table
for(i= 0; i < 64; ++i)
{
	if(shm_table.shm_pages[i].id == id)
        {
	    //check that it was case 1
	    exist = 1;
	    break;
	}
}
//Case 2: If our shared memory segment does not exist then proceed in the for loop
if(!exist)
    {
        //loop through the table to find the page
        for(i = 0; i < 64; ++i)
        {
            if(shm_table.shm_pages[i].id == 0)
            {
		//allocate a new page
		char *page = kalloc();
                memset(page, 0, PGSIZE);
                shm_table.shm_pages[i].frame = page;
                shm_table.shm_pages[i].id = id;
                break;
            }
        }
    }
// Now we will map the id to the memory in the page table 
         mappages(
		myproc()->pgdir,
		(void *)PGROUNDUP(myproc()->sz),
            	PGSIZE,
            	V2P(shm_table.shm_pages[i].frame),
            	(PTE_W | PTE_U)
		);
    shm_table.shm_pages[i].refcnt++;
    *pointer = (char *)PGROUNDUP(myproc()->sz);
    myproc()->sz += PGSIZE;
    release(&(shm_table.lock));

return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
//you write this too!
for(int i = 0; i < 64; ++i)   // Loop through the page table
{
    //check if id is found
    if(shm_table.shm_pages[i].id == id)
    {
        //check if memory is shared
        if(shm_table.shm_pages[i].refcnt > 1)
        {
 	     --shm_table.shm_pages[i].refcnt;
	}
	else 
	{
            shm_table.shm_pages[i].id = 0;
            shm_table.shm_pages[i].frame = 0;
            shm_table.shm_pages[i].refcnt = 0;
        }
	break;
    }
}



return 0; //added to remove compiler warning -- you should decide what to return
}
