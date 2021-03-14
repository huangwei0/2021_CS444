#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "rand.h"


#ifdef HALT
int
sys_halt(void)
{
    outb(0xf4, 0x00);
    return 0;
}
#endif // HALT

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int 
sys_renice(void)
{
   int pid, nice_value, return_value;
  argint(1, &nice_value);
  argint(0, &pid);
  return_value = renice(nice_value, pid);
  return return_value;
}


int 
sys_random(void)
{
    srand(suptime());
  return rand();
}


#ifdef GETPPID
int
sys_getppid(void)
{
    int ppid = 1;

    if (myproc()->parent) {
        ppid = myproc()->parent->pid;
    }
    return ppid;
}
#endif // GETPPID

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

#ifdef PROC_TIMES
//# error I implemented a function here called suptime (because it
//# error makes me think of supper time.
//# error it does all the things of sys_uptime() below. I then have
//# error sys_uptime() just call suptime().
int suptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

#endif // PROC_TIMES

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  #ifdef PROC_TIMES
    return suptime();
  #endif //PROC_TIMES
    
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
