#ifndef __BENNY_THREAD_H
# define __BENNY_THREAD_H

typedef int benny_thread_t;

int benny_thread_create(benny_thread_t *, void (*func)(void*), void *);
int benny_thread_join(benny_thread_t);
int benny_thread_exit(int);
int benny_thread_bid(benny_thread_t);

#ifdef NOPE
benny_thread_t benny_thread_self(void);
#endif // NOPE

#endif // __BENNY_THREAD_H
