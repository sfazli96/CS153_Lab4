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

// The shm_open function
// Looks through the shm table to determine if segment ID exists, allocates a page and maps
// that data to it before storing it in the table
int shm_open(int id, char **pointer) {
  //you write this
  char exist = 0; // Value that determines whether the ID exists or not
  int i = 0; 

  // Get the lock for the shm table
  acquire(&(shm_table.lock));
  
  // Case 1: The target ID already exists
  // First loop through the entire shm table to see if ID exists or not
  for(i= 0; i < 64; ++i)
  {
	// If the ID does exist, then set exist to 1 and break
  	if(shm_table.shm_pages[i].id == id)
        {
	    exist = 1;
	    break;
	}
  } 

  // Case 2: The target ID does not exist in the table
  // Run this process in that case
  if(!exist)
  {
      // Loop through the table to find the page where the shared memory segment does not exist
      for(i = 0; i < 64; ++i)
      {
	  // If found, allocate a new page and then break
          if(shm_table.shm_pages[i].id == 0)
          {
		char *page = kalloc();
                memset(page, 0, PGSIZE);
                shm_table.shm_pages[i].frame = page;
                shm_table.shm_pages[i].id = id;
                break;
          }
      }
  }
  // Adjust the memory table and add the ID to its corresponding memory page
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

  return 0;
}

// The shm_close function
// Looks for the shared memory segment in shm table and decrements reference count
// Clears the shm_table if it is zero (completely empty)
int shm_close(int id) {
  //you write this too!

  int i = 0;
  acquire(&(shm_table.lock)); 
 
  // Loop through the page table first to look for the ID
  for(i = 0; i < 64; ++i)
  {
      // If the ID is found, then check to see if the reference count number is greater than 0
      if(shm_table.shm_pages[i].id == id)
      {
          // Decrement if refcnt is greater than 1
          if(shm_table.shm_pages[i].refcnt > 1)
          {
 	       --shm_table.shm_pages[i].refcnt;
	  }

    	  // Set the values to zero otherwise
	  else 
	  {
              shm_table.shm_pages[i].id = 0;
              shm_table.shm_pages[i].frame = 0;
              shm_table.shm_pages[i].refcnt = 0;
          }
	  break;
      }
  }
  release(&(shm_table.lock)); 

  return 0;
}
