#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

#define MAX_CHILDREN 512

typedef struct {
    int pid;
    int ppid;
    char comm[256];
} ProcessInfo;

int compare_pid(const void *a, const void *b) {
    return ((ProcessInfo *)a)->pid - ((ProcessInfo *)b)->pid;
}

void print_process_tree(ProcessInfo processes[], int num_processes, int idx, int level, int show_pids, int is_last_child);

int main(int argc, char *argv[]) {
    int show_pids = 0;
    int numeric_sort = 0;

    // Check for command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            show_pids = 1;
        } else if (strcmp(argv[i], "-n") == 0) {
            numeric_sort = 1;
        } else if (strcmp(argv[i], "-V") == 0) {
            printf("pstree version 1.0\n");
            return 0;
        }
    }

    // Open /proc directory
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("Error opening /proc directory");
        return 1;
    }

    ProcessInfo processes[MAX_CHILDREN];
    int num_processes = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (isdigit(entry->d_name[0])) {
            char stat_file[256];
            int n = snprintf(stat_file, sizeof(stat_file), "/proc/%s/stat", entry->d_name);

            if (n >= sizeof(stat_file)) {
                fprintf(stderr, "Error: Path too long\n");
                continue;
            }

            FILE *file = fopen(stat_file, "r");
            if (!file) {
                perror("Error opening stat file");
                continue;
            }

            ProcessInfo process;
            if (fscanf(file, "%d %s %*c %d", &process.pid, process.comm, &process.ppid) != 3) {
                perror("Error reading stat file");
                fclose(file);
                continue;
            }

            fclose(file);

            processes[num_processes++] = process;
        }
    }

    // Sort processes
    if (numeric_sort) {
        qsort(processes, num_processes, sizeof(ProcessInfo), compare_pid);
    }

    // Print process tree
    for (int i = 0; i < num_processes; i++) {
        if (processes[i].ppid == 0) {
            print_process_tree(processes, num_processes, i, 0, show_pids, 1);
        }
    }

    closedir(dir);
    return 0;
}

void print_process_tree(ProcessInfo processes[], int num_processes, int idx, int level, int show_pids, int is_last_child) {
    for (int i = 0; i < level - 1; i++) {
        printf(is_last_child ? "    " : "│   ");
    }

    if (level > 0) {
        printf(is_last_child ? "└── " : "├── ");
    }

    printf("%s", processes[idx].comm);
    if (show_pids) {
        printf("(%d)", processes[idx].pid);
    }
    printf("\n");

    int num_children = 0;
    for (int i = idx + 1; i < num_processes; i++) {
        if (processes[i].ppid == processes[idx].pid) {
            num_children++;
        }
    }

    for (int i = idx + 1; i < num_processes; i++) {
        if (processes[i].ppid == processes[idx].pid) {
            print_process_tree(processes, num_processes, i, level + 1, show_pids, --num_children == 0);
        }
    }
}
