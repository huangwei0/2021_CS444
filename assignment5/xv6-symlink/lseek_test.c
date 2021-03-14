#include "types.h"
#include "user.h"
#include "fcntl.h"

#ifdef LSEEK
# define CHARS "1234567890abcdefghijklmnopqrstuvwxyz"
# define FNAME "lseek_test.txt"

char buf[] = CHARS;
char lbuf[] = "ABC";
#endif // LSEEK

int
main(int argc, char *argv[]) {
#ifdef LSEEK
    int flags = O_CREATE | O_RDWR | O_TRUNC;
    int fd = open(FNAME, flags);
    char ch = 0x0;
    int rez = -1;

    if (fd < 0) {
        printf(2, "could not open file\n");
        exit();
    }
    write(fd, buf, strlen(buf));

    // The lseek(fd, 0, SEEK_SET) should move us to just before
    // the first character in the file.
    // 1234567890abcdefghijklmnopqrstuvwxyz
    //^
    rez = lseek(fd, 0, SEEK_SET);
    assert(rez == 0);
    read(fd, &ch, 1);
    printf(1, "%s %d:  '%c'  %d\n", __FILE__, __LINE__, ch, rez);
    assert(ch == '1');

    // We are between the 1 and the 2. The lseek(fd, 4, SEEK_SET)
    // will move to just before the 5.
    // 1234 567890abcdefghijklmnopqrstuvwxyz
    //     ^
    rez = lseek(fd, 4, SEEK_SET);
    assert(rez == 4);
    read(fd, &ch, 1);
    printf(1, "%s %d:  '%c'  %d\n", __FILE__, __LINE__, ch, rez);
    assert(ch == '5');

    // We are just past the 5. The lseek(fd, -1, SEEK_CUR) will move
    // back to just before the 5, to re-read it.
    // 1234 567890abcdefghijklmnopqrstuvwxyz
    //     ^
    rez = lseek(fd, -1, SEEK_CUR);
    assert(rez == 4);
    read(fd, &ch, 1);
    printf(1, "%s %d:  '%c'  %d\n", __FILE__, __LINE__, ch, rez);
    assert(ch == '5');

    // We are just past the 5. The lseek(fd, 5, SEEK_CUR) will move
    // to just before the a.
    // 1234567890 abcdefghijklmnopqrstuvwxyz
    //           ^
    rez = lseek(fd, 5, SEEK_CUR);
    assert(rez == 10);
    read(fd, &ch, 1);
    printf(1, "%s %d:  '%c'  %d\n", __FILE__, __LINE__, ch, rez);
    assert(ch == 'a');

    // We are just past the a. The lseek(fd, -1, SEEK_END) will move
    // the end of the file and backup 1 spot, to just before the z.
    // 1234567890abcdefghijklmnopqrstuvwxy z
    //                                    ^
    rez = lseek(fd, -1, SEEK_END);
    assert(rez == strlen(buf) - 1);
    read(fd, &ch, 1);
    printf(1, "%s %d:  '%c'  %d\n", __FILE__, __LINE__, ch, rez);
    assert(ch == 'z');

    // We are at the end of the file. We backup to just before the a
    // and write ABC, overwriting the abc with ABC.
    // We then backup to just before the A and read it.
    // 1234567890 ABCdefghijklmnopqrstuvwxyz
    //           ^
    rez = lseek(fd, 10, SEEK_SET);
    assert(rez == 10);
    write(fd, lbuf, strlen(lbuf));
    rez = lseek(fd, - strlen(lbuf), SEEK_CUR);
    read(fd, &ch, 1);
    printf(1, "%s %d:  '%c'  %d\n", __FILE__, __LINE__, ch, rez);
    assert(ch == 'A');

    // We seek 5000 characters past the end of the file and
    // write ABC. We then seek to the end and backup 1 spot
    // and read the C.
    //kdebug(3);
    rez = lseek(fd, 5000, SEEK_END);
    assert(rez == (5000 + strlen(buf)));
    write(fd, lbuf, strlen(lbuf));
    rez = lseek(fd, -1, SEEK_END);
    read(fd, &ch, 1);
    printf(1, "%s %d:  '%c'  %d\n", __FILE__, __LINE__, ch, rez);
    assert(ch == 'C');

    // We try to move to before the beginning of the file.
    rez = lseek(fd, -1, SEEK_SET);
    assert(rez == -1);

    close(fd);
#endif // LSEEK
    exit();
}

