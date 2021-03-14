#include "types.h"
#include "user.h"

int main(void)
{
#ifdef HALT
    halt();
#endif // HALT
    exit();
}
