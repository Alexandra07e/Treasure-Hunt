#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
//#include "treasure_manager.c"

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


struct sigaction sa;
struct stat st;

void modify_commandFile(char *message){
    int command=open("commands.txt",O_CREAT|O_APPEND|O_RDWR,0777);
    if(command==-1){
        perror("error opening the command file");
        exit(1);
    }

    char buffer[256] = {0};
    time_t now = time(NULL);
    snprintf(buffer, sizeof(buffer), "<%s>  --%s\n", strtok(ctime(&now), "\n"), message);        
    write(command,buffer,strlen(buffer));

    if(close(command)==-1){
        perror("error closing the command file");
        exit(1);
    }
}

void handle_signal1(int signal){
    char message[100]={0};
    
    if(signal==SIGUSR1){
        strcpy(message,"Hunt status: prints hunts info");
        modify_commandFile(message);
    }
}

void start_monitor(){
    int pid=fork();
    if(pid<0){
        perror("error with FORK");
        exit(1);
    }

    if(pid==0){

        sa.sa_handler=handle_signal1;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags=0;

        sigaction(SIGUSR1,&sa,NULL);

        while(1)
            pause();
    } else{
        printf("At PID %d start_monitor command was given\n",pid);
        sleep(2);
        kill(pid,SIGUSR1);
        sleep(1);
    }
}

void parcurgere_director(){
    DIR *d=NULL;
    struct dirent *file;
    char path[pathSIZE];

    int count_treasures=0;

    if((d=opendir("treasureHunt"))==NULL){
        perror("eroare la deschidere");
        exit(1);
    }

    while((file=readdir(d))!=NULL){
        if(strcmp(file->d_name,".") ==0 || strcmp(file->d_name,"..")==0)
            continue;
        
        printf("%s\n",file->d_name);
        snprintf(path,sizeof(path),"%s/%s","treasureHunt",file->d_name);
        if(lstat(path,&st)==-1 || !S_ISREG(st.st_mode)){
            if(S_ISDIR(st.st_mode)){
                printf("Hunt name: %s --",file->d_name);
                parcurgere_director(path);
            }
        } else if(S_ISREG(st.st_mode)){
            TREASURE t;
            //snprintf(path_tr,sizeof(path_tr),"%s/treasure.dat",path);
            int tr=open(path,O_RDONLY,0777);
            while(read(tr,&t,sizeof(t))==sizeof(t))
                count_treasures++;
            close(tr);
            printf("%d treasure files\n)",count_treasures);
            }
        }
    closedir(d);
}

void handle_signal2(int signal){
    char message[100]={0};

    if(signal==SIGUSR1){
        strcpy(message,"Hunts status: lists hunts and total number of treasures");
        modify_commandFile(message);
    }

    parcurgere_director();
}

void list_hunts(){
    sa.sa_handler=handle_signal2;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;

    sigaction(SIGUSR1,&sa,NULL);

    printf("At PID %d start_monitor command was given\n",getpid());
    sleep(2);
    kill(getpid(),SIGUSR1);
    sleep(1);

    while(1)
        pause();
}


int main(){
    
    //start_monitor();
    list_hunts();
    return 0;
}