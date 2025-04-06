#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#define nameSIZE 100
#define clueTextSIZE 200
#define pathSIZE 70
#define SIZE 50

typedef struct TREASURE{
    int id;
    char userName[nameSIZE];
    double latitude,longitude;
    char clueText[clueTextSIZE];
    int value;
}TREASURE;

typedef struct SET{
    TREASURE v[SIZE];
    int dim;
}SET;

struct stat st;
int index_treasure=0;


char *getLastModificationTime(const char *filename){
    if(lstat(filename,&st)==-1){
        perror("unable to get file info");
        exit(-5);
    }

    time_t lastModTime=st.st_mtime;
    char *time=ctime(&lastModTime);
    return time;
}


void add_treasure(SET *s,const char *hunt,const char *treasure,const char *log){
    int tr,logg;
    char path1[pathSIZE],path2[pathSIZE];
    if(snprintf(path1,sizeof(path1),"%s/%s",hunt,treasure)==-1){
        perror("error with path 1");
        exit(-4);
    }
    if(snprintf(path2,sizeof(path2),"%s/%s",hunt,log)==-1){
        perror("error with path 2");
        exit(-4);
    }

    if(lstat(hunt,&st)==-1){
        perror("directory not created :<");
        if(mkdir(hunt,0777)==-1){
            perror("trouble with the directory!");
            exit(-2);
        }
    }
    if((tr=open(path1,O_RDWR|O_APPEND|O_CREAT,0777))==-1){
        perror("trouble opening treasure file!");
        exit(-3);
    }
    char output_text[]="Introduce treasure data: ";
    if(write(1,output_text,strlen(output_text))==-1){
        perror("error printing");
        exit(-3);
    }

    char buffer[4096]={0};
    if(read(0,buffer,4096)==-1){
        perror("trouble reading treasures");
        exit(-3);
    }
    char *p=NULL;
    int i=0;
    p=strtok(buffer," ");
    while(p!=NULL){
        switch(i){
            case 0:{
                s->v[index_treasure].id=strtol(p,NULL,10);
                break;
            }
            case 1:{
                strcpy(s->v[index_treasure].userName,p);
                break;
            }
            case 2:{
                s->v[index_treasure].latitude=strtol(p,NULL,10);
                break;
            }
            case 3:{
                s->v[index_treasure].longitude=strtol(p,NULL,10);
                break;
            }
            case 4:{
                strcpy(s->v[index_treasure].clueText,p);
                break;
            }
            case 5:{
                s->v[index_treasure].value=strtol(p,NULL,10);
            }
        }
        i++;
        p=strtok(NULL," ");
    }

    if(write(tr,buffer,strlen(buffer))==-1){
        perror("trouble incrementing the treasure file");
        exit(-3);
    }

    if(close(tr)==-1){
        perror("trouble closing the treasure file");
        exit(-3);
    }

    if((logg=open(path2,O_RDWR|O_APPEND|O_CREAT,0777))==-1){
        perror("trouble opening log file!");
        exit(-3);
    }

    strcpy(buffer,"added treasure ");
    char numStr[10];
    if(sprintf(numStr,"%d",index_treasure)==-1){
        perror("error with conversion");
    }

    strcat(buffer,numStr);
    strcat(buffer,"\n");
    if(write(logg,buffer,strlen(buffer))==-1){
        perror("trouble writing in the log file!");
        exit(-3);
    }
 

    if(close(logg)==-1){
        perror("trouble closing the log file");
        exit(-3);
    }

    index_treasure++;
}

void list_treasures(const char *hunt,const char *treasure){
    char buffer[4096]="Hunt name:";
    strcat(buffer,hunt);
    strcat(buffer,", File size:");

    char path1[pathSIZE];
    if(snprintf(path1,sizeof(path1),"%s/%s",hunt,treasure)==-1){
        perror("error with path 1");
        exit(-4);
    }
    int tr;
    if((tr=open(path1,O_RDONLY,0777))==-1){
        perror("trouble opening treasure file!");
        exit(-3);
    }

    char numStr[10];
    if(stat(path1,&st)==-1){
        perror("error with the file size");
        exit(-5);
    }
    sprintf(numStr,"%ld",st.st_size);
    strcat(buffer,numStr);
    strcat(buffer,", Last modification time:");
    char *time=getLastModificationTime(path1);
    strcat(buffer,time);


    strcat(buffer,"\nList of treasures:\n");
    write(1,buffer,strlen(buffer));

    strcpy(buffer,"");

    while(read(tr,buffer,4096)){
        strcat(buffer,"\n");
        write(1,buffer,strlen(buffer));
    }
    if(close(tr)==-1){
        perror("trouble closing the treasure file");
        exit(-3);
    }
}

void view_treasure(const char *hunt,const char *treasure,int id){
    
}

void remove_treasure(){

}

void remove_hunt(){

}

int main(int argc,char **argv){

    /*int file_treasures;
    if(lstat(argv[1],&st)==-1){
        perror("not created :<\n");
        if((file_treasures=creat(argv[1],0777))==-1){
            perror("file not created!");
            exit(-1);
        }
    } else{
        if((file_treasures=open(argv[1],O_RDONLY,777))==-1){
            perror("file not opened!");
            exit(-1);
        }
    }*/

    SET s;
    s.dim=0;

    /*add_treasure(argv[1],argv[2],argv[3]);
    add_treasure(argv[1],argv[2],argv[3]);
    add_treasure(argv[1],argv[2],argv[3]);
    add_treasure(argv[1],argv[2],argv[3]);*/
    list_treasures(argv[1],argv[2]);
    return 0;
}