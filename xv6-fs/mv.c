#include "types.h"
#include "stat.h"
#include "user.h"

#define DIRSTR 128

int mv(char *, char *);

int
main(int argc, char *argv[])
{
    int i = 0;
    int res = -1;
    char *src = NULL;
    char *dest = NULL;
    struct stat st;
    char dirstr[DIRSTR] = {0};

    if (argc < 3) {
        printf(2, "must have at least 2 arguments\n");
        exit();
    }

    src = argv[1];
    dest = argv[argc - 1];
    // mv f1 f2 f3 dest
    res = stat(dest, &st);

    if (res < 0) {
        mv(src, dest);
    }
    else {
        switch (st.type) {
        case T_FILE:
            unlink(dest);
            mv(src, dest);
            break;
        case T_DIR:
            // mv f1 f2 f3 dir
            // mv f1 dir/f1
            for (i = 1; i < (argc - 1); i++) {
                memset(dirstr, 0, DIRSTR);
                strcpy(dirstr, dest);
                dirstr[strlen(dirstr)] = '/';
                strcpy(&(dirstr[strlen(dirstr)]), argv[i]);
                mv(argv[i], dirstr);
            }
            break;
        default:
            printf(2, "cannot rename to an existing device\n");
            break;
        }
    }

    exit();
}

int
mv(char *src, char *dest)
{
    int res = -1;

    if (link(src, dest) < 0) {
        printf(2, "mv %s %s: failed\n", src, dest);
    }
    else {
        if (unlink(src) < 0) {
            printf(2, "mv %s %s: failed\n", src, dest);
        }
        else {
            res = 0;
        }
    }
    return res;
}
