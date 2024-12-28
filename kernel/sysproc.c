#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

extern pte_t *walk(pagetable_t pagetable, uint64 va, int alloc);
#ifdef LAB_PGTBL
int sys_pgaccess(void)
{
  uint64 start_va;
  int num_pages;
  uint64 user_bitmask_addr;
  uint64 curr_va;
  uint64 bitmask = 0;
  uint64 bit_position = 1;

  struct proc *curr_proc = myproc();

  argaddr(0, &start_va);
  argint(1, &num_pages);
  argaddr(2, &user_bitmask_addr);

  for(curr_va = start_va;curr_va < start_va + num_pages * PGSIZE; curr_va += PGSIZE)
  {
    pte_t *pte = walk(curr_proc->pagetable, curr_va, 0);

    if(pte&&(*pte & PTE_A))
    {
      bitmask |= bit_position;
      *pte &= ~PTE_A;
    }

    bit_position <<= 1;
  }
  
  if(copyout(curr_proc->pagetable, user_bitmask_addr, (char *)&bitmask, sizeof(bitmask)) < 0)
  {
    return -1;
  }
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
