#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(){
    
    printf("Enter string: ");
    char* string = (char*)malloc(1000);
    scanf("%s", string);
    
    char* answer = NULL;
    int l = strlen(string); //length of substring
    int n = 1; //number of substrings with length l
    
    while (l > 0){ //decreasing length of substring per iteration
        l--;
        n++;
        
        char* substrings[n];
        
        int i; //starting position of substring
        for (i = 0; i < n; i++){ //store all substrings with length l
            char* substring = (char*)malloc(100);
            strncpy(substring, string+i, l);
            substrings[i] = substring;
        }
        
        //check for matching substrings
        for (i = 0; i < n; i++){
            int j;
            for (j = 0; j < n; j++){
                int compare = strcmp(substrings[i], substrings[j]);
                if (i != j && compare == 0){ //different substrings but equal
                    answer = substrings[i];
                    break;
                }
            }
        }
        
        if (answer != NULL){ break; }
    }
    
    if (answer == NULL){ //did not find repeating substring
        printf("No repeating substring");
    }
    else{ //longest repeating substring
        printf("Substring: %s\n", answer);
    }
    
    return EXIT_SUCCESS;
}