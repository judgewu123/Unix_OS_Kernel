#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"

/* NTU OS 2022 */
/* Page fault handler */
int handle_pgfault(struct proc* p) {
  /* Find the address that caused the fault */
  uint64 va = (r_stval());
  if(va >= p->sz){
    return -1;
  }
  
  if(va < p->trapframe->sp){
    return -1;
  }

  va = PGROUNDDOWN(va);
  pte_t *pte;
  if(*(pte=walk(p->pagetable, va, 0))==0){
    // haven't use this pte before
    char* mem;
    mem = kalloc();
    if(mem==0){
      return -1;
    }
    memset(mem, 0, PGSIZE);
    if(mappages(p->pagetable, va, PGSIZE, (uint64)mem, PTE_V|PTE_W|PTE_X|PTE_R|PTE_U)!=0){
      kfree(mem);
      return -2;
    }
  }else if(*pte & PTE_S){
    // the swapped page was triggered
      char *mem = kalloc();
      if(mem == 0) return -1;
      memset(mem, 0, PGSIZE);
      begin_op();
      read_page_from_disk(ROOTDEV, mem, PTE2BLOCKNO(*pte));
      bfree_page(ROOTDEV, PTE2BLOCKNO(*pte));
      end_op();
      uint64 flag = PTE_FLAGS(*pte);
      *pte = PA2PTE(mem) | flag;
      *pte |= PTE_V;
      *pte &= (~PTE_S);
  }


  return 0;
}
