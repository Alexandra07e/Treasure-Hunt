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

#define nameSIZE 100
#define clueTextSIZE 200
#define pathSIZE 512
#define SIZE_CHUNK 50

typedef struct TREASURE {
    int id;
    char userName[nameSIZE];
    double latitude, longitude;
    char clueText[clueTextSIZE];
    int value;
} TREASURE;

typedef struct{
    int score;
    char user[nameSIZE];
    int checked;
} USER;

struct stat trr;

int main (int argc, char **argv){

    if (argc != 2){
        perror("not enough arguments,missing hunt");
        return -1;
    }

    TREASURE t;
    char path_tr[pathSIZE];
    snprintf (path_tr, sizeof(path_tr), "%s/treasure.dat", argv[1]);
    if (lstat(path_tr, &trr) == -1){
        perror("error getting the path");
        return -1;
    }

    int tr = open(path_tr, O_RDONLY, 0777);
    if (tr == -1){
        perror("errror opening the treasure file");
        return -1;
    }

    USER v[SIZE_CHUNK]= { 0 };

    int cnt = 0;
    while (read(tr, &t, sizeof(t)) == sizeof(t)){
        v[cnt].checked = 0;
        strcpy(v[cnt].user, t.userName);
        v[cnt].score = t.value;

        for (int i=0; i<cnt; i++){
            if (strcmp(v[i].user, t.userName) == 0){
                v[i].score += t.value;
                v[cnt].checked = 1;
            }
        }

        if (v[cnt].checked != 1)
            cnt++;

    }


    if (close(tr) == -1){
        perror("errror closing the treasure file");
        return -1;
    }

    for(int i=0; i<cnt; i++){
        printf("Treasure with user %s has the score %d\n",v[i].user,v[i].score);
    }

    return 0;
}