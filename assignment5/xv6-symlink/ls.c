#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

#ifdef SYMLINK
# include "fcntl.h"
#endif //SYMLINK

#ifdef LS_IS_MORE
char
filetype(int sttype)
{
    char ftype = '-';

    switch (sttype) {
    case T_DIR:
        ftype = 'd';
        break;
    case T_FILE:
        ftype = 'f';
        break;
    case T_DEV:
        ftype = 'D';
        break;
#ifdef SYMLINK
    case T_SYMLINK:
      ftype = 's';
      break;
#endif //SYMLINK
    default:
        ftype = '?';
        break;
    }

    return ftype;
}
#endif // LS_IS_MORE

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void
ls(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

#ifdef SYMLINK
  if(lstat(path, &st) < 0){
    printf(2, "* ls: cannot stat %s\n", path);
    return;
  }
#else //SYMLINK
  if((fd = open(path, 0)) < 0){
    printf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    printf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }
#endif //SYMLINK

  switch(st.type){
#ifdef SYMLINK
  case T_SYMLINK:
  {
    struct stat lst;

    stat(path, &lst);
    if(lst.type == T_DIR){
      char sbuf[24] = {'\0'};

      readlink(path, sbuf, sizeof(sbuf));
      ls(sbuf);
      return;
    }
  }
#endif //SYMLINK
  case T_FILE:
#ifdef SYMLINK
  printf(1,"%s\t%c\t%d\t%d\t%d"
          , fmtname(path), filetype(st.type), st.nlink, st.ino, st.size);
          if(st.type == T_SYMLINK){
              char sbuf[24] = {'\0'};

              readlink(path, sbuf, sizeof(sbuf));
              printf(1, "-> %s", sbuf);
          }
          printf(1, "\n");
#else //SYMLINK
#ifdef LS_IS_MORE
      printf(1, "%s\t%c\t%d\t%d\t%d\n"
             , fmtname(path), filetype(st.type), st.nlink, st.ino, st.size);
#else // LS_IS_MORE
    printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
#endif // LS_IS_MORE
#endif //SYMLINK
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf(1, "ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
#ifdef SYMLINK
  if((fd = open(path, O_RDONLY)) < 0) {
    printf(2, "failed to open directory\n");
    return;
  }
#endif //SYMLINK
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
#ifdef SYMLINK
  if(lstat(buf, &st) < 0){
    printf(1, "ls: cannot lstat %s\n", buf);
    continue;
  }
  printf(1, "%s\t%c\t%d\t%d\t%d"
          , fmtname(buf), filetype(st.type), st.nlink, st.ino, st.size);
  if(st.type == T_SYMLINK){
      char sbuf[24] = {'\0'};

      readlink(buf, sbuf, sizeof(sbuf));
      printf(1, "-> %s", sbuf);
  }
  printf(1, "\n");
#else //SYMLINK
      if(stat(buf, &st) < 0){
        printf(1, "ls: cannot stat %s\n", buf);
        continue;
      }
#ifdef LS_IS_MORE
      printf(1, "%s\t%c\t%d\t%d\t%d\n", fmtname(buf), filetype(st.type), st.nlink, st.ino, st.size);
#else // LS_IS_MORE
      printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
#endif // LS_IS_MORE
#endif //SYMLINK
    }
#ifdef SYMLINK
  close(fd);
#endif //SYMLINK
    break;
  }
#ifndef SYMLINK
    close(fd);
#endif //SYMLINK
}

int
main(int argc, char *argv[])
{
  int i;

#ifdef LS_IS_MORE
  printf(1, "%s\t\t%s\t%s\t%s\t%s\n",
         "name", "type", "# lks", "ino #", "size"
      );
#endif // LS_IS_MORE
  if(argc < 2){
    ls(".");
    exit();
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit();
}
