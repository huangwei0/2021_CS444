//
// File descriptors
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#ifdef LSEEK
# include "fcntl.h"
# include "mmu.h"
#endif // LSEEK

struct devsw devsw[NDEV];
struct {
  struct spinlock lock;
  struct file file[NFILE];
} ftable;

#ifdef KDEBUG
extern ushort proc_kdebug_level;
#endif // KDEBUG

#ifndef MIN
# define MIN(_a,_b) ((_a) < (_b) ? (_a) : (_b))
#endif // MIN

void
fileinit(void)
{
  initlock(&ftable.lock, "ftable");
}

// Allocate a file structure.
struct file*
filealloc(void)
{
  struct file *f;

  acquire(&ftable.lock);
  for(f = ftable.file; f < ftable.file + NFILE; f++){
    if(f->ref == 0){
      f->ref = 1;
      release(&ftable.lock);
      return f;
    }
  }
  release(&ftable.lock);
  return 0;
}

// Increment ref count for file f.
struct file*
filedup(struct file *f)
{
  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("filedup");
  f->ref++;
  release(&ftable.lock);
  return f;
}

// Close file f.  (Decrement ref count, close when reaches 0.)
void
fileclose(struct file *f)
{
  struct file ff;

  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("fileclose");
  if(--f->ref > 0){
    release(&ftable.lock);
    return;
  }
  ff = *f;
  f->ref = 0;
  f->type = FD_NONE;
  release(&ftable.lock);

  if(ff.type == FD_PIPE)
    pipeclose(ff.pipe, ff.writable);
  else if(ff.type == FD_INODE){
    begin_op();
    iput(ff.ip);
    end_op();
  }
}

// Get metadata about file f.
int
filestat(struct file *f, struct stat *st)
{
  if(f->type == FD_INODE){
    ilock(f->ip);
    stati(f->ip, st);
    iunlock(f->ip);
    return 0;
  }
  return -1;
}

// Read from file f.
int
fileread(struct file *f, char *addr, int n)
{
  int r;

  if(f->readable == 0)
    return -1;
  if(f->type == FD_PIPE)
    return piperead(f->pipe, addr, n);
  if(f->type == FD_INODE){
    ilock(f->ip);
    if((r = readi(f->ip, addr, f->off, n)) > 0)
      f->off += r;
    iunlock(f->ip);
    return r;
  }
  panic("fileread");
}

//PAGEBREAK!
// Write to file f.
int
filewrite(struct file *f, char *addr, int n)
{
  int r;

  if(f->writable == 0)
    return -1;
  if(f->type == FD_PIPE)
    return pipewrite(f->pipe, addr, n);
  if(f->type == FD_INODE){
    // write a few blocks at a time to avoid exceeding
    // the maximum log transaction size, including
    // i-node, indirect block, allocation blocks,
    // and 2 blocks of slop for non-aligned writes.
    // this really belongs lower down, since writei()
    // might be writing a device like the console.
    int max = ((MAXOPBLOCKS-1-1-2) / 2) * 512;
    int i = 0;
    while(i < n){
      int n1 = n - i;
      if(n1 > max)
        n1 = max;

      begin_op();
      ilock(f->ip);
#ifdef APPEND_FILE
      if (f->writable & FILE_APPENDABLE) {
          fileseek(f, 0, SEEK_END);
      }
#endif // APPEND_FILE
      if ((r = writei(f->ip, addr + i, f->off, n1)) > 0)
        f->off += r;
      iunlock(f->ip);
      end_op();

      if(r < 0)
        break;
      if(r != n1)
        panic("short filewrite");
      i += r;
    }
    return i == n ? n : -1;
  }
  panic("filewrite");
}

#ifdef LSEEK
int
fileseek(struct file *file, int offset, int whence)
{
    int tot_offset = 0;

    // Based on the offset value, the size of the file in bytes,
    // and whence, calculate the absolute location in the file.
    switch (whence) {
    case SEEK_SET:
        tot_offset = offset;
        break;
    case SEEK_CUR:
        tot_offset = file->off + offset;
        break;
    case SEEK_END:
        tot_offset = file->ip->size + offset;
        break;
    default:
        return -1;
        break;
    }
# ifdef KDEBUG
    if (proc_kdebug_level > 2) {
        cprintf("kdebug: %s %d: offset: %d  tot offset: %d  size: %d   off: %d\n"
                , __FILE__, __LINE__, offset, tot_offset
                , file->ip->size
                , file->off
            );
    }
# endif // KDEBUG
    if (tot_offset < 0) {
        //cprintf("%d %s: attempt to seek before begining\n", __LINE__, __FILE__);
        return -1;
    }
    if (tot_offset > file->ip->size) {
        // the file is going to grow, we need to clear all the new space.
        int new_fbytes = tot_offset - file->ip->size;
        char *new_mbytes = kalloc();

        // Since file is going to grow, move to the end.
        file->off = file->ip->size;
        memset(new_mbytes, 0x0, PGSIZE);
        while (new_fbytes > 0) {
            // write NULLs into the hole
            filewrite(file, new_mbytes, MIN(new_fbytes, PGSIZE));
            new_fbytes -= PGSIZE;
        }
        kfree(new_mbytes);
    }
    file->off = tot_offset;
# ifdef KDEBUG
    if (proc_kdebug_level > 2) {
        cprintf("kdebug: %s %d: tot offset: %d  size: %d   off: %d\n"
                , __FILE__, __LINE__, tot_offset
                , file->ip->size
                , file->off
            );
    }
# endif // KDEBUG
    return tot_offset;
}
#endif // LSEEK

