#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

static const char *dirpath = "/home/fandipj/musictemp";
pthread_t tid;
char musiclist[1005][1005];
int counter;

static int xmp_getattr(const char *path, struct stat *stbuf)
{
  int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
	res = lstat(fpath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
  	char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		int sz = strlen(de->d_name);
		printf("%s\n",de->d_name);
		if(sz > 4 && de->d_name[sz - 1] == '3' && de->d_name[sz - 2] == 'p' && de->d_name[sz - 3] == 'm' && de->d_name[sz - 4] == '.'){
				
		}
		res = (filler(buf, de->d_name, &st, 0));
			if(res!=0) break;
	}

	closedir(dp);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;
  	int fd = 0 ;

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

void listdir(const char *name)
{
    if(strcmp(name, dirpath) == 0)
        return;

    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            //printf("%s [%s]\n", path, entry->d_name);
            listdir(path);
        } else {
            int sz = strlen(entry->d_name);
            if(sz > 4 && entry->d_name[sz - 1] == '3' && entry->d_name[sz - 2] == 'p' && entry->d_name[sz - 3] == 'm' && entry->d_name[sz - 4] == '.'){
                printf("%s %s\n", name, entry->d_name);
                char pathasal[1005], namafile[1005], pathtujuan[1005];
                int jumlah = 0;
                if(counter>0){
                    for(int i=0; i<counter; i++){
                        if(strcmp(musiclist[i], entry->d_name)==0){
                            jumlah++;
                        }
                    }
                }            
                
                strcpy(musiclist[counter], entry->d_name);
                //printf("LIST: %s\n",musiclist[counter]);
                counter++;
                
                if(jumlah>0){
                    char nama[1005],temp[1005];
                    strcpy(temp, entry->d_name);
                    char* pos = strtok(temp, ".");  
                    strcpy(nama, pos);
                    
                    sprintf(namafile, "%s_%d.mp3", nama, jumlah);
                }
                else{
                    strcpy(namafile,entry->d_name);
                }

                strcpy(pathasal,name);
                strcat(pathasal,"/");
                strcat(pathasal,entry->d_name);

                strcpy(pathtujuan, dirpath);
                strcat(pathtujuan,"/");
                strcat(pathtujuan,namafile);

                pid_t child_id;
				child_id = fork();
				if (child_id == 0) 
				{
					// this is child
					char *argv[4] = {"cp", pathasal, pathtujuan, NULL};
			    	execv("/bin/cp", argv);
				}
            }
        }
    }
    closedir(dir);
}

void* joinMusic(){
	char dirhome[1005] = "/home/fandipj";
	memset(musiclist,0,sizeof(musiclist));
	counter = 0;
	listdir(dirhome);
 	return NULL;
}


static void* xmp_init(struct fuse_conn_info *conn)
{
	int status;
	struct stat sb;
	if(stat(dirpath, &sb)==0 && S_ISDIR(sb.st_mode)){
      
    }
    else{
        mkdir(dirpath, 0750);
    }

	pthread_create(&tid,NULL,&joinMusic,NULL);
	
	while((wait(&status))>0);

	return NULL;
}

void xmp_destroy(void* privateData)
{
	char filePath[1005];

 	DIR *dirMusic;
	struct dirent *de;
	dirMusic = opendir(dirpath);

 	while((de = readdir(dirMusic)) != NULL){
		if (de->d_type == DT_REG) {
			sprintf(filePath, "%s/%s", dirpath, de->d_name);
			remove(filePath);
		}
	}
	remove(dirpath);
}

static struct fuse_operations xmp_oper = {
	.init		= xmp_init,
	.destroy	= xmp_destroy,
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
