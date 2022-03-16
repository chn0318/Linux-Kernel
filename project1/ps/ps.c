#include<stdio.h>
#include<dirent.h>
#include<ctype.h>
#include<string.h>
#define MAX_SIZE 300
int translat(char c)
{
	if(c <= '9'&&c >= '0')
		return 1;
	else
		return -1;
}

// 判断字符串是否是数字
int isStringNum(char *str)
{
	int length = strlen(str);
	int i,n=0;
	int cnt=0;
	if(length == 0)
		return 0;
	
	for(i=0; i<length; i++) {
		cnt += translat(str[i]);
	}

	if(cnt == length)
		return 1;
	else
		return -1;
}

char getstatus(char* str){
    int i=0;
    int flag=0;
    while(i<strlen(str)){
        if(str[i]=='('){
            flag++;
        }
        if(str[i]==')'){
            flag--;
        }
        if(isupper(str[i])&&flag==0){
            return str[i]; 
        }
        i++;
    }
    return 'E';
}
int main(){
    printf("%5s  S CMD\n","PID");
    DIR* dp;
    struct dirent *dirp;
    if((dp=opendir("/proc"))==NULL){
        printf("Can not open /proc !");
        return 0;
    }
    while ((dirp=readdir(dp))!=NULL)
    {
        char path[30]="/proc/";
        if(isStringNum(dirp->d_name)==1){
            printf("%5s  ",dirp->d_name);
            FILE* fp;       
            strcat(path,dirp->d_name);
            char path_stat[30];
            char path_cmdline[30];
            char path_comm[30];
            memcpy(path_stat,path,strlen(path)+1);
            strcat(path_stat,"/stat");
            memcpy(path_cmdline,path,strlen(path)+1);
            strcat(path_cmdline,"/cmdline");
            memcpy(path_comm,path,strlen(path)+1);
            strcat(path_comm,"/comm");
            if((fp=fopen(path_stat,"r"))==NULL){
                printf("Can not open /proc/pid/stat");
                return 0;
            }
            char str[MAX_SIZE];
            fgets(str,MAX_SIZE,fp);
            printf("%c  ",getstatus(str));
            fp=fopen(path_cmdline,"r");
            memset(str,' ',MAX_SIZE);
            if(fgets(str,MAX_SIZE,fp)!=NULL){
                int index=0;
                while (1)
                {
                    if(str[index]=='\0'&&str[index+1]=='\0'){
                        break;
                    }
                    else{
                        if(str[index]=='\0'){
                            str[index]=' ';
                        }
                        index++;
                    }
                }
                printf("%s\n",str);
            }
            else{
                fp=fopen(path_comm,"r");
                printf("%c",'[');
                while(fgets(str,MAX_SIZE,fp)!=NULL){
                    str[strlen(str)-1]=']';
                    printf("%s\n",str);
                }
            }
        }

    }
    return 0;
    FILE* fp;
    char str[MAX_SIZE];
    memset(str,' ',MAX_SIZE);
    fp=fopen("/proc/485/cmdline","r");
    fgets(str,MAX_SIZE,fp);
    int index=0;
    while (1)
    {
        if(str[index]=='\0'&&str[index+1]=='\0'){
            break;
        }
        else{
            if(str[index]=='\0'){
                str[index]=' ';
            }
            index++;
        }
    }
    printf("%s\n",str);
    return 0;

}