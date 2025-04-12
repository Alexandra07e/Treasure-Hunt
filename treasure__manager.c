#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#define nameSIZE 100
#define clueTextSIZE 200
#define pathSIZE 512

typedef struct TREASURE {
    int id;
    char userName[nameSIZE];
    double latitude, longitude;
    char clueText[clueTextSIZE];
    int value;
} TREASURE;

struct stat tr;
struct stat lg;
struct stat st;

void check_directory_files(const char *hunt) {
    char path_dir[pathSIZE], path_tr[pathSIZE], path_log[pathSIZE];
    snprintf(path_dir, sizeof(path_dir), "%s", hunt);
    snprintf(path_tr, sizeof(path_tr), "%s/treasure.dat", hunt);
    snprintf(path_log, sizeof(path_log), "%s/log.txt", hunt);

    if (stat(path_dir, &st) == -1) {
        perror("directory doesn't exist");
        exit(1);
    }

    if (stat(path_tr, &tr) == -1) {
        perror("treasure file doesn't exist");
        exit(1);
    }

    if (stat(path_log, &lg) == -1) {
        perror("log file doesn't exist");
        exit(1);
    }
}

void init(const char *hunt) {
    char path_tr[pathSIZE], path_log[pathSIZE], link_path[pathSIZE];
    snprintf(path_tr, sizeof(path_tr), "%s/treasure.dat", hunt);
    snprintf(path_log, sizeof(path_log), "%s/log.txt", hunt);
    snprintf(link_path, sizeof(link_path), "logged_hunt-%s", hunt);

    int tr = creat(path_tr, 0777);
    if (tr == -1) {
        perror("error creating the treasure file");
        exit(1);
    }
    int log = creat(path_log, 0777);
    if (log == -1) {
        perror("error creating the log file");
        exit(1);
    }

    if (close(tr) == -1) {
        perror("error closing the treasure file");
        exit(1);
    }

    if (close(log) == -1) {
        perror("error closing the log file");
        exit(1);
    }

    check_directory_files(hunt);

    if (access(link_path, F_OK) == -1) {
        if (symlink(path_log, link_path) == -1) {
            perror("error creating the symbolic link");
            exit(1);
        }
    }
}

void modify_log(const char *hunt, char *message) {
    char path1[pathSIZE];
    char path2[pathSIZE];
    snprintf(path1, sizeof(path1), "%s/log.txt", hunt);
    snprintf(path2, sizeof(path2), "%s/treasure.dat", hunt);

    check_directory_files(hunt);

    int log = open(path1, O_RDWR | O_APPEND, 0777);
    if (log == -1) {
        perror("trouble opening the log file");
        exit(1);
    }

    char buffer[256] = {0};
    time_t now = time(NULL);

    snprintf(buffer, sizeof(buffer), "<%s>  --%s\n", strtok(ctime(&now), "\n"), message);
    if (write(log, buffer, strlen(buffer)) != strlen(buffer)) {
        perror("error writing in log");
        exit(1);
    }

    if (close(log) == -1) {
        perror("error closing the log file");
        exit(1);
    }
}

TREASURE readTreasure() {
    TREASURE t;

    char buffer_out[4096] = {0};
    char buffer_in[4096] = {0};

    strcpy(buffer_out, "Type in id:");
    write(1, buffer_out, strlen(buffer_out));
    read(0, buffer_in, 5);
    t.id = strtol(buffer_in, NULL, 10);
    if (t.id <= 0) {
        perror("invalid id\n");
        exit(1);
    }

    strcpy(buffer_out, "Type in userName:");
    write(1, buffer_out, strlen(buffer_out));
    read(0, buffer_in, nameSIZE);
    buffer_in[strcspn(buffer_in, "\n")] = '\0';
    strcpy(t.userName, buffer_in);

    strcpy(buffer_out, "Type in latitude:");
    write(1, buffer_out, strlen(buffer_out));
    read(0, buffer_in, 10);
    t.latitude = strtod(buffer_in, NULL);
    if (t.latitude == 0 || t.latitude > 90 || t.latitude < -90) {
        perror("invalid latitude\n");
        exit(1);
    }

    strcpy(buffer_out, "Type in longitude:");
    write(1, buffer_out, strlen(buffer_out));
    read(0, buffer_in, 10);
    t.longitude = strtod(buffer_in, NULL);
    if (t.longitude == 0 || t.longitude > 180 || t.longitude < -180) {
        perror("invalid id\n");
        exit(1);
    }

    strcpy(buffer_out, "Type in clue Text:");
    write(1, buffer_out, strlen(buffer_out));
    read(0, buffer_in, clueTextSIZE);
    buffer_in[strcspn(buffer_in, "\n")] = '\0';
    strcpy(t.clueText, buffer_in);


    strcpy(buffer_out, "Type in value:");
    write(1, buffer_out, strlen(buffer_out));
    read(0, buffer_in, 10);
    t.value = strtol(buffer_in, NULL, 10);
    if (t.value <= 0) {
        perror("invalid value\n");
        exit(1);
    }

    write(1, "\n", 1);
    return t;
}

void add_treasure(const char *hunt) {
    if (stat(hunt, &st) == -1) {
        perror("directory not created");
        if (mkdir(hunt, 0777) == -1) {
            perror("trouble with the directory!");
            exit(-2);
        }
        init(hunt);
    }
    int tr = -1;

    char path_tr[pathSIZE];

    snprintf(path_tr, sizeof(path_tr), "%s/treasure.dat", hunt);

    if ((tr = open(path_tr, O_RDWR | O_APPEND, 0777)) == -1) {
        perror("trouble opening the treasure file");
        exit(1);
    }

    TREASURE t = readTreasure();

    ssize_t leg = write(tr, &t, sizeof(t));
    if (leg != sizeof(t)) {
        perror("trouble writing in the treasure file");
        close(tr);
        exit(1);
    }
    if (close(tr) == -1) {
        perror("error closing the treasure file");
        exit(1);
    }

    char msg[30] = {0};
    snprintf(msg, sizeof(msg), "added treasure\n");
    modify_log(hunt, msg);
}

void list_treasures(const char *hunt) {
    char buffer[4096] = {0};
    char path_tr[pathSIZE];

    check_directory_files(hunt);
    snprintf(path_tr, sizeof(path_tr), "%s/treasure.dat", hunt);

    int len = snprintf(buffer, sizeof(buffer), "Hunt name: %s\nTotal file size: %ld\nLast modification time: %s\nList of treasures:\n", hunt, tr.st_size, ctime(&tr.st_mtime));
    if (write(1, buffer, len) != len) {
        perror("error writing to output stream");
        exit(2);
    }

    // Reading the treasures
    int tr = open(path_tr, O_RDONLY, 0777);
    if (tr == -1) {
        perror("trouble opening the treasure file");
        exit(1);
    }

    TREASURE t;
    strcpy(buffer, "");
    while ((read(tr, &t, sizeof(t))) == sizeof(t)) {
        int len = snprintf(buffer, sizeof(buffer), "Id: %d\nUser name: %s\nLatitude: %.2lf\nLongitude: %.2lf\nClue text: %s\nValue: %d\n\n", t.id, t.userName, t.latitude, t.longitude, t.clueText, t.value);
        if (write(1, buffer, len) != len) {
            perror("error writing to output stream");
            exit(2);
        }
        strcpy(buffer, "");
    }
    write(1, "\n", 1);

    if (close(tr) == -1) {
        perror("error closing the treasure file");
        exit(1);
    }

    char msg[30] = {0};
    snprintf(msg, sizeof(msg), "listed treasures\n");
    modify_log(hunt, msg);
}

void view_treasure(const char *hunt, int treasure_id) {
    check_directory_files(hunt);

    char path_tr[pathSIZE];
    snprintf(path_tr, sizeof(path_tr), "%s/treasure.dat", hunt);
    int tr = open(path_tr, O_RDONLY, 0777);
    if (tr == -1) {
        perror("error opening the treasure file");
        exit(1);
    }

    TREASURE t;
    char buffer[4096] = {0};

    int find = 0;
    while (read(tr, &t, sizeof(t)) == sizeof(t)) {
        if (t.id == treasure_id) {
            int len = snprintf(buffer, sizeof(buffer), "The treasure with the id %d:\nUser name: %s\nLatitude: %.2lf\nLongitude: %.2lf\nClue text: %s\nValue: %d\n", t.id, t.userName, t.latitude, t.longitude, t.clueText, t.value);
            if (write(1, buffer, len) != len) {
                perror("error writing to output stream");
                exit(2);
            }
            find = 1;
            char msg[30] = {0};
            snprintf(msg, sizeof(msg), "viewed treasure %d\n", treasure_id);
            modify_log(hunt, msg);
            break;
        }
    }
    if (find == 0) {
        strcpy(buffer, "INDEX NOT FOUND");
        write(1, buffer, strlen(buffer));
    }
    write(1, "\n", 1);
    if (close(tr) == -1) {
        perror("error closing the treasure file");
        exit(1);
    }
}

void remove_hunt(const char *hunt) {
    check_directory_files(hunt);
    char path_tr[pathSIZE], path_log[pathSIZE], link_path[pathSIZE];

    snprintf(path_tr, sizeof(path_tr), "%s/treasure.dat", hunt);
    snprintf(path_log, sizeof(path_log), "%s/log.txt", hunt);
    snprintf(link_path, sizeof(link_path), "logged_hunt-%s", hunt);

    if (remove(path_tr) == -1) {
        perror("trouble with deleting the treasure file");
        exit(-1);
    }

    if (unlink(link_path) == -1) {
        perror("trouble with deleting the symbolic link");
        exit(-1);
    }

    if (remove(path_log) == -1) {
        perror("trouble with deleting the log file");
        exit(-1);
    }

    if (rmdir(hunt) == -1) {
        perror("trouble with deleting the directory");
        exit(-1);
    }

    char msg[50] = {0};
    strcpy(msg, "\nDIRECTORY DELETED SUCCESSFULLY\n");
    write(1, msg, strlen(msg));
}

void remove_treasure(const char *hunt, int treasure_id) {
    check_directory_files(hunt);
    char path_tr[pathSIZE], path_clone[pathSIZE];
    snprintf(path_tr, sizeof(path_tr), "%s/treasure.dat", hunt);
    snprintf(path_clone, sizeof(path_clone), "%s/clone.dat", hunt);

    int tr = open(path_tr, O_RDWR | O_APPEND, 0777);
    if (tr == -1) {
        perror("trouble opening the treasure file");
        exit(1);
    }

    int clone = open(path_clone, O_CREAT | O_RDWR | O_APPEND, 0777);
    if (clone == -1) {
        perror("error creating the path to the clone tr");
        exit(1);
    }

    TREASURE t;
    int find = 0;
    while (read(tr, &t, sizeof(t)) == sizeof(t)) {
        if (t.id != treasure_id) {
            write(clone, &t, sizeof(t));
        } else {
            find = 1;
        }
    }

    if (close(tr) == -1) {
        perror("error closing the treasure file");
        exit(1);
    }
    if (close(clone) == -1) {
        perror("error closing the treasure-clone file");
        exit(1);
    }

    if (find == 1) {
        if (remove(path_tr) == -1) {
            perror("error removing the treasure-original file");
            exit(1);
        }
        if (rename(path_clone, path_tr) == -1) {
            perror("trouble renaming the file");
            exit(-1);
        }
        printf("Treasure successfully deleted!!\n");
        char msg[30] = {0};
        snprintf(msg, sizeof(msg), "removed treasure %d\n", treasure_id);
        modify_log(hunt, msg);
    } else {
        write(1, "ID treasure not found\n", strlen("ID treasure not found\n"));
        if (remove(path_clone) == -1) {
            perror("error removing the treasure-clone file");
            exit(1);
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        perror("error with the command arguments");
        exit(1);
    }

    if (strcmp(argv[2], "add") == 0) {
        add_treasure(argv[1]);
    } else if (strcmp(argv[2], "list") == 0) {
        list_treasures(argv[1]);
    } else if (strcmp(argv[2], "view") == 0) {
        int treasure_id;
        char buffer[256] = {0};
        int len = snprintf(buffer, sizeof(buffer), "Type in the ID for the treasure you want to view: ");
        write(1, buffer, len);
        strcpy(buffer, "");
        read(0, buffer, 10);
        treasure_id = atoi(buffer);
        view_treasure(argv[1], treasure_id);
    } else if (strcmp(argv[2], "remove_treasure") == 0) {
        int treasure_id;
        char buffer[256] = {0};
        int len = snprintf(buffer, sizeof(buffer), "Type in the ID for the treasure you want to remove: ");
        write(1, buffer, len);
        strcpy(buffer, "");
        read(0, buffer, 10);
        treasure_id = atoi(buffer);
        remove_treasure(argv[1], treasure_id);
    } else if (strcmp(argv[2], "remove_hunt") == 0) {
        remove_hunt(argv[1]);
    } else {
        perror("invalid argument");
        exit(1);
    }

    return 0;
}