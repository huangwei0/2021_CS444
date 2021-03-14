#ifndef __STAT_H
# define __STAT_H

#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Device

#ifdef SYMLINK
# define T_SYMLINK 4
#endif // SYMLINK

struct stat {
  short type;  // Type of file
  int dev;     // File system's disk device
  uint ino;    // Inode number
  short nlink; // Number of links to file
  uint size;   // Size of file in bytes
};

#endif // __STAT_H