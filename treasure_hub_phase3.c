#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>

struct stat trr;
struct stat st;

int monitor_pid = -1;
int monitor_run = 0;
int monitor_stop = 0;

int pfd_monitor[2];

#define nameSIZE 100
#define clueTextSIZE 200
#define pathSIZE 1024

typedef struct TREASURE {
    int id;
    char userName[nameSIZE];
    double latitude, longitude;
    char clueText[clueTextSIZE];
    int value;
} TREASURE;

void list_hunts(const char* directory) {
    DIR* d = NULL;
    struct dirent* file;
    char path[pathSIZE], path_tr[pathSIZE];
    int count_treasures = 0;

    if ((d = opendir(directory)) == NULL) {
        perror("error opening the directory:(");
        return;
    }

    while ((file = readdir(d)) != NULL) {
        if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0 || strcmp(file->d_name, ".vscode") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", directory, file->d_name);
        if (lstat(path, &st) == -1)
            continue;

        if (S_ISDIR(st.st_mode)) {
            printf("Hunt name: %s --", file->d_name);
            list_hunts(path);
        }
        else if (S_ISREG(st.st_mode)) {
            TREASURE t;
            snprintf(path_tr, sizeof(path_tr), "%s/treasure.dat", directory);
            if (strcmp(path, path_tr) == 0) {
                int tr = open(path, O_RDONLY, 0777);
                if (tr == -1) {
                    perror("couldn't open the treasure file!");
                    return;
                }
                count_treasures = 0;
                while (read(tr, &t, sizeof(t)) == sizeof(t))
                    count_treasures++;
                if (close(tr) == -1) {
                    perror("couldn't close the treasure file");
                    return;
                }
                printf("%d treasure files\n", count_treasures);
            }
        }
    }

    if (closedir(d) == -1) {
        perror("couldn't close the directory");
        return;
    }
}

void list_treasures(const char* hunt) {
    char buffer[4096] = { 0 };
    char path_tr[pathSIZE], path_dir[pathSIZE];

    snprintf(path_dir, sizeof(path_dir), "%s", hunt);
    if (lstat(path_dir, &st) == -1) {
        write(1,"directory doesn't exist??",strlen("directory doesn't exist??"));
        return;
    }

    snprintf(path_tr, sizeof(path_tr), "%s/treasure.dat", hunt);

    if (lstat(path_tr, &trr) == -1) {
        write(1,"treasure file doesn't exist",strlen("treasure file doesn't exist"));
        return;
    }

    int len = snprintf(buffer, sizeof(buffer),
        "Hunt name: %s\nTotal file size: %ld\nLast modification time: %s\nList of treasures:\n",
        hunt, trr.st_size, ctime(&trr.st_mtime));

    if (write(1, buffer, len) != len) {
        perror("error writing to output stream");
        exit(2);
    }

    int tr = open(path_tr, O_RDONLY, 0777);
    if (tr == -1) {
        write(1,"trouble opening the treasure file",strlen("trouble opening the treasure file"));
        return;
    }

    TREASURE t;
    strcpy(buffer, "");
    while (read(tr, &t, sizeof(t)) == sizeof(t)) {
        int len = snprintf(buffer, sizeof(buffer),
            "Id: %d\nUser name: %s\nLatitude: %.2lf\nLongitude: %.2lf\nClue text: %s\nValue: %d\n\n",
            t.id, t.userName, t.latitude, t.longitude, t.clueText, t.value);
        if (write(1, buffer, len) != len) {
            perror("error writing to output stream");
            exit(2);
        }
        strcpy(buffer, "");
    }

    write(1, "\n", 1);

    if (close(tr) == -1) {
        write(1,"trouble closing the treasure file",strlen("trouble closing the treasure file"));
        return;
    }
}

void view_treasure(const char* hunt, int treasure_id) {
    char path_tr[pathSIZE], path_dir[pathSIZE];
    snprintf(path_dir, sizeof(path_dir), "%s", hunt);

    if (lstat(path_dir, &st) == -1) {
        write(1,"directory doesn't exist??",strlen("directory doesn't exist??"));
        return;
    }

    snprintf(path_tr, sizeof(path_tr), "%s/treasure.dat", hunt);

    if (lstat(path_tr, &trr) == -1) {
        write(1,"treasure file doesn't exist",strlen("treasure file doesn't exist"));
        return;
    }

    int tr = open(path_tr, O_RDONLY, 0777);
    if (tr == -1) {
        write(1,"trouble opening the treasure file",strlen("trouble opening the treasure file"));
        return;
    }

    TREASURE t;
    char buffer[4096] = { 0 };
    int find = 0;

    while (read(tr, &t, sizeof(t)) == sizeof(t)) {
        if (t.id == treasure_id) {
            int len = snprintf(buffer, sizeof(buffer),
                "The treasure with the id %d:\nUser name: %s\nLatitude: %.2lf\nLongitude: %.2lf\nClue text: %s\nValue: %d\n",
                t.id, t.userName, t.latitude, t.longitude, t.clueText, t.value);
            if (write(1, buffer, len) != len) {
                perror("error writing to output stream");
                exit(2);
            }
            find = 1;
            break;
        }
    }

    if (!find) {
        strcpy(buffer, "ID treasure not found\n");
        write(1, buffer, strlen(buffer));
    }

    write(1, "\n", 1);

    if (close(tr) == -1) {
        write(1,"trouble closing the treasure file",strlen("trouble closing the treasure file"));
        return;
    }
}

void calculate_score(const char *directory){
    DIR* d = NULL;
    struct dirent *file;

    if ((d = opendir(directory)) == NULL){
        perror("Failed to open current directory");
        return;
    }

    while ((file = readdir(d)) != NULL){
        if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0 || strcmp(file->d_name, ".vscode") == 0) continue;
            if (file->d_type == DT_DIR){
                int pfd_monitor_second[2];
                if (pipe(pfd_monitor_second) == -1) {
                    perror("error with the pipe");
                    return;
                }

                int pid = fork();
                    if (pid == -1) {
                        perror("error using fork");
                        exit(-1);
                    } 
                else if (pid == 0) {
                //child child (nephew) process 
                    close(pfd_monitor_second[0]); // close read
                    dup2(pfd_monitor_second[1], 1); // write output to pipe
                    
                    execlp("./phase3_2", "phase3_2", file->d_name, NULL);
                    perror("execlp failed");
                    exit(1);
                } 
                else {
                // parent to the nephew => child 
                    close(pfd_monitor_second[1]); // close write
                    char buffer[256];
                    printf("\nHunt %s --\n", file->d_name);
                    int size;
                    while ((size = read(pfd_monitor_second[0], buffer, sizeof(buffer))) > 0) {
                        buffer[size] = '\0';
                        printf("%s", buffer);
                    }
                    close(pfd_monitor_second[0]);
                    waitpid(pid, NULL, 0); //wait for a child process to end for nice aand fault-less output
                }
            }
    }

    if (closedir(d) == -1){
        perror("couldn't close the directory");
        return;
    }
}

void handle_signals(int signal) {
    usleep(100000);
    int file = open("commands.txt", O_RDONLY, 0777);
    if (file == -1) {
        perror("error opening the file");
        return;
    }

    char buffer[256] = { 0 };
    int size = read(file, buffer, sizeof(buffer));
    buffer[size] = '\0';
    if (size <= 0) {
        perror("error reading from the command file");
        close(file);
        exit(1);
    }

    if (close(file) == -1) {
        perror("error closing the command file");
        return;
    }

    char* p = strtok(buffer, " ");
    if (!p) return;

    if (strcmp(p, "list_hunts") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("couldn't get the current directory");
            return;
        }
        list_hunts(cwd);
    }
    else if (strcmp(p, "list_treasures") == 0) {
        char* hunt = strtok(NULL, " ");
        if (hunt)
            list_treasures(hunt);
        else
            write(1, "Hunt ID doesn't exist\n", strlen("Hunt ID doesn't exist\n"));
    }
    else if (strcmp(p, "view_treasure") == 0) {
        char* hunt = strtok(NULL, " ");
        char* treasure_id = strtok(NULL, " ");
        if (hunt && treasure_id) {
            view_treasure(hunt, strtol(treasure_id, NULL, 10));
        }
        else {
            write(1, "Hunt ID and/or treasure ID don't exist\n", strlen("Hunt ID and/or treasure ID don't exist\n"));
        }
    }
    else if(strcmp(p,"calculate_score")==0){
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("couldn't get the current directory");
            return;
        }
        calculate_score(cwd);
    }
}

void handle_signal_stop(int signal) {
    write(1, "Monitor stopping...\n", strlen("Monitor stopping...\n"));
    usleep(2000000);
    exit(0);
}

void start_monitor() {
    if (monitor_run != 0) {
        write(1, "Monitor already running!!\n", strlen("Monitor already running!!\n"));
        return;
    }

    if (pipe(pfd_monitor) < 0){
        perror("error with the pipe");
        exit(-1);
    }

    int pid = fork();
    if (pid < 0) {
        perror("error creating child!");
        exit(1);
    }

    if (pid == 0) {
        //child process - capatul de scriere - monitor

        close(pfd_monitor[0]);
        dup2(pfd_monitor[1], 1); //redirectionam de la stdout in main
        close(pfd_monitor[1]);


        struct sigaction sa1;
        sa1.sa_handler = handle_signals;
        sigemptyset(&sa1.sa_mask);
        sa1.sa_flags = SA_RESTART;
        if (sigaction(SIGUSR1, &sa1, NULL) == -1) {
            perror("error with sigaction for SIGUSR1");
            exit(-1);
        }

        struct sigaction sa2;
        sa2.sa_handler = handle_signal_stop;
        sigemptyset(&sa2.sa_mask);
        sa2.sa_flags = SA_RESTART;
        if (sigaction(SIGUSR2, &sa2, NULL) == -1) {
            perror("error with sigaction for SIGUSR2");
            exit(-1);
        }

        while (1)
            pause();

        exit(0);
    }
    else {
        //parent process - capatul de citire 
        close(pfd_monitor[1]);
        monitor_pid = pid;
        monitor_run = 1;
        monitor_stop = 0;
        printf("At PID %d start_monitor command was given\n\n", monitor_pid);
    }
}

void handle_signal_stopMonitor(int signal) {
    int status; 
    int pidd;

    while ((pidd = waitpid(-1, &status, 0)) > 0) {
        if (pidd == monitor_pid) {
            printf("Monitor stopped with the status %d\n\n", WEXITSTATUS(status));
            monitor_pid = -1;
            monitor_run = 0;
            monitor_stop = 1;
        }
    }
}

void stop_monitor() {
    struct sigaction sa_stop;
    sa_stop.sa_handler = handle_signal_stopMonitor;
    sigemptyset(&sa_stop.sa_mask);
    sa_stop.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa_stop, NULL) == -1) {
        perror("error with sigaction for SIGCHLD");
        exit(-1);
    }
}

void write_to_file(char* command) {
    int file = open("commands.txt", O_CREAT | O_RDWR | O_TRUNC, 0777);
    if (file == -1) {
        perror("couldn't open the command file");
        exit(1);
    }

    strcat(command, " ");
    write(file, command, strlen(command));
    if (close(file) == -1) {
        perror("error closing the command file");
        exit(1);
    }

    if (kill(monitor_pid, SIGUSR1) == -1) {
        perror("SIGSUR1 is not a murderer");
        exit(1);
    }
    usleep(2000000);

    char buffer[4096] = { 0 };
    int size = read(pfd_monitor[0], buffer, sizeof(buffer));
    if (size > 0){
        buffer[size] = '\0';
        printf("%s\n", buffer);
    }
    else printf("trouble reading from pipe");
}


void menu_interactive() {
    printf("\n-------TREASURE HUB-------\n\n");
    printf(">>start_monitor\n");
    printf(">>list_hunts\n");
    printf(">>list_treasures <huntID>\n");
    printf(">>view_treasure <huntID> <treasureID>\n");
    printf(">>calculate_score\n");
    printf(">>stop_monitor\n");
    printf(">>exit\n");
}

int main() {
    char command[50];
    while (1) {
        menu_interactive();
        fflush(stdout);
        int size = read(0, command, sizeof(command));
        if (size <= 0) {
            perror("couldn't read from stdin");
            exit(-1);
        }
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "start_monitor") == 0) {
            start_monitor();
        }
        else if (strcmp(command, "list_hunts") == 0 ||
            strncmp(command, "list_treasures", 14) == 0 ||
            strncmp(command, "view_treasure", 13) == 0) {
            if (monitor_run == 0)
                printf("Monitor hasn't started yet:(\n");
            else if (monitor_stop == 1)
                printf("Monitor has stopped :(\n");
            else {
                write_to_file(command);
            }
        }
        else if (strncmp(command, "calculate_score", 15) == 0){
            if (monitor_run == 0)
                printf("Monitor hasn't started yet:(\n");
            else if (monitor_stop == 1)
                printf("Monitor has stopped :(\n");
            else
                write_to_file(command);
        }
        else if (strcmp(command, "stop_monitor") == 0) {
            if (monitor_stop == 1)
                printf("Monitor has already stopped:(\n");
            else if (monitor_run == 0)
                printf("Monitor hasn't started yet:(\n");
            else {
                stop_monitor();
                if (kill(monitor_pid, SIGUSR2) == -1) {
                    perror("SIGSUR2 is not a murderer");
                    exit(1);
                }
                usleep(20000000);
            }
        }
        else if (strcmp(command, "exit") == 0) {
            if (monitor_run == 0 && monitor_stop == 0) {
                printf("Monitor hasn't even started, exiting program...\n");
                exit(0);
            }
            if (monitor_stop != 1) {
                printf("Monitor hasn't stopped, we cannot exit\n");
            }
            else {
                printf("Exiting program...\n");
                exit(0);
            }
        }
        else {
            printf("Unknown command lmao\n");
        }
        strcpy(command, "");
    }
    return 0;
}
