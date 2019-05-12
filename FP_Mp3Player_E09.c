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

void loading(char *x) {
    srand(time(0));
    for (int i = 0; i < 15; i++) {
        system("clear");
        printf("%s\n", x);
        for (int j = 0; j < 15; j++)
            printf("-");
        printf("\n");
        for (int j = 0; j < i; j++)
            printf("|");
        printf("\n");
        for (int j = 0; j < 15; j++)
            printf("-");
        printf("\n");
        int cur = rand() % 2;
        sleep(cur);
    }
    system("clear");
}

int isDigit(char* x) {
    for (int i = 0; i < strlen(x); i++) {
        if (!('0' <= x[i] && x[i] <= '9'))
            return 0;
    }
    return 1;
}

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
    memset(totalplaylistsong, 0, sizeof(totalplaylistsong));
    int err = pthread_create(&(t),NULL,&player,NULL); //membuat thread
    DIR *dp;
    struct dirent *de;
    dp = opendir(musicdir);
    while ((de = readdir(dp)) != NULL) {
        if (strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, ".") == 0)
            continue;
        totalsong++;
    }
    loading("Initalize...");
    printf("--------------------------------------------\n");
    printf("| Welcome to Final Project Mp3 Player E09! |\n");
    printf("--------------------------------------------\n\n");
    
    while (1) {
        char cursong[1005] = "NONE";
        char curplay[1005] = "Global";
        int now = 0;
        dp = opendir(musicdir);
        while ((de = readdir(dp)) != NULL) {
            if (strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, ".") == 0)
                continue;
            if (now == currentsongnumber) {
                strcpy(cursong, de->d_name);
                break;
            }
            now++;
        }
        if (currentplaylistnumber != -1) {
            strcpy(curplay, pname[currentplaylistnumber]);
        }
        printf("------------------------\n");
        printf("| Current Playlist: %s\n", curplay);
        printf("------------------------\n");
        printf("| Currently Playing: %s\n", cursong);
        printf("------------------------\n");
        char inp[1005];
        printf("\nType \"help\" for command list.\n");
        printf("Type Your Command: ");
        scanf("%s", inp);
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
        } else if (strcmp(inp, "help") == 0) {
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
        } else if (strcmp(inp, "play") == 0) {
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
        } else if (strcmp(inp, "list") == 0) {
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
        } else if (strcmp(inp, "pause") == 0) {
            system("clear");
            pauseflag = 1;
            if (playflag == 1) {
                printf("Song Paused.\n");
            } else {
                printf("No playing song.\n");
            }
        } else if (strcmp(inp, "prev") == 0) {
            system("clear");
            if (currentplaylistnumber == -1)
                currentsongnumber = (currentsongnumber - 1 + totalsong) % totalsong;
            else {
                currentplaylistsongnumber = (currentplaylistsongnumber - 1 + totalplaylistsong[currentplaylistnumber]) % totalplaylistsong[currentplaylistnumber];
                currentsongnumber = playlist[currentplaylistnumber][currentplaylistsongnumber];
            }
            stopflag = 1;
            playflag = 1;
            printf("Playing Previous Song.\n");
        } else if (strcmp(inp, "next") == 0) {
            system("clear");
            if (currentplaylistnumber == -1)
                currentsongnumber = (currentsongnumber + 1 + totalsong) % totalsong;
            else {
                currentplaylistsongnumber = (currentplaylistsongnumber + 1) % totalplaylistsong[currentplaylistnumber];
                currentsongnumber = playlist[currentplaylistnumber][currentplaylistsongnumber];
            }
            stopflag = 1;
            playflag = 1;
            printf("Playing Next Song.\n");
        } else if (strcmp(inp, "resume") == 0) {
            system("clear");
            pauseflag = 0;
            if (playflag == 1) {
                printf("Song Resumed.\n");
            } else {
                printf("No playing Song.\n");
            }
        } else if (strcmp(inp, "listp") == 0) {
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
        } else if (strcmp(inp, "addp") == 0) {
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
        } else if (strcmp(inp, "remp") == 0) {
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
        } else if (strcmp(inp, "addsp") == 0) {
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
        } else if (strcmp(inp, "remsp") == 0) {
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
                    if (now <= totalsong) {
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
        } else if (strcmp(inp, "movep") == 0) {
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
        } else if (strcmp(inp, "back") == 0) {
            system("clear");
            if (currentplaylistnumber == -1) {
                printf("You're already in the global playlist.\n");
            } else {
                playflag = 0;
                currentsongnumber = -1;
                currentplaylistnumber = -1;
                printf("You're moved to the global playlist.\n");
            }
        } else if (strcmp(inp, "exit") == 0) {
            loading("Exiting...");
            exit(0);
        } else {
            system("clear");
            printf("Invalid Command.\n");
        }
    }
    exit(0);
    return 0;
}
