#include "types.h"
#include "stat.h"
#include "user.h"

char *
filetype(int sttype)
{
    static char ftype[20];

    memset(ftype, 0, sizeof(ftype));
    switch (sttype) {
    case T_DIR:
        strcpy(ftype, "directory");
        break;
    case T_FILE:
        strcpy(ftype, "regular file");
        break;
    case T_DEV:
        strcpy(ftype, "device");
        break;
    default:
        strcpy(ftype, "unknown file type");
        break;
    }

    return ftype;
}

int
main(int argc, char *argv[])
{
    int i;
    int res;
    struct stat st;

    for (i = 1; i < argc; i++) {
        res = stat(argv[i], &st);
        if (res < 0) {
            printf(2, "stat failed on file: %s\n", argv[i]);
        }
        else {
            printf(1, "name:   %s\n type:  %s\n links: %d\n inode: %u\n size:  %d\n"
                   , argv[i], filetype(st.type), st.nlink, st.ino, st.size);
        }
    }

    exit();
}
