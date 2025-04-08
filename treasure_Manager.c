#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>

#define nameSIZE 100
#define clueTextSIZE 200
#define pathSIZE 512

typedef struct TREASURE{
    int id;
    char userName[nameSIZE];
    double latitude,longitude;
    char clueText[clueTextSIZE];
    int value;
}TREASURE;

struct stat st;

void init(const char *hunt){
    char path1[pathSIZE],path2[pathSIZE];
    snprintf(path1,sizeof(path1),"%s/treasure.dat",hunt);
    snprintf(path2,sizeof(path2),"%s/log.dat",hunt);
    int tr=creat(path1,0777);
    int log=creat(path2,0777);
}


void trim_newline(char *str) {
    char *p = strchr(str, '\n');
    if (p) *p = '\0';
}

char *getLastModificationTime(const char *filename){
    if(lstat(filename,&st)==-1){
        perror("unable to get file info");
        exit(-5);
    }

    time_t lastModTime=st.st_mtime;
    char *time=ctime(&lastModTime);
    return time;
}



void modify_log(const char *hunt,char *message){
    char path1[pathSIZE];
    char path2[pathSIZE];
    snprintf(path1,sizeof(path1),"%s/log.dat",hunt);
    snprintf(path2,sizeof(path2),"%s/treasure.dat",hunt);
    int log=open(path1,O_RDWR|O_APPEND,0777);
    if(log==-1){
        perror("trouble opening the log file");
        exit(1);
    }
    char *time=getLastModificationTime(path2);
    trim_newline(time);

    char buffer[256]={0};
    snprintf(buffer,sizeof(buffer),"%s  --%s\n",time,message);
    write(log,buffer,sizeof(buffer));

    close(log);
}

TREASURE readTreasure(){
    TREASURE t;

    char buffer_out[4096]={0};
    char buffer_in[4096]={0};

    strcpy(buffer_out,"Type in id:");
    write(1,buffer_out,strlen(buffer_out));
    read(0,buffer_in,5);
    t.id=strtol(buffer_in,NULL,10);
    
    strcpy(buffer_out,"Type in userName:");
    write(1,buffer_out,strlen(buffer_out));
    read(0,buffer_in,nameSIZE);
    trim_newline(buffer_in);
    strcpy(t.userName,buffer_in);

    strcpy(buffer_out,"Type in latitude:");
    write(1,buffer_out,strlen(buffer_out));
    read(0,buffer_in,10);
    t.latitude=strtod(buffer_in,NULL);

    strcpy(buffer_out,"Type in longitude:");
    write(1,buffer_out,strlen(buffer_out));
    read(0,buffer_in,10);
    t.longitude=strtod(buffer_in,NULL);

    strcpy(buffer_out,"Type in clue Text:");
    write(1,buffer_out,strlen(buffer_out));
    read(0,buffer_in,clueTextSIZE);
    trim_newline(buffer_in);
    strcpy(t.clueText,buffer_in);
   

    strcpy(buffer_out,"Type in value:");
    write(1,buffer_out,strlen(buffer_out));
    read(0,buffer_in,10);
    t.value=strtol(buffer_in,NULL,10);

    write(1,"\n",1);
    return t;
}


void add_treasure(const char *hunt){
    if(lstat(hunt,&st)==-1){
        perror("directory not created :<");
        if(mkdir(hunt,0777)==-1){
            perror("trouble with the directory!");
            exit(-2);
        }
        init(hunt);
    }
    int tr=-1;

    
    DIR *dir;
    struct dirent *in;
    char *filename=NULL;
    char path1[pathSIZE];
    
    if((dir=opendir(hunt))==NULL){
        perror("trouble accessing the directory!");
        exit(1);
    }
    while((in=readdir(dir))!=NULL){
        filename=in->d_name;

        snprintf(path1,sizeof(path1),"%s/%s",hunt,filename);
        if(lstat(path1,&st)==-1){
            perror("trouble with the path to files in dir");
            exit(1);
        }

        if(strcmp(filename,"treasure.dat")==0){//e fisierul de treasure
            snprintf(path1,sizeof(path1),"%s/treasure.dat",hunt);
            if((tr=open(path1,O_RDWR|O_APPEND,0777))==-1){
                perror("trouble opening the treasure file");
                exit(1);
            }
            TREASURE t=readTreasure();
            
            if(write(tr,&t,sizeof(t))==-1){
                perror("trouble writing in the treasure file");
                exit(1);
            }
            close(tr);
        }
        else
            if(strcmp(filename,"log.dat")==0){  //e fisierul de log
                char msg[]="added treasure";
                strcat(msg,"\n");
                modify_log(hunt,msg);
            }
    }
    closedir(dir);
}

void list_treasures(const char *hunt){
    char buffer[4096]={0};
    char path1[pathSIZE];
    snprintf(path1,sizeof(path1),"%s/treasure.dat",hunt);
    if(lstat(path1,&st)==-1){
        perror("error building the path to the treasure file");
        exit(1);
    }

    char *time=getLastModificationTime(path1);
    int len=snprintf(buffer,sizeof(buffer),"Hunt name: %s\nTotal file size: %ld\nLast modification time: %s",hunt,st.st_size,time);
    write(1,buffer,len);

    //reading the treasures
    int tr=open(path1,O_RDONLY,0777);
    if(tr==-1){
        perror("trouble opening the treasure file");
        exit(1);
    }

    TREASURE t;
    strcpy(buffer,"");
    while((read(tr,&t,sizeof(t)))==sizeof(t)){
        int len=snprintf(buffer,sizeof(buffer),"List of treasures:\nId: %d\nUser name: %s\nLatitude: %.2lf\nLongitude: %.2lf\nClue text: %s\nValue: %d\n\n",t.id, t.userName, t.latitude, t.longitude, t.clueText, t.value);
        write(1,buffer,len);
        strcpy(buffer,"");
    }
    write(1,"\n",1);
    close(tr);
}


void view_treasure(const char *hunt,int treasure_id){
    char path1[pathSIZE];
    snprintf(path1,sizeof(path1),"%s/treasure.dat",hunt);
    int tr=open(path1,O_RDONLY,0777);
    if(tr==-1){
        perror("error opening the treasure file");
        exit(1);
    }
    
    TREASURE t;
    char buffer[4096]={0};
    int find=0;
    while(read(tr,&t,sizeof(t))==sizeof(t)){
        if(t.id==treasure_id){
            int len=snprintf(buffer,sizeof(buffer),"The treasure with the id %d:\nUser name: %s\nLatitude: %.2lf\nLongitude: %.2lf\nClue text: %s\nValue: %d\n",t.id, t.userName, t.latitude, t.longitude, t.clueText, t.value);
            write(1,buffer,len);
            find=1;
            break;
        }
    }
    if(find==0){
        strcpy(buffer,"INDEX NOT FOUND");
        write(1,buffer,strlen(buffer));
    }
    write(1,"\n",1);
    close(tr);
}

void remove_hunt(const char *hunt){
    char path1[pathSIZE],path2[pathSIZE];

    snprintf(path1,sizeof(path1),"%s/treasure.dat",hunt);
    snprintf(path2,sizeof(path2),"%s/log.dat",hunt);

    if(remove(path1)==-1){
        perror("trouble with deleting the treasure file");
        exit(-1);
    }
    if(remove(path2)==-1){
        perror("trouble with deleting the log file");
        exit(-1);
    }

    if(rmdir(hunt)==-1){
        perror("trouble with deleting the directory");
        exit(-1);
    }

    char msg[50]={0};
    strcpy(msg,"\nDIRECTORY DELETED SUCCESSFULLY\n");
    write(1,msg,strlen(msg));
}

int main(int argc,char **argv){
    //add_treasure(argv[1]);
    //add_treasure(argv[1]);
    //add_treasure(argv[1]);
    list_treasures(argv[1]);
    view_treasure(argv[1],atoi(argv[2]));
    remove_hunt(argv[1]);
}