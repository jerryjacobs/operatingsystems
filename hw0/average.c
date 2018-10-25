#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
    
    int i;
    int ints[10];
    for (i = 0; i < 10; i++){
        printf("Enter integer %d: ", i+1);
        int n;
        scanf("%d", &n);
        ints[i] = n;
    }
    
    int sum = 0;
    for (i = 0; i < 10; i++){
        sum += ints[i];
    }
    
    printf("Average: %f\n", sum*1.0/10);
    
    return EXIT_SUCCESS;
}