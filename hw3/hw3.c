//includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <libgen.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "hw3.h"

//functions
void* readfile(void* arg);
void critical_section(FILE* f);

//global variables
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
bool buffer_full;
int pos;
char** paths;
int terminated;


//main program
int main(int argc, char* argv[])
{
    //check for 4 arguments
    if (argc != 4){
        fprintf(stderr, "ERROR: Invalid arguments\nUSAGE: ./a.out <input-directory> <buffer-size> <output-file>\n");
        return EXIT_FAILURE;
    }
    
    //determine maxwords and allocate memory for words
    maxwords = atoi(argv[2]);
    if (maxwords < 0){
        perror("ERROR: Second argument is negative");
        return EXIT_FAILURE;
    }
    words = calloc(maxwords, sizeof(char*)); 
    printf("MAIN: Dynamically allocated memory to store %d words\n", maxwords);
    fflush(stdout);
    
    //open directory
    struct dirent* d;
    DIR* dir;
    dir = opendir(argv[1]);
    if (dir == NULL){
        perror("ERROR: Could not open directory");
        return EXIT_FAILURE;
    }
    printf("MAIN: Opened \"%s\" directory\n", argv[1]);
    fflush(stdout);
    
    //other variables
    terminated = 0;         //number of terminated child threads
    pos = 0;                //index of words array
    buffer_full = false;    //buffer is not full
    int* child;             //child number
    int n = 0;              //number of children
    int i = 0;              //track file number
    int rc;
    struct stat file_info;
    
    //determine number of readable files
    while (( d = readdir(dir)) != NULL){
        char* p = (char*)malloc(100);
        strcpy(p, argv[1]);
        strcat(p, "/");
        strcat(p, d->d_name);
        lstat(p, &file_info);
        if (S_ISREG(file_info.st_mode)){ n++; }
        free(p);
    }
    closedir(dir);
    opendir(argv[1]);
    
    pthread_t tid[n]; //keep track of thread IDs
    paths = (char**)calloc(n, sizeof(char*));
    
    //read through the directory
    while ((d = readdir(dir)) != NULL){ //read every file
        
        //build path
        char* path = (char*)malloc(100);
        strcpy(path, argv[1]);
        strcat(path, "/");
        strcat(path, d->d_name);
        lstat(path, &file_info);
        
        if (S_ISREG(file_info.st_mode)){ //check for regular file
            
            //replace all punctuation with space
            FILE* f;
            int ch;
            f = fopen(path, "r+");
            while ((ch = fgetc(f)) != EOF){
                if (isalnum(ch) == 0){
                    fseek(f, -1, SEEK_CUR);
                    fputc(' ',f);
                    fseek(f, 0, SEEK_CUR);
                }
            }
            fclose(f);
            
            //assign file to thread
            paths[i] = (char*)malloc(100);
            strcpy(paths[i], path);
            child = malloc(sizeof(int));
            *child = i;
            rc = pthread_create(&tid[i], NULL, readfile, child);
            if (rc != 0){
              perror("MAIN: Could not create thread");
              return EXIT_FAILURE;
            }
            printf("MAIN: Created child thread for \"%s\"\n", d->d_name);
            fflush(stdout);
            
            i++;
        }
        free(path);
    }
    
    //close directory
    closedir(dir);
    printf("MAIN: Closed \"%s\" directory\n", argv[1]);
    fflush(stdout);
    
    //create output file
    int fd = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1){
        perror("ERROR: open() failed");
        return EXIT_FAILURE;
    }
    printf("MAIN: Created \"%s\" output file\n", argv[3]);
    fflush(stdout);
    
    //continuously check if buffer is full
    while (terminated != n){
        if (buffer_full){
            printf("MAIN: Buffer is full; writing %d words to output file\n", maxwords);
            fflush(stdout);
            while (pos > 0){ //clear buffer
                int i = maxwords - pos;
                write(fd, words[i], strlen(words[i]));
                write(fd, "\n", 1);
                pos--;
            }
            buffer_full = false;
        }
        usleep(10000);
    }
    
    //wait for child threads to terminate
    for (i = 0; i < n; i++){
        rc = pthread_join(tid[i], NULL);
        if (rc != 0){
            perror("ERROR: Could not join thread");
            return EXIT_FAILURE;
        }
        printf("MAIN: Joined child thread: %u\n", (unsigned int)tid[i]);
        fflush(stdout);
    }
    
    //finished with threads
    printf("MAIN: All threads are done; writing %d words to output file", pos);
    fflush(stdout);
    i = 0;
    while (i < pos){
        write(fd, words[i], strlen(words[i]));
        write(fd, "\n", 1);
        i++;
    }
    
    //free memory
    int f;
    for (f = 0; f < maxwords; f++){
        free(words[f]);
    }
    free(words);
    free(paths);
    
    return EXIT_SUCCESS;
}


//function executed by each thread
void* readfile(void* arg)
{
    usleep(100);
    //open file
    int child = *(int*)arg;
    free(arg);
    FILE* f;
    f = fopen(paths[child], "r");
    printf("TID %u: Opened \"%s\"\n", (unsigned int)pthread_self(), basename(paths[child]));
    fflush(stdout);
    
    //critical section
    pthread_mutex_lock(&mutex);
    critical_section(f);
    pthread_mutex_unlock(&mutex);
    
    //close file
    fclose(f);
    printf("TID %u: Closed \"%s\"; and exiting\n", (unsigned int)pthread_self(), basename(paths[child]));
    fflush(stdout);
    
    terminated++;
    return NULL;
}


//critical section called by threads
void critical_section(FILE* f)
{
    //read words
    char string[100];
    while (fscanf(f, "%s", string) != EOF){ //read each word
        if (strlen(string) > 1){ //larger than 2 char
            
            //copy word
            char* word = (char*)malloc(sizeof(char)*strlen(string)+1);
            int n;
            for (n = 0; n < strlen(string); n++){
                word[n] = string[n];
            }
            word[n] = '\0'; //null terminator
            
            //store word in buffer
            words[pos] = (char*)malloc(sizeof(char)*strlen(word)+1);
            strcpy(words[pos], word);
            printf("TID %u: Stored \"%s\" in shared buffer at index [%d]\n", (unsigned int)pthread_self(), word, pos);
            fflush(stdout);
            pos++; //increment position
            free(word);
            
            //buffer is full
            if (pos == maxwords){
                buffer_full = true;
                while (buffer_full){ //wait until buffer has been cleared
                    usleep(10000);
                }
            }
        }
    }
}