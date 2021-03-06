#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

int copy_reg(char const *argv[], int bufsize);
int copy_symlink(char const *argv[], int bufsiz);

int main(int argc, char const *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Usage: %s src_file dest_file\n", argv[0]);
        return 1;
    }

    struct stat sb;
    if (lstat (argv[1], &sb) == -1) {
        perror("lstat failed");
        return 2;
    }

    switch ((sb.st_mode) & S_IFMT) {

        case S_IFREG:
            if(copy_reg(argv, sb.st_size) < 0) {
                perror("copy_reg");
                return 5;
            }
            break;

        case S_IFDIR:
            fprintf(stderr, "Can't copy directory yet\n");
            return 4; 
            break;

        case S_IFLNK:
            if(copy_symlink(argv, sb.st_size) < 0) {
                perror("copy_symlink failed");
                return 9;
            }
            break;

        case S_IFIFO:
        case S_IFCHR:
        case S_IFBLK:
        case S_IFSOCK:
            if(mknod (argv[2], sb.st_mode, sb.st_rdev) == -1) {
                perror("mknod failed");
                return 7;
            }
            break;

        default: 
            fprintf(stderr, "Unknown filetype\n");
            return 10;
    }

    return 0;
}

int copy_reg(char const *argv[], int bufsize) {

    int fd1 = open (argv[1], O_RDONLY);
    if (fd1 < 0) {
        perror("Failed to open src file");
        return -1;
    }

    int fd2 = open (argv[2], O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd2 < 0) {
        perror("Failed to open or create dest file");
        return -2;
    }    

    char buf[bufsize]; 
    ssize_t nbytes, nbytes_w;    

    off_t offset_r = 0, offset_w = 0;
    while ((nbytes = pread(fd1, buf, bufsize, offset_r)) > 0) {
        offset_r += nbytes;
        while (nbytes) {
            nbytes_w = pwrite(fd2, buf, nbytes, offset_w);
            if (nbytes_w == -1) {
                perror("pwrite failed");
                return -3;
            }
            nbytes -= nbytes_w;
            offset_w += nbytes_w;
        }
    }
    if (nbytes == -1) {
        perror("pread failed");
        return -4;
    }

    if (close(fd1) < 0) {
        perror("Closing src file failed");
        return -5;
    }

    if (close(fd2) < 0) {
        perror("Closing dest file failed");
        return -6;
    }
    
    return 0;
}

int copy_symlink(char const *argv[], int bufsiz) {
 
    char * buf = malloc(bufsiz + 1);
    if (buf == NULL) {
        perror("malloc failed");
        return -1;
    }

    ssize_t nbytes = readlink(argv[1], buf, bufsiz);
    if (nbytes == -1) {
        perror("readlink failed");
        return -2;
    }
    buf[nbytes] = '\0';

    if (nbytes == bufsiz)
       printf("(Returned buffer may have been truncated)\n");

    if(symlink(buf, argv[2]) == -1) {
        perror("symlink falied");
        return -3;
    }

    free(buf);
    return 0;
}
