#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h> /* open */
#include <sys/types.h> /* getpid */

#include "utils.h"

#define _XOPEN_SOURCE 700
#define BUFSIZE 1000
long int *buf[1000];

void fprintf_cmdline_info(const int pid, FILE *out) 
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%d/cmdline", pid);

    FILE *f = fopen(path, "r");

    char buf[BUFSIZE + 1];
    int len = fread(buf, 1, BUFSIZE, f);
    buf[len] = '\0';

    fprintf(out, "CMDLINE: ");
    fprintf(out, "%s\n\n", buf);

    fclose(f);
}

void fprintf_cwd_info(const int pid, FILE *out) 
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%d/cwd", pid);

    char buf[BUFSIZE + 1];
    int len = readlink(path, buf, BUFSIZE);
    buf[len] = '\0';

    fprintf(out, "CWD: ");
    fprintf(out, "%s\n\n", buf);
}

void fprintf_environ_info(const int pid, FILE *out) 
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%d/environ", pid);

    char buf[BUFSIZE];
    size_t len;

    fprintf(out, "ENVIRON:\n");
    FILE *f = fopen(path, "r");
    while ((len = fread(buf, 1, BUFSIZE, f)) > 0) 
    {
        for (int i = 0; i < len; ++i)
            if (buf[i] == 0)
                buf[i] = '\n';

        buf[len] = '\0';
        fprintf(out, "%s", buf);
    }
    fprintf(out, "\n");

    fclose(f);
}

void fprintf_exe_info(const int pid, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%d/exe", pid);

    char buf[BUFSIZE + 1];
    int len = readlink(path, buf, BUFSIZE);
    buf[len] = '\0';

    fprintf(out, "EXE: ");
    fprintf(out, "%s\n\n", buf);
}

void fprintf_root_info(const int pid, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%d/root", pid);

    char buf[BUFSIZE + 1];
    int len = readlink(path, buf, BUFSIZE);
    buf[len] = '\0';

    fprintf(out, "ROOT: ");
    fprintf(out, "%s\n\n", buf);
}


void fprintf_fd_info(const int pid, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%d/fd", pid);

    DIR *dp = opendir(path);
    struct dirent *dirp;
    char full_path[BUFSIZE + 1];
    char buf[BUFSIZE + 1];
    fprintf(out, "FD:\n");

    while ((dirp = readdir(dp)) != NULL)
    {
        if ((strcmp(dirp->d_name, ".") != 0)
                && (strcmp(dirp->d_name, "..") != 0))
        {
            sprintf(full_path, "%s/%s", path, dirp->d_name);
            int len = readlink(full_path, buf, BUFSIZE);
            buf[len] = '\0';

            fprintf(out, "%s -> %s\n", dirp->d_name, buf);
        }
    }
    fprintf(out, "\n");

    closedir(dp);
}

void fprintf_stat_info(const int pid, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%d/stat", pid);

    FILE *f = fopen(path, "r");
    fprintf(out, "STAT:\n");

    char buf[BUFSIZE + 1] = "\0";
    fread(buf, 1, BUFSIZE, f);

    char *stat = strtok(buf, " ");
    int i = 0;

    while (stat != NULL)
    {
        fprintf(out, stat_no_descr[i++], stat);
        stat = strtok(NULL, " ");
    }
    fprintf(out, "\n");
    fclose(f);
}

void fprintf_statm_info(const int pid, FILE *out) 
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%d/statm", pid);

    FILE *f = fopen(path, "r");
    char buf[BUFSIZE];
    fread(buf, 1, BUFSIZE, f);

    char *statm = strtok(buf, " ");
    fprintf(out, "STATM: \n");
    for (int i = 0; statm != NULL; i++) {
        fprintf(out, statm_patterns[i], statm);
        statm = strtok(NULL, " ");
    }
    fprintf(out, "\n");
    fclose(f);
}

void fprintf_io_info(const int pid, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%d/io", pid);

    FILE *f = fopen(path, "r");
    fprintf(out, "IO:\n");

    char buf[BUFSIZE + 1];
    int len;

    while ((len = fread(buf, 1, BUFSIZE, f)) > 0)
    {
        for (int i = 0; i < len; ++i)
            if (buf[i] == 0)
                buf[i] = '\n';

        buf[len] = '\0';
        fprintf(out, "%s", buf);
    }
    fprintf(out, "\n");

    fclose(f);
}

void fprintf_maps_info(const int pid, FILE *out)
{
    char *line;
    int start_addr, end_addr, page_size = 4096;
    size_t line_size;
    ssize_t line_length;
    size_t total_page_num = 0;
    char pathToOpen[PATH_MAX];
    snprintf(pathToOpen, PATH_MAX, "/proc/%d/maps", pid);
    FILE *file = fopen(pathToOpen, "r");
    fprintf(out,"\nMAPS:\n");
    int lengthOfRead;
    fprintf(out, "%s\t\t%s\t\t%s\n", "bytes", "in pages", "addr-addr");
    while ((line_length = getline(&line, &line_size, file)), !feof(file))
    {
    
        if (!feof(file) && line_length == -1)
        {
            perror("getline():");
            fclose(file);
            free(line);
            exit(1);
        }
        sscanf(line, "%x-%x", &start_addr, &end_addr);
        total_page_num += end_addr - start_addr;
        fprintf(out, "%d\t\t%d\t\t\t%s", (end_addr - start_addr), (end_addr - start_addr) / page_size, line);
    }
    fclose(file);
    fprintf(out, "Total size (in bytes): %ld\n", total_page_num);
    total_page_num /= page_size;
    fprintf(out, "Total size (in pages): %ld\n", total_page_num);
}

void fprintf_comm_info(const int pid, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%d/comm", pid);

    FILE *f = fopen(path, "r");

    char buf[BUFSIZE + 1];
    int len = fread(buf, 1, BUFSIZE, f);
    buf[len] = '\0';

    fprintf(out, "COMM: ");
    fprintf(out, "%s\n", buf);

    fclose(f);
}

void fprintf_task_info(const int pid, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%d/task", pid);

    DIR *dp = opendir(path);
    struct dirent *dirp;
    char full_path[BUFSIZE + 1];
    char buf[BUFSIZE + 1];
    fprintf(out, "TASK:\n");

    while ((dirp = readdir(dp)) != NULL)
    {
        if ((strcmp(dirp->d_name, ".") != 0)
         && (strcmp(dirp->d_name, "..") != 0))
            fprintf(out, "%s\n", dirp->d_name);
    }
    fprintf(out, "\n");

    closedir(dp);
}

int get_pid(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Введите pid процесса для исследования\n");
        exit(1);
    }

    int pid = atoi(argv[1]);
    char check_dir[PATH_MAX];
    snprintf(check_dir, PATH_MAX, "/proc/%d", pid);
    if (!pid || access(check_dir, F_OK)) {
        printf("Директории для введенного pid не найдено\n");
        exit(1);
    }

    return pid;
}

typedef struct {
    uint64_t pfn : 55;
    unsigned int soft_dirty : 1;
    unsigned int file_page : 1;
    unsigned int swapped : 1;
    unsigned int present : 1;
} PagemapEntry;

/* Parse the pagemap entry for the given virtual address.
 *
 * @param[out] entry      the parsed entry
 * @param[in]  pagemap_fd file descriptor to an open /proc/pid/pagemap file
 * @param[in]  vaddr      virtual address to get entry for
 * @return 0 for success, 1 for failure
 */
int pagemap_get_entry(PagemapEntry *entry, int pagemap_fd, uintptr_t vaddr)
{
    size_t nread;
    ssize_t ret;
    uint64_t data;

    nread = 0;
    while (nread < sizeof(data)) {
        ret = pread(pagemap_fd, ((uint8_t*)&data) + nread, sizeof(data) - nread,
                (vaddr / sysconf(_SC_PAGE_SIZE)) * sizeof(data) + nread);
        nread += ret;
        if (ret <= 0) {
            return 1;
        }
    }
    entry->pfn = data & (((uint64_t)1 << 55) - 1);
    entry->soft_dirty = (data >> 55) & 1;
    entry->file_page = (data >> 61) & 1;
    entry->swapped = (data >> 62) & 1;
    entry->present = (data >> 63) & 1;
    return 0;
}

/* Convert the given virtual address to physical using /proc/PID/pagemap.
 *
 * @param[out] paddr physical address
 * @param[in]  pid   process to convert for
 * @param[in] vaddr virtual address to get entry for
 * @return 0 for success, 1 for failure
 */
int virt_to_phys_user(uintptr_t *paddr, pid_t pid, uintptr_t vaddr)
{
    char pagemap_file[BUFSIZ];
    int pagemap_fd;

    snprintf(pagemap_file, sizeof(pagemap_file), "/proc/%ju/pagemap", (uintmax_t)pid);
    pagemap_fd = open(pagemap_file, O_RDONLY);
    if (pagemap_fd < 0) {
        return 1;
    }
    PagemapEntry entry;
    if (pagemap_get_entry(&entry, pagemap_fd, vaddr)) {
        return 1;
    }
    close(pagemap_fd);
    *paddr = (entry.pfn * sysconf(_SC_PAGE_SIZE)) + (vaddr % sysconf(_SC_PAGE_SIZE));
    return 0;
}

int fprintf_pagemap_info(const int pid, FILE *out) {
    char buffer[BUFSIZ];
    char maps_file[BUFSIZ];
    char pagemap_file[BUFSIZ];
    int maps_fd;
    int offset = 0;
    int pagemap_fd;

    snprintf(maps_file, sizeof(maps_file), "/proc/%ju/maps", (uintmax_t)pid);
    snprintf(pagemap_file, sizeof(pagemap_file), "/proc/%ju/pagemap", (uintmax_t)pid);
    maps_fd = open(maps_file, O_RDONLY);
    if (maps_fd < 0) {
        perror("open maps");
        return EXIT_FAILURE;
    }
    pagemap_fd = open(pagemap_file, O_RDONLY);
    if (pagemap_fd < 0) {
        perror("open pagemap");
        return EXIT_FAILURE;
    }
    fprintf(out, "\nPAGEMAP:\n");
    int r = 0;
    fprintf(out, "addr\t\t\tpfn\tsoft-dirty\tfile/shared\tswapped\tpresent\tlibrary\n");
    for (;;) {
        ssize_t length = read(maps_fd, buffer + offset, sizeof buffer - offset);
        if (length <= 0) break;
        length += offset;
        for (size_t i = offset; i < (size_t)length; i++) {
            uintptr_t low = 0, high = 0;
            if (buffer[i] == '\n' && i) {
                const char *lib_name;
                size_t y;
                /* Parse a line from maps. Each line contains a range that contains many pages. */
                {
                    size_t x = i - 1;
                    while (x && buffer[x] != '\n') x--;
                    if (buffer[x] == '\n') x++;
                    while (buffer[x] != '-' && x < sizeof buffer) {
                        char c = buffer[x++];
                        low *= 16;
                        if (c >= '0' && c <= '9') {
                            low += c - '0';
                        } else if (c >= 'a' && c <= 'f') {
                            low += c - 'a' + 10;
                        } else {
                            break;
                        }
                    }
                    while (buffer[x] != '-' && x < sizeof buffer) x++;
                    if (buffer[x] == '-') x++;
                    while (buffer[x] != ' ' && x < sizeof buffer) {
                        char c = buffer[x++];
                        high *= 16;
                        if (c >= '0' && c <= '9') {
                            high += c - '0';
                        } else if (c >= 'a' && c <= 'f') {
                            high += c - 'a' + 10;
                        } else {
                            break;
                        }
                    }
                    lib_name = 0;
                    for (int field = 0; field < 4; field++) {
                        x++;
                        while(buffer[x] != ' ' && x < sizeof buffer) x++;
                    }
                    while (buffer[x] == ' ' && x < sizeof buffer) x++;
                    y = x;
                    while (buffer[y] != '\n' && y < sizeof buffer) y++;
                    buffer[y] = 0;
                    lib_name = buffer + x;
                }
                /* Print Amount of Pages */
                buf[r++] =  (uintmax_t)((high - low) / sysconf(_SC_PAGE_SIZE));
                fprintf(out, "Pages: %ju\n", (uintmax_t)((high - low) / sysconf(_SC_PAGE_SIZE)));
                
                /* Get info about all pages in this page range with pagemap. */
                {
                    PagemapEntry entry;
                    for (uintptr_t addr = low; addr < high; addr += sysconf(_SC_PAGE_SIZE)) {
                        /* TODO always fails for the last page (vsyscall), why? pread returns 0. */
                        if (!pagemap_get_entry(&entry, pagemap_fd, addr)) {
                            fprintf(out, "%jx\t%jx\t\t\t%u\t\t\t%u\t\t%u\t\t%u\t%s\n",
                                (uintmax_t)addr,
                                (uintmax_t)entry.pfn,
                                entry.soft_dirty,
                                entry.file_page,
                                entry.swapped,
                                entry.present,
                                lib_name
                            );
                        }
                    }
                }
                buffer[y] = '\n';
            }
        }
    }
    close(maps_fd);
    close(pagemap_fd);
    
    return 0;
}


int main (int argc, char **argv) 
{
    int id = get_pid(argc, argv);
    char *name = argv[2];
    FILE *f = fopen(name, "w");    

    fprintf_cmdline_info(id, f);
    fprintf_cwd_info(id, f);
    fprintf_exe_info(id, f);
    fprintf_comm_info(id, f);
    fprintf_task_info(id, f);
    fprintf_root_info(id, f);
    fprintf_environ_info(id, f);
    fprintf_fd_info(id, f);
    fprintf_stat_info(id, f);
    fprintf_statm_info(id, f);
    fprintf_io_info(id, f);
    fprintf_maps_info(id, f);
    fprintf_pagemap_info(id, f);
    
    fclose(f);

    return 0; 
}

