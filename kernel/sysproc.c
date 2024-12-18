#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"




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

#define PTE_A (1<<6)
#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
    uint64 start;
    int len;
    uint64 vec;

    // Lấy tham số từ user-space
    argaddr(0, &start);
    argint(1, &len);
    argaddr(2, &vec);

    // Giới hạn số trang để tránh quá tải
    if (len > 32) // Giới hạn lên 32 nếu bitmask là uint32
        len = 32;

    uint32 bitmask = 0; // Sử dụng uint32 thay vì uint64
    struct proc *p = myproc(); // Lấy tiến trình hiện tại

    for(int i = 0; i < len; i++) {
        uint64 va = start + i * PGSIZE;
        pte_t *pte = walk(p->pagetable, va, 0);
        if(pte && (*pte & PTE_V) && (*pte & PTE_U)) {
            if(*pte & PTE_A) {
                bitmask |= (1U << i); // Sử dụng 1U cho uint32
                *pte &= ~PTE_A;
            }
        }
    }

    // Copy bitmask ra user-space
    if(copyout(p->pagetable, vec, (char *)&bitmask, sizeof(bitmask)) < 0)
        return -1;

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
