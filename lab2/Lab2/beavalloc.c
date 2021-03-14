// R. Jesse Chaney
// chaneyr@oregonstate.edu

/***************************************
Data: 2/4/2021
Course: cs444
Author: Wei Huang
reference: https://github.com/orourtim/beavalloc
      https://github.com/hydra314/cs444-assigment2
      https://github.com/kerrmat/beavalloc
****************************************/

#include "beavalloc.h"

typedef struct mem_block_s {
    uint8_t free;
    size_t capacity;
    size_t size;

    struct mem_block_s *prev;
    struct mem_block_s *next;
} mem_block_t;


#define BLOCK_SIZE (sizeof(mem_block_t))
#define BLOCK_DATA(__curr) (((void *) __curr) + (BLOCK_SIZE))

static mem_block_t *block_list_head = NULL;
static mem_block_t *block_list_tail = NULL;
const static size_t header_size =  sizeof(mem_block_t);

static void *lower_mem_bound = NULL;
static void *upper_mem_bound = NULL;

static uint8_t isVerbose = FALSE;
static FILE *beavalloc_log_stream = NULL;


// This is some gcc magic.
static void init_streams(void) __attribute__((constructor));
static int check_free_block(size_t size);
static void *get_block(size);
static int *find_block(void *ptr);
static int *merge_blocks(mem_block_t *b1, mem_block_t*b2);




// Leave this alone
static void
init_streams(void)
{
    beavalloc_log_stream = stderr;
}

// Leave this alone
void
beavalloc_set_verbose(uint8_t verbosity)
{
    isVerbose = verbosity;
    if (isVerbose) {
        fprintf(beavalloc_log_stream, "Verbose enabled\n");
    }
}

// Leave this alone
void
beavalloc_set_log(FILE *stream)
{
    beavalloc_log_stream = stream;
}

//function
static int check_free_block(size_t size){
  mem_block_t *curr = block_list_head;
  while(curr != NULL){
    if(curr->free &&(curr->capacity - size >0)){
      if(isVerbose){
        printf("free lock exits.\n" );
      }
      return TRUE;
    }
    curr = curr->next;
  }
  return FALSE;
}

static void *get_block(size_t size){
  mem_block_t *curr = block_list_head;
  while(curr != NULL){
    if(curr->free &&(curr->capacity - size > 0)){
      curr->free =  FALSE;
      curr->size = size;
      if(curr->capacity - size > size){
        mem_block_t *block = NULL;
        char *block_g = (char *)curr;

        block_g += header_size + size;
        block = (mem_block_t *)block_g;

        block->size = 0;
        block->capacity = curr->capacity - size - header_size;
        block->free = TRUE;
        block->prev = curr;
        block->next = curr->next;


        curr->next = block;
        curr->size = size;
        curr->capacity = size;

        if (block->next == NULL)
            block_list_tail = block;
      }
      return (curr+1);
    }
    curr = curr ->next;
  }
  return NULL;
}


// The basic memory allocator.
// If you pass NULL or 0, then NULL is returned.
// If, for some reason, the system cannot allocate the requested
//   memory, set errno and return NULL.
// Do not allocate a chunk of memory smaller than 20 bytes (plus
//   whatever you need for your structure).
// You must use sbrk() or brk() in requesting more memory for your
//   beavalloc() routine to manage.
void *
beavalloc(size_t size){
    void *ptr = NULL;
    size_t bytes = 0;
    struct mem_block_s *block= NULL;

    if(size == (size_t)NULL) {
      if(isVerbose){
        printf("beavalloc size is NULL\n");
      }
      return NULL;
    }

    if(lower_mem_bound == NULL){
      if (isVerbose){
        printf("base memory  location set\n");
      }
      lower_mem_bound = sbrk(0);
    }

    if( check_free_block(size)){
      ptr = get_block(size);
    }
    else{
      int remainder = (size + header_size) % 1024;
      int multiplier = (size + header_size) / 1024;
      if (remainder) {
        multiplier++;
      }
      bytes = 1024 * multiplier;
      block = sbrk(bytes);
      if (block == (void *)-1) {
         errno = ENOMEM;
         return NULL;
      }

       upper_mem_bound = sbrk(0);
       block->next = NULL;
       block->free = FALSE;
       block->size = size;
       block->capacity = bytes - header_size;

       if (block_list_head == NULL) {
         block->prev = NULL;
         block_list_head = block_list_tail = block;
       }
       else {
        block->prev = block_list_tail;
        block_list_tail->next = block;
        block_list_tail = block;
      }

      ptr = block + 1;
    }
    return ptr;
}

//function

static int *find_block(void *ptr)
{
   mem_block_t *block;
   for(block = block_list_head; block; block = block->next){
      if(block == ptr){
         if(isVerbose)
            printf("Found block  at address %p\n", ptr);
         return block;
      }
   }
   return NULL;
}

static int *merge_blocks(mem_block_t *b1, mem_block_t*b2)
{
   b1->capacity += b2->capacity + header_size;
   //remove_node
   if(b2->next)
      (b2->next)->prev = b2->prev;
   if(b2->prev)
      (b2->prev)->next = b2->next;
   return b1;
}


void
beavfree(void *ptr)
{
  mem_block_t *block = find_block(ptr);
  if(!ptr || !block || block->free){
     if(isVerbose)
        printf("Failed to find block\n");
     return;
  }
  //reset_freed_block;
  block->free = TRUE;
  block->capacity += block->size;
  block->size = 0;
  for(block = block_list_head; block; block = block->next){
     if(block->free && block->prev && (block->prev)->free){
        block = merge_blocks(block->prev, block);
     }
     if(block->free && block->next && (block->next)->free){
        block = merge_blocks(block, block->next);
     }
  }
}

// Completely reset your heap back to zero bytes allocated.
// You are going to like being able to do this.
// Implementation can be done in as few as 1 line, though
//   you will probably use more to reset the stats you keep
//   about heap.
// After you've called this function, everything you had in
//   the heap is just __GONE__!!!
// You should be able to call beavalloc() after calling beavalloc_reset()
//   to restart building the heap again.
void
beavalloc_reset(void)
{
  if(block_list_head){
    brk(block_list_head);
    block_list_head = NULL;
    block_list_tail = NULL;
  }
}

// This is like the regular calloc() call. See the man page for details.
// Unlike the memory returned from malloc(), the memory returned from a
//   call to calloc() will initialize the returned memory to all zeroes.
// Your beavcallo() should do the same.
// Do youself a BIG favor, don't try to reimplement the capabality of
//   beavalloc() in here, call it instead.
void *
beavcalloc(size_t nmemb, size_t size)
{
     void *ptr = NULL;
    if(!nmemb || !size){
      return NULL;
    }
    ptr = beavalloc(nmemb * size);
    memset(ptr, 0, nmemb * size);
     return ptr;
}

// This is like the regular realloc() call. See the man page for details.
// As realloc() is magic, so is beavrealloc().
// Do youself a BIG favor, don't try to reimplement the capabality of
//   beavalloc() in here, call it instead.
void *
beavrealloc(void *ptr, size_t size)
{
    void *nptr = NULL;
    if(!size){
      return NULL;
    }
    if(!ptr){
      return beavalloc(size);
    }
    nptr = beavalloc(size);
    memmove(nptr, ptr, size);
    beavfree(ptr);
     return nptr;
}

// This is exactly like the strdup() call. See the man page for details.
// Do youself a BIG favor, don't try to reimplement the capabality of
//   beavalloc() in here, call it instead.
void *
beavstrdup(const char *s)
{

    void *nptr = NULL;

    size_t length = strlen(s);
    size_t size = length*sizeof(char) + 1;

    if(size > 0){
      nptr = beavalloc(size);
      memcpy(nptr, s, size);
    }

    return nptr;
}

// Leave this alone
void
beavalloc_dump(void)
{
    mem_block_t *curr = NULL;
    unsigned i = 0;
    unsigned user_bytes = 0;
    unsigned capacity_bytes = 0;
    unsigned block_bytes = 0;
    unsigned used_blocks = 0;
    unsigned free_blocks = 0;

    fprintf(beavalloc_log_stream, "Heap map\n");
    fprintf(beavalloc_log_stream
            , "  %s\t%s\t%s\t%s\t%s"
              "\t%s\t%s\t%s\t%s\t%s"
            "\n"
            , "blk no  "
            , "block add "
            , "next add  "
            , "prev add  "
            , "data add  "

            , "blk size "
            , "capacity "
            , "size     "
            , "excess   "
            , "status   "
        );
    for (curr = block_list_head, i = 0; curr != NULL; curr = curr->next, i++) {
        fprintf(beavalloc_log_stream
                , "  %u\t\t%9p\t%9p\t%9p\t%9p\t%u\t\t%u\t\t"
                  "%u\t\t%u\t\t%s\t%c"
                , i
                , curr
                , curr->next
                , curr->prev
                , BLOCK_DATA(curr)

                , (unsigned) (curr->capacity + BLOCK_SIZE)
                , (unsigned) curr->capacity
                , (unsigned) curr->size
                , (unsigned) (curr->capacity - curr->size)
                , curr->free ? "free  " : "in use"
                , curr->free ? '*' : ' '
            );
        fprintf(beavalloc_log_stream, "\n");
        user_bytes += curr->size;
        capacity_bytes += curr->capacity;
        block_bytes += curr->capacity + BLOCK_SIZE;
        if (curr->free == TRUE) {
            free_blocks++;
        }
        else {
            used_blocks++;
        }
    }
    fprintf(beavalloc_log_stream
            , "  %s\t\t\t\t\t\t\t\t"
              "%u\t\t%u\t\t%u\t\t%u\n"
            , "Total bytes used"
            , block_bytes
            , capacity_bytes
            , user_bytes
            , capacity_bytes - user_bytes
        );
    fprintf(beavalloc_log_stream
            , "  Used blocks: %4u  Free blocks: %4u  "
              "Min heap: %9p    Max heap: %9p   Block size: %lu bytes\n"
            , used_blocks
            , free_blocks
            , lower_mem_bound
            , upper_mem_bound
            , BLOCK_SIZE
        );
}
