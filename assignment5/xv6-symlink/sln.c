#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
#ifdef SYMLINK
  if(argc != 3){
    printf(2, "Usage: ln old new\n");
  }
  else if(symlink(argv[1], argv[2]) < 0) {
    printf(2, "symlink %s %s: failed\n", argv[1], argv[2]);
  }
#endif //SYMLINK
  exit();
}
