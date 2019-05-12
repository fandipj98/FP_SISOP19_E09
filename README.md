# Laporan Resmi Sistem Operasi Final Project E09

#### Fandi Pranata Jaya - 05111740000056
#### Fadhil Musaad Al Giffary - 05111740000116

### Soal:
Buatlah sebuah music player dengan bahasa C yang memiliki fitur play nama_lagu, pause, next, prev, list lagu. Selain music player juga terdapat FUSE untuk mengumpulkan semua jenis file yang berekstensi .mp3 kedalam FUSE yang tersebar pada direktori /home/user. Ketika FUSE dijalankan, direktori hasil FUSE hanya berisi file .mp3 tanpa ada direktori lain di dalamnya. Asal file tersebut bisa tersebar dari berbagai folder dan subfolder. program mp3 mengarah ke FUSE untuk memutar musik.
Note: playlist bisa banyak, link mp3player

### Jawaban:
#### FUSE
Pertama - tama kita perlu membuat fungsi `xmp_init()`yaitu fungsi yang dijalankan sebelum fuse di mount. Dalam fungsi `xmp_init()`, kita membuat folder `musictemp` sebagai folder temporary yang nanti nya program FUSE akan di mount ke folder temporary tersebut yang letaknya di `/home/user/musictemp`. Kemudian dalam fungsi `xmp_init()` kita perlu membuat thread dengan fungsi `joinMusic()` untuk mengumpulkan semua jenis file yang berekstensi .mp3 kedalam folder `musictemp` yang tersebar pada direktori `/home/user`. Syntaxnya adalah seperti berikut ini:
```
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

static struct fuse_operations xmp_oper = {
	.init		= xmp_init,
	.destroy	= xmp_destroy,
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
};
```
Kemudian dalam fungsi thread `joinMusic()`, kita memanggil fungsi `listdir()` dengan parameter path `/home/fandipj`. Pada fungsi `listdir()`, kita membaca semua file dan subdirectory dari path yang dipassing dengan fungsi c `opendir()` dan `readdir()`. Jika yang terbaca adalah subfolder, maka akan memanggil fungsi `listdir()` lagi secara rekursi dengan parameter path `path folder/subfolder`. Jika yang terbaca adalah file dan berekstensi .mp3, maka nama filenya akan disimpan kedalam array of string `musiclist`, namun sebelumnya perlu dicek terlebih dahulu apakah nama file .mp3 tersebut sama dengan salah satu file .mp3 yang sudah pernah terbaca dan tersimpan dalam array of string `musiclist`, jika nama filenya sudah pernah ada, maka file tersebut nanti nya sebelum dicopy ke folder `musictemp` yang ada di `/home/user/musictemp`, akan di rename terlebih dahulu dengan format `namafile_jumlahfilekembar.mp3`. Jika nama filenya belum pernah ada, maka file tersebut langsung dicopy ke folder `musictemp` yang ada di `/home/user/musictemp`. Cara copy file .mp3 tersebut menggunakan fungsi `system()` dengan perintah `cp 'path asal' 'path tujuan'`. Thread akan memanggil fungsi `listdir()` tersebut secara rekursi sampai semua file dan subfolder yang berada di `/home/user` sudah terbaca semua. Syntaxnya adalah seperti berikut ini:
```
char musiclist[1005][1005];
int counter;

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
                char command[1005], pathasal[1005], namafile[1005], pathtujuan[1005];
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

                //printf("COMMAND: %s\n",command);

                sprintf(command, "cp '%s' '%s'", pathasal, pathtujuan);

                system(command);
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
```
Kemudian sebelum program di unmount, maka program akan menjalankan fungsi `xmp_destroy()` yang berisi perintah untuk membaca semua file yang ada didalam folder `musictemp` dengan menggunakan fungsi c `opendir()` dan `readdir()`. Kemudian semua file yang berada didalam folder yang dimount tersebut didelete dengan menggunakan fungsi c `remove()`. Setelah semua file dalam folder tersebut dihapus, maka folder `musictemp` yang merupakan folder temporary dan folder yang dimount tersebut akan dihapus dengan fungsi c `remove()`. Syntaxnya adalah seperti berikut ini:
```
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
```
#### Mp3 Player
Pertama - tama buat thread untuk mp3 playernya yang mengarah ke mount point FUSE. berikut syntaxnya:
```
#include <ao/ao.h>
#include <mpg123.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
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

#define BITS 8

char musicdir[1005] = "/home/fandipj/FP/musicfuse/";
char song[1005];
int playlist[1005][1005];
char pname[1005][1005];
int totalplaylistsong[1005];
int totalplaylist = 0;
int currentsongnumber = -1;
int currentplaylistnumber = -1;
int currentplaylistsongnumber = -1;
int totalsong;
int playflag = 0;
int stopflag = 0;
int pauseflag = 0;
int refreshflag = 0;

pthread_t t;

void* player(void* arg)
{
    char tmppath[1005];
    mpg123_handle *mh;
    unsigned char *buffer;
    size_t buffer_size;
    size_t done;
    int err;

    int driver;
    ao_device *dev;

    ao_sample_format format;
    int channels, encoding;
    long rate;

    /* initializations */
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));


    /* decode and play */
    while (1) {
        /* open the file and get the decoding format */
        strcpy(tmppath, musicdir);
        DIR *dp;
        struct dirent *de;
        dp = opendir(musicdir);
        int cnt = 0;
        stopflag = 0;
        while (playflag == 0) {
            sleep(1);
        }
       
        while ((de = readdir(dp)) != NULL) {
            if (strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, ".") == 0)
                continue;
            if (cnt == currentsongnumber) {
                strcat(tmppath, de->d_name);
                break;
            }
            cnt++;
        }
        mpg123_open(mh, tmppath);
        mpg123_getformat(mh, &rate, &channels, &encoding);
        /* set the output format and open the output device */
        format.bits = mpg123_encsize(encoding) * BITS;
        format.rate = rate;
        format.channels = channels;
        format.byte_format = AO_FMT_NATIVE;
        format.matrix = 0;
        dev = ao_open_live(driver, &format, NULL);

        while (playflag == 1 && stopflag == 0) {
            while (pauseflag == 1) {
                sleep(1);
            }
            if (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
                ao_play(dev, buffer, done);
            else {
                if (currentplaylistnumber == -1)
                    currentsongnumber = (currentsongnumber + 1 + totalsong) % totalsong;
                else {
                    currentplaylistsongnumber = (currentplaylistsongnumber + 1) % totalplaylistsong[currentplaylistnumber];
                    currentsongnumber = playlist[currentplaylistnumber][currentplaylistsongnumber];
                }
                break;
            }
        }
    }

    /* clean up */
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();

    return NULL;
}

int main(void)
{
    int err = pthread_create(&(t),NULL,&player,NULL); //membuat thread
}
```
Kemudian hitung jumlah total lagu yang ada dalam folder FUSE. Kemudian didalam fungsi `int main()` terdapat beberapa fungsi yang akan dijalankan setelah mendapatkan inputan string yang sesuai yaitu:
1. Fungsi untuk stop music yang sedang di play. Syntaxnya adalah seperti berikut ini:
```
if (strcmp(inp, "stop") == 0) {
    system("clear");
    if (playflag == 1) {
	stopflag = 1;
	playflag = 0;
	currentsongnumber = -1;
	printf("Song Stopped.\n");
    } else {
	printf("No playing song.\n");
    }
}
```
2. Fungsi untuk menampilkan semua command list yang ada. Syntaxnya adalah seperti berikut ini:
```
else if (strcmp(inp, "help") == 0) {
    system("clear");
    printf("------------------------\n");
    printf("| Here's Your Command:\n");
    printf("------------------------\n");
    printf("| 1. | list (List all song)\n");
    printf("| 2. | play ([Song Name].mp3 | [Song Number]) (Play a song)\n");
    printf("| 3. | stop (Stop currently playing song)\n");
    printf("| 4. | next (Play next song)\n");
    printf("| 5. | prev (Play previous song)\n");
    printf("| 6. | pause (Pause current song)\n");
    printf("| 7. | resume (Resume current song)\n");
    printf("| 8. | listp (List all Playlist)\n");
    printf("| 9. | addp [Playlist Name] (Make New Playlist)\n");
    printf("| 10.| remp [Playlist Name] (Remove Playlist)\n");
    printf("| 11.| addsp [Playlist Name] press enter, then ([Song Name].mp3 | [Song Number]) (Add Song To Playlist)\n");
    printf("| 12.| remsp [Playlist Name] press enter, then ([Song Name].mp3 | [Song Number]) (Remove Song From Playlist)\n");
    printf("| 13.| movep [Playlist Name] (Move to Playlist)\n");
    printf("| 14.| back (Move to Global Playlist)\n");
    printf("| 15.| help (List all commands)\n");
    printf("| 16.| exit (Exit player)\n");
    printf("------------------------\n");
}
```
3. Fungsi untuk play music yang dipilih. Music akan berjalan terus sampai habis, kemudian akan secara otomatis program mp3 player akan play music selanjutnya. Syntaxnya adalah seperti berikut ini:
```
else if (strcmp(inp, "play") == 0) {
    scanf(" %[^\n]", song);
    system("clear");
    dp = opendir(musicdir);
    int zzz = -1;
    int cnt = 0;
    int sz = strlen(song);
    if (sz > 4 && song[sz - 1] == '3' && song[sz - 2] == 'p' && song[sz - 3] == 'm' && song[sz - 4] == '.') {
	while ((de = readdir(dp)) != NULL) {
	    if (strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, ".") == 0)
		continue;
	    if (strcmp(de->d_name, song) == 0) {
		zzz = cnt;
		break;
	    }
	    cnt++;
	}
    } else if (isDigit(song)) {
	int now = atoi(song);
	if (currentplaylistnumber == -1) {
	    if (now <= totalsong) {
		zzz = now - 1;
	    }
	} else {
	    if (now <= totalplaylistsong[currentplaylistnumber]) {
		zzz = playlist[currentplaylistnumber][now - 1];
	    }
	}
    }
    if (zzz != -1) {
	if (currentplaylistnumber == -1) {
	    stopflag = 1;
	    pauseflag = 0;
	    playflag = 1;

	    printf("Playing A Song.\n");
	    currentsongnumber = zzz;
	} else {
	    int found = 0;
	    for (int i = 0; i < totalplaylistsong[currentplaylistnumber]; i++) {
		if (playlist[currentplaylistnumber][i] == zzz) {
		    found = 1;
		    currentplaylistsongnumber = i;
		    break;
		}
	    }
	    if (found) {
		stopflag = 1;
		pauseflag = 0;
		playflag = 1;
		printf("Playing A Song.\n");
		currentsongnumber = zzz;
	    } else {
		printf("Song unavailable.\n");
	    }
	}
    } else {
	printf("Song unavailable.\n");
    }
}
```
4. Fungsi untuk menampilkan list nama lagu yang ada di global atau playlist. Syntaxnya adalah seperti berikut ini:
```
else if (strcmp(inp, "list") == 0) {
    system("clear");
    printf("-------------\n");
    printf("| SONG LIST |\n");
    printf("-------------\n");
    int cnt = 0;
    if (currentplaylistnumber == -1) {
	dp = opendir(musicdir);
	while ((de = readdir(dp)) != NULL) {
	    if (strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, ".") == 0)
		continue;
	    printf("| %d. | %s", cnt + 1, de->d_name);
	    if (cnt == currentsongnumber)
		printf(" (Now Playing)");
	    printf("\n");
	    cnt++;
	}
    } else {
	int localcnt = 0;
	for (int i = 0; i < totalplaylistsong[currentplaylistnumber]; i++) {
	    int cur = playlist[currentplaylistnumber][i];
	    cnt = 0;
	    dp = opendir(musicdir);
	    while ((de = readdir(dp)) != NULL) {
		if (strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, ".") == 0)
		    continue;
		if (cur == cnt) {
		    printf("| %d. | %s", localcnt + 1, de->d_name);
		    if (cnt == currentsongnumber)
			printf(" (Now Playing)");
		    printf("\n");
		    localcnt++;
		    break;
		}
		cnt++;
	    }
	}
    }
}
```
5. Fungsi untuk pause music yang sedang di play. Syntaxnya adalah seperti berikut ini:
```
else if (strcmp(inp, "pause") == 0) {
    system("clear");
    pauseflag = 1;
    if (playflag == 1) {
	printf("Song Paused.\n");
    } else {
	printf("No playing song.\n");
    }
}
```
6. Fungsi untuk memutar music sebelumnya (prev). Syntaxnya adalah seperti berikut ini:
```
else if (strcmp(inp, "prev") == 0) {
    system("clear");
    if (currentsongnumber != -1) {
	if (currentplaylistnumber == -1)
	    currentsongnumber = (currentsongnumber - 1 + totalsong) % totalsong;
	else {
	    currentplaylistsongnumber = (currentplaylistsongnumber - 1 + totalplaylistsong[currentplaylistnumber]) % totalplaylistsong[currentplaylistnumber];
	    currentsongnumber = playlist[currentplaylistnumber][currentplaylistsongnumber];
	}
	pauseflag = 0;
	stopflag = 1;
	playflag = 1;
	printf("Playing Previous Song.\n");
    } else {
	printf("No currently playing song.\n");
    }
}
```
7. Fungsi untuk memutar music setelahnya (next). Syntaxnya adalah seperti berikut ini:
```
else if (strcmp(inp, "next") == 0) {
    system("clear");
    if (currentsongnumber != -1) {
	if (currentplaylistnumber == -1)
	    currentsongnumber = (currentsongnumber + 1 + totalsong) % totalsong;
	else {
	    currentplaylistsongnumber = (currentplaylistsongnumber + 1) % totalplaylistsong[currentplaylistnumber];
	    currentsongnumber = playlist[currentplaylistnumber][currentplaylistsongnumber];
	}
	pauseflag = 0;
	stopflag = 1;
	playflag = 1;
	printf("Playing Next Song.\n");
    } else {
	printf("No currently playing song.\n");
    }
}
```
8. Fungsi untuk melanjutkan music yang di pause (resume). Syntaxnya adalah seperti berikut ini:
```
else if (strcmp(inp, "resume") == 0) {
    system("clear");
    pauseflag = 0;
    if (playflag == 1) {
	printf("Song Resumed.\n");
    } else {
	printf("No playing Song.\n");
    }
}
```
9. Fungsi untuk menampilkan semua playlist yang ada. Syntaxnya adalah seperti berikut ini:
```
else if (strcmp(inp, "listp") == 0) {
    system("clear");
    if (totalplaylist == 0) {
        printf("No Available Playlist.\n");
    } else {
        printf("------------\n");
        printf("| PLAYLIST |\n");
        printf("------------\n");
        for (int i = 0; i < totalplaylist; i++) {
            printf("| %d. | %s\n", i + 1, pname[i]);
        }
    }
}
```
10. Fungsi untuk menambahkan playlist. Syntaxnya adalah seperti berikut ini:
```
else if (strcmp(inp, "addp") == 0) {
    char tmpplaylist[1005];
    scanf(" %[^\n]", tmpplaylist);
    system("clear");
    int found = 0;
    for (int i = 0; i < totalplaylist; i++) {
	if (strcmp(tmpplaylist, pname[i]) == 0) {
	    found = 1;
	    break;
	}
    }
    if (found) {
	printf("Playlist already exist.\n");
    } else {
	strcpy(pname[totalplaylist], tmpplaylist);
	totalplaylist++;
	printf("Playlist Created.\n");
    }
}
```
11. Fungsi untuk menghapus playlist. Syntaxnya adalah seperti berikut ini:
```
else if (strcmp(inp, "remp") == 0) {
    char tmpplaylist[1005];
    scanf(" %[^\n]", tmpplaylist);
    system("clear");
    int found = -1;
    for (int i = 0; i < totalplaylist; i++) {
	if (strcmp(tmpplaylist, pname[i]) == 0) {
	    found = i;
	    break;
	}
    }
    if (found == -1) {
	printf("Playlist not found.\n");
    } else if (found == currentplaylistnumber) {
	printf("You cant remove current playlist.\n");
    } else {
	for (int i = found; i < totalplaylist - 1; i++) {
	    strcpy(pname[i], pname[i + 1]);
	    for (int j = 0; j < totalplaylistsong[i + 1]; i++) {
		playlist[i][j] = playlist[i + 1][j];
	    }
	    totalplaylistsong[i] = totalplaylistsong[i + 1];
	}
	totalplaylistsong[totalplaylist - 1] = 0;
	memset(playlist[totalplaylist - 1], 0, sizeof (playlist[totalplaylist - 1]));
	memset(pname[totalplaylist - 1], 0, sizeof (pname[totalplaylist - 1]));
	totalplaylist--;
	printf("Playlist Removed.\n");
    }
}
```
12. Fungsi untuk menambahkan lagu kedalam playlist tujuan. Syntaxnya adalah seperti berikut ini:
```
else if (strcmp(inp, "addsp") == 0) {
    char tmpplaylist[1005];
    scanf(" %[^\n]", tmpplaylist);
    system("clear");
    int plidx = -1;
    for (int i = 0; i < totalplaylist; i++) {
	if (strcmp(tmpplaylist, pname[i]) == 0) {
	    plidx = i;
	    break;
	}
    }
    if (plidx == -1) {
	printf("Playlist not found.\n");
    } else if (plidx == currentplaylistnumber) {
	printf("You cant add song to current playlist.\n");
    } else {
	char tmpsong[1005];
	dp = opendir(musicdir);
	int cnt = 0;
	while ((de = readdir(dp)) != NULL) {
	    if (strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, ".") == 0)
		continue;
	    printf("%d.%s", cnt + 1, de->d_name);
	    if (cnt == currentsongnumber)
		printf(" (Now Playing)");
	    printf("\n");
	    cnt++;
	}
	printf("\nInput Song Name or Song Number:\n");
	scanf(" %[^\n]", tmpsong);
	system("clear");
	dp = opendir(musicdir);
	int sgidx = -1;
	cnt = 0;
	int sz = strlen(tmpsong);
	if (sz > 4 && tmpsong[sz - 1] == '3' && tmpsong[sz - 2] == 'p' && tmpsong[sz - 3] == 'm' && tmpsong[sz - 4] == '.') {
	    while ((de = readdir(dp)) != NULL) {
		if (strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, ".") == 0)
		    continue;
		if (strcmp(de->d_name, tmpsong) == 0) {
		    sgidx = cnt;
		    break;
		}
		cnt++;
	    }
	} else if (isDigit(tmpsong)) {
	    int now = atoi(tmpsong);
	    if (now <= totalsong) {
		sgidx = now - 1;
	    }
	}
	if (sgidx != -1) {
	    int found = 0; 
	    for (int i = 0; i < totalplaylistsong[plidx]; i++) {
		if (sgidx == playlist[plidx][i]) {
		    found = 1;
		    break;
		}
	    }
	    if (found) {
		printf("Song Already added in the Playlist.\n");
	    } else {
		playlist[plidx][totalplaylistsong[plidx]] = sgidx;
		totalplaylistsong[plidx]++;
		printf("Song succesfully added to Playlist.\n");
	    }

	} else {
	    printf("Song unavailable.\n");
	}
    }
}
```
13. Fungsi untuk menghapus lagu dari playlist tujuan. Syntaxnya adalah seperti berikut ini:
```
else if (strcmp(inp, "remsp") == 0) {
    char tmpplaylist[1005];
    scanf(" %[^\n]", tmpplaylist);
    system("clear");
    int plidx = -1;
    for (int i = 0; i < totalplaylist; i++) {
	if (strcmp(tmpplaylist, pname[i]) == 0) {
	    plidx = i;
	    break;
	}
    }
    if (plidx == -1) {
	printf("Playlist not found.\n");
    } else if (plidx == currentplaylistnumber) {
	printf("You cant remove song from current playlist.\n");
    } else {
	char tmpsong[1005];
	int localcnt = 0;
	int cnt;
	for (int i = 0; i < totalplaylistsong[plidx]; i++) {
	    int cur = playlist[plidx][i];
	    cnt = 0;
	    dp = opendir(musicdir);
	    while ((de = readdir(dp)) != NULL) {
		if (strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, ".") == 0)
		    continue;
		if (cur == cnt) {
		    printf("| %d. | %s", localcnt + 1, de->d_name);
		    if (cnt == currentsongnumber)
			printf(" (Now Playing)");
		    printf("\n");
		    localcnt++;
		    break;
		}
		cnt++;
	    }
	}
	printf("\nInput Song Name or Song Number:\n");
	scanf(" %[^\n]", tmpsong);
	system("clear");
	dp = opendir(musicdir);
	int sgidx = -1;
	cnt = 0;
	int sz = strlen(tmpsong);
	if (sz > 4 && tmpsong[sz - 1] == '3' && tmpsong[sz - 2] == 'p' && tmpsong[sz - 3] == 'm' && tmpsong[sz - 4] == '.') {
	    int xxx = 0;
	    while ((de = readdir(dp)) != NULL) {
		if (strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, ".") == 0)
		    continue;
		if (strcmp(de->d_name, tmpsong) == 0) {
		    xxx = cnt;
		    break;
		}
		cnt++;
	    }
	    for (int i = 0; i < totalplaylistsong[plidx]; i++) {
		if (playlist[plidx][i] == xxx) {
		    sgidx = i;
		    break;
		}
	    }
	} else if (isDigit(tmpsong)) {
	    int now = atoi(tmpsong);
	    if (now <= totalplaylistsong[plidx]) {
		sgidx = now - 1;
	    }
	}
	if (sgidx != -1) {
	    for (int i = sgidx; i < totalplaylistsong[plidx] - 1; i++) {
		playlist[plidx][i] = playlist[plidx][i + 1];
	    }
	    totalplaylistsong[plidx]--;
	    printf("Song removed from the Playlist.\n");

	} else {
	    printf("Song unavailable.\n");
	}
    }
}
```
14. Fungsi untuk pindah ke playlist tujuan agar dapat menampilkan list lagu dan play lagu yang ada pada playlist yang bersangkutan. Sebelum pindah ke playlist tujuan, maka jika ada lagu yang sedang di play, maka akan distop secara otomatis. Syntaxnya adalah seperti berikut ini:
```
else if (strcmp(inp, "movep") == 0) {
    char tmpplaylist[1005];
    scanf(" %[^\n]", tmpplaylist);
    system("clear");
    if (strcmp(pname[currentplaylistnumber], tmpplaylist) == 0) {
	printf("You're already in the playlist.\n");
    } else {
	int idx = -1;
	for (int i = 0; i < totalplaylist; i++) {
	    if (strcmp(tmpplaylist, pname[i]) == 0) {
		idx = i;
		break;
	    }
	}
	if (idx == -1) {
	    printf("Playlist not Found.\n");
	} else {
	    playflag = 0;
	    currentsongnumber = -1;
	    currentplaylistnumber = idx;
	    printf("You're moved to the playlist.\n");
	}
    }
}
```
15. Fungsi untuk kembali ke menu sebelumnya (global). Sebelum kembali ke menu sebelumnya, maka lagu yang sedang diplay akan di stop. Syntaxnya adalah seperti berikut ini:
```
else if (strcmp(inp, "back") == 0) {
    system("clear");
    if (currentplaylistnumber == -1) {
	printf("You're already in the global playlist.\n");
    } else {
	playflag = 0;
	currentsongnumber = -1;
	currentplaylistnumber = -1;
	printf("You're moved to the global playlist.\n");
    }
}
```
16. Fungsi untuk exit program. Syntaxnya adalah seperti berikut ini:
```
else if (strcmp(inp, "exit") == 0) {
    loading("Exiting...");
    exit(0);
}
```
