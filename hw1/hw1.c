#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

int main(int argc, char* argv[]){
    
    //CHECKING ARGUMENTS
    
    if (argc == 1){
        perror("ERROR: Directory path not given");
        return EXIT_FAILURE;
    }
    if (argc > 3){
        perror("ERROR: More than 3 arguments given");
        return EXIT_FAILURE;
    }
    
    int numwords;
    if (argc == 3){
        numwords = atoi(argv[2]);
    }
    if (argc == 3 && numwords < 0){
        perror("ERROR: Second argument is negative");
        return EXIT_FAILURE;
    }
    
    struct dirent* d;
    DIR* dir;
    dir = opendir(argv[1]);
    if (dir == NULL){
        perror("ERROR: Could not open directory");
        return EXIT_FAILURE;
    }
    
    //READ DIRECTORY
    
    char** words = (char**)calloc(16, sizeof(char*));
    int* occurs = (int*)calloc(16, sizeof(int));
    printf("Allocated initial parallel arrays of size 16.\n");
    int length = 16;
    int index = 0;
    int count = 0;
    struct stat file_info;
    
    while ((d = readdir(dir)) != NULL){ //read every file
    
        //build path
        char* path = (char*)malloc(100);
        strcat(path, argv[1]);
        strcat(path, "/");
        strcat(path, d->d_name);
        lstat(path, &file_info);
        
        if (S_ISREG(file_info.st_mode)){ //check for regular file
            FILE* f;
            
            //replace all punctuation with space
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
            
            //READ FILE
            
            f = fopen(path, "r");
            
            char string[80];
            while (fscanf(f, "%s", string) != EOF){ //read each word
                if (strlen(string) > 1){ //larger than 2 char

                    char* word = (char*)malloc(sizeof(char)*strlen(string)+1);
                    int n;
                    for (n = 0; n < strlen(string); n++){
                        word[n] = string[n];
                    }
                    word[n] = '\0'; //null terminator

                    bool match = false;
                    int i;
                    for (i = 0; i < index; i++){ //search for matching words
                        if (strcmp(words[i], word) == 0){
                            match = true;
                            break;
                        }
                    }
                    
                    if (match){ //increase count
                        occurs[i]++;
                    }
                    else{ //add new word
                        words[index] = (char*)malloc(strlen(word)+1);
                        strcpy(words[index], word);
                        occurs[index] = 1;
                        index++;
                        
                        if (index+1 == length){ //increase array size
                            words = (char**)realloc(words, (length+16) * sizeof(char*));
                            occurs = (int*)realloc(occurs, (length+16) * sizeof(int));
                            length += 16;
                            printf("Re-allocated parallel arrays to be size %d.\n", length);
                        }
                    }
                    count++;
                    free(word);
                }
            }
            fclose(f);
        }
        free(path);
    }
    
    //OUTPUT
    
    printf("All done (successfully read %d words; %d unique words).\n", count, index);
    if(argc == 3 && numwords < index){ //some words
        printf("First %d words (and corresponding counts) are:\n", numwords);
        int k;
        for (k = 0; k < numwords; k++){
            printf("%s -- %d\n", words[k], occurs[k]);
        }
    }
    else{ //all words
        printf("All words (and corresponding counts) are:\n");
        int k;
        for (k = 0; k < index; k++){
            printf("%s -- %d\n", words[k], occurs[k]);
        }
    }
    
    //FREE MEMORY
    
    int f;
    for (f = 0; f < index; f++){
        free(words[f]);
    }
    free(words);
    free(occurs);
    
    closedir(dir);
    return EXIT_SUCCESS;
}