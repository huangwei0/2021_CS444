#include "types.h"
#include "user.h"

int 
main(int argc, char *argv[])
{
	  int nice_value = 0;
  int pid = 0;
  int i = 0;
for(i = 2; i < argc; i++){

  pid = atoi(argv[1]);
  nice_value = atoi(argv[i]);

  renice(nice_value, pid);

}

	 exit();
}
