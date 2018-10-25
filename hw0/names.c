#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//swaps two elements in an array
void swap(char** names, int i, int j){
    char* temp = names[i];
    names[i] = names[j];
    names[j] = temp;
}

//inserts names by comparing adjacent elements until correct position found
void insert(char** names, int i, char* n){
    
    names[i] = n;
    while (i > 0){
        int compare = strcmp(names[i], names[i-1]);
        if (compare < 0){
            swap(names, i, i-1);
            i--;
        }
        else{
            break;
        }
    }
}

int main(){
    
    int i;
    char* names[10];
    for (i = 0; i < 10; i++){
        printf("Enter name %d: ", i+1);
        char* n = (char*)malloc(100);
        scanf("%s", n);
        insert(names, i, n); //determines where to put name in array
    }
    
    printf("Sorted:\n");
    for (i = 0; i < 10; i++){
        printf("%s\n", names[i]);
    }
    
    return EXIT_SUCCESS;
}