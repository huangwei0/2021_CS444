#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "stat.h"

#ifdef APPEND_FILE
# define NUMS "1234567890"
# define LCASE "abcdefghijklmnopqrstuvwxyz"
# define UCASE "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
# define PLUSSES "+++++"
# define MINUSES "-----"
# define FNAME1 "atest1.txt"
# define FNAME2 "atest2.txt"

char buf[1000] = {0};
#endif // APPEND_FILE

int
main(int argc, char *argv[]) {
#ifdef APPEND_FILE
    int flags = O_CREATE | O_RDWR | O_TRUNC | O_APPEND;
    int fd = -1;
    int rez = -1;
    struct stat st;

    //unlink(FNAME1);
    if ((fd = open(FNAME1, flags)) < 0) {
        printf(2, "could not open file\n");
        exit();
    }
    // Just put something into the file.
    memset(buf, 0x0, sizeof(buf));
    strcpy(buf, NUMS);
    write(fd, buf, strlen(buf));

    // If we seek back to the beginning of the file and
    // write to it, will it still append?
    rez = lseek(fd, 0, SEEK_SET);
    assert(rez == 0);
    memset(buf, 0x0, sizeof(buf));
    strcpy(buf, PLUSSES);
    write(fd, buf, strlen(buf));

    // If we seek into the middle of the file and
    // write to it, will it still append?
    rez = lseek(fd, 10, SEEK_SET);
    assert(rez == 10);
    memset(buf, 0x0, sizeof(buf));
    strcpy(buf, LCASE);
    write(fd, buf, strlen(buf));

    // If we seek a different place middle of the file and
    // write to it, will it still append?
    rez = lseek(fd, 20, SEEK_SET);
    assert(rez == 20);
    memset(buf, 0x0, sizeof(buf));
    strcpy(buf, MINUSES);
    write(fd, buf, strlen(buf));

    // If we again, seek back to the beginning of the file and
    // write to it, will it still append?
    rez = lseek(fd, 0, SEEK_SET);
    assert(rez == 0);
    memset(buf, 0x0, sizeof(buf));
    strcpy(buf, UCASE);
    write(fd, buf, strlen(buf));

    // If wee seek past the end of the file and write, will
    // it append to the old end of the file or past the hole
    // we just created in the file?
    rez = lseek(fd, 100, SEEK_END);
    memset(buf, 0x0, sizeof(buf));
    strcpy(buf, NUMS);
    write(fd, buf, strlen(buf));
    close(fd);

    // Now, let's chek out the contents of the file.
    rez = stat(FNAME1, &st);
    fd = open(FNAME1, O_RDONLY);
    read(fd, buf, sizeof(buf));
    printf(1, "file contents   : \"%s\"\n", buf);
    printf(1, "length of string: %d\n", strlen(buf));
    close(fd);

    // We need to test to make sure we did not mess up non-append
    // write to a file.
    flags = O_CREATE | O_WRONLY | O_TRUNC;
    if ((fd = open(FNAME2, flags)) < 0) {
        printf(1, "sad panda\n");
        exit();
    }
    memset(buf, 0x0, sizeof(buf));
    strcpy(buf, NUMS);
    write(fd, buf, strlen(buf));

    rez = lseek(fd, 0, SEEK_SET);
    memset(buf, 0x0, sizeof(buf));
    strcpy(buf, PLUSSES);
    write(fd, buf, strlen(buf));
    close(fd);
#endif // APPEND_FILE
    exit();
}
