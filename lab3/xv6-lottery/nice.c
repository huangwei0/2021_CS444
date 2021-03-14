#include "types.h"
#include "user.h"
 


 int
 main(int argc, char *argv[])
 {
	renice(getpid(),atoi(argv[1]));
	exec(argv[2], &argv[2]);
 	exit();
 } 