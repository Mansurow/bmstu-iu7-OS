#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include "utils.h"

#define BUFSIZE 1000

void fprintf_cmdline_info(const char *pid, FILE *out) 
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/cmdline", pid);

    FILE *f = fopen(path, "r");

    char buf[BUFSIZE + 1];
    int len = fread(buf, 1, BUFSIZE, f);
    buf[len] = '\0';

    fprintf(out, "CMDLINE: ");
    fprintf(out, "%s\n\n", buf);

    fclose(f);
}

void fprintf_cwd_info(const char *pid, FILE *out) 
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/cwd", pid);

    char buf[BUFSIZE + 1];
    int len = readlink(path, buf, BUFSIZE);
    buf[len] = '\0';

    fprintf(out, "CWD: ");
    fprintf(out, "%s\n\n", buf);
}

void fprintf_environ_info(const char *pid, FILE *out) 
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/environ", pid);

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

void fprintf_exe_info(const char *pid, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/exe", pid);

    char buf[BUFSIZE + 1];
    int len = readlink(path, buf, BUFSIZE);
    buf[len] = '\0';

    fprintf(out, "EXE: ");
    fprintf(out, "%s\n\n", buf);
}

void fprintf_root_info(const char *pid, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/root", pid);

    char buf[BUFSIZE + 1];
    int len = readlink(path, buf, BUFSIZE);
    buf[len] = '\0';

    fprintf(out, "ROOT: ");
    fprintf(out, "%s\n\n", buf);
}


void fprintf_fd_info(const char *pid, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/fd", pid);

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

void fprintf_stat_info(const char *pid, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/stat", pid);

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

void fprintf_statm_info(const char *pid, FILE *out) 
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/statm", pid);

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

void fprintf_io_info(const char *pid, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/io", pid);

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

void fprintf_maps_info(const char *pid, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/maps", pid);

    FILE *f = fopen(path, "r");
    fprintf(out, "MAPS:\n");

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

void fprintf_comm_info(const char *pid, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/comm", pid);

    FILE *f = fopen(path, "r");

    char buf[BUFSIZE + 1];
    int len = fread(buf, 1, BUFSIZE, f);
    buf[len] = '\0';

    fprintf(out, "COMM: ");
    fprintf(out, "%s\n", buf);

    fclose(f);
}

void fprintf_task_info(const char *pid, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/task", pid);

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

void fprintf_pagemap_info(const char *pid, FILE *out) {
    DIR *d;
    struct dirent *dir;

    char pathToOpen[PATH_MAX];
    snprintf(pathToOpen, PATH_MAX, "/proc/%s/pagemap", pid);

    fprintf(out, "PAGEMAP:\n");

    d = opendir(pathToOpen);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            fprintf(out, "%s\n", dir->d_name);
        }
        closedir(d);
    }
}

int main (int argc, char **argv) 
{
    char *id = argv[1];

    fprintf_cmdline_info(id, stdout);
    fprintf_cwd_info(id, stdout);
    fprintf_exe_info(id, stdout);
    fprintf_comm_info(id, stdout);
    fprintf_task_info(id, stdout);
    fprintf_root_info(id, stdout);
    fprintf_environ_info(id, stdout);
    fprintf_fd_info(id, stdout);
    fprintf_stat_info(id, stdout);
    fprintf_statm_info(id, stdout);
    fprintf_io_info(id, stdout);
    fprintf_maps_info(id, stdout);
    fprintf_pagemap_info(id, stdout);
    
    return -1; 
}

