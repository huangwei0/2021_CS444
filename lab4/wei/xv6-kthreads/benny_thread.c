#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

#ifdef KTHREADS

#include "benny_thread.h"

#ifndef PGSIZE
// copied from mmu.h
# define PGSIZE 4096
#endif // PGSIZE

#ifndef NULL
# define NULL 0x0
#endif // NULL

struct benny_thread_s {
    int bid;
    void *bt_stack;
    void *mem_stack;
};

extern int kthread_create(void (*func)(void *), void *, void *);
extern int kthread_join(int);
extern int kthread_exit(int);

static struct benny_thread_s *bt_new(void);

int
benny_thread_create(benny_thread_t *abt, void (*func)(void*), void *arg_ptr)
{
    struct benny_thread_s *bt = bt_new();
    int result = -1;

    bt->bid = kthread_create(func, arg_ptr, bt->bt_stack);
    //assert(bt->bid == kthread_current());
    //printf(1, "\n%s %d: new thread %d\n", __FILE__, __LINE__, bt->bid);

    if (bt->bid != 0) {
        *abt = (benny_thread_t) bt;
        result = 0;
    }
    return result;
}

int
benny_thread_bid(benny_thread_t abt)
{
    struct benny_thread_s *bt = (struct benny_thread_s *) abt;

    return bt->bid;
}

int
benny_thread_join(benny_thread_t abt)
{
    struct benny_thread_s *bt = (struct benny_thread_s *) abt;
    int retVal = -1;
    
    retVal = kthread_join(bt->bid);
    if (retVal == 0) {
        free(bt->mem_stack);
        bt->bt_stack = bt->mem_stack = NULL;
        free(bt);
    }
    
    return retVal;
}

int
benny_thread_exit(int exit_value)
{
    // This function never returns. As part of the exit, the thread
    // disables itself from running again and calls the 
    // scheduler().
    return kthread_exit(exit_value);
}

static struct benny_thread_s *
bt_new(void)
{
    struct benny_thread_s *bt = malloc(sizeof(struct benny_thread_s));

    if (bt == NULL) {
        return NULL;
    }

    // allocate 2 pages worth of memory and then make sure the
    // beginning address used for the stack is page alligned.
    // we want it page alligned so that we don't generate a
    // page fault by accessing the stack for a thread.
    bt->bt_stack = bt->mem_stack = malloc(PGSIZE * 2);
    if (bt->bt_stack == NULL) {
        free(bt);
        return NULL;
    }
    if (((uint) bt->bt_stack) % PGSIZE != 0) {
        // allign the thread stack to a page boundary
        bt->bt_stack += (PGSIZE - ((uint) bt->bt_stack) % PGSIZE);
    }
    bt->bid = -1;

    return bt;
}

#endif // KTHREADS
