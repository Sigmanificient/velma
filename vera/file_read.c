#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int file_read_n(char const *filepath, char *buff, size_t size)
{
    int fd = open(filepath, O_RDONLY);
    int rd;

    if (fd < 0)
        return fd;
    rd = read(fd, buff, size);
    close(fd);
    return rd;
}

ssize_t file_read(char const *filepath, char **dest)
{
    struct stat fi;
    char *content;

    if (stat(filepath, &fi) < 0 || !S_ISREG(fi.st_mode))
        return -1;
    content = malloc((fi.st_size + 1) * sizeof *content);
    if (content == NULL)
        return -1;
    content[fi.st_size] = '\0';
    if (file_read_n(filepath, content, fi.st_size) == fi.st_size) {
        *dest = content;
        return fi.st_size;
    }
    free(content);
    return -1;
}
