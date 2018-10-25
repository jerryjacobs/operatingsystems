#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

char* process_expr(char* expr){
    
    int i = 0;
    int parent_pid = getpid();
    int children[64]; //keeps track of pids for children
    int pipes[64][2]; //2D array of pipes
    int childcount = 0; //number of children
    int p_count = 0; //keeps track of parentheses
    char* ret = malloc(10); //return value
    
    printf("PID %d: My expression is \"%s\"\n", parent_pid, expr);
    fflush(stdout);
    
    //determine operator
    char operate;
    if (expr[0] == '('){
        operate = expr[1];
        
        //check for illegal operator
        if (operate != '+' && operate != '-' && operate != '*' && operate != '/'){
            char badop[128];
            
            //determine illegal operator
            int j = 0;
            while (expr[j+1] != ' ' && expr[j+1] != '(' && expr[j+1] != ')'){
                badop[j] = expr[j+1];
                j++;
            }
            badop[j] = '\0';
            
            printf("PID %d: ERROR: unknown \"%s\" operator; exiting\n", parent_pid, badop);
            fflush(stdout);
            exit (EXIT_FAILURE);
        }
    }
    else{
        return expr;
    }
    
    printf("PID %d: Starting \"%c\" operation\n", parent_pid, operate);
    fflush(stdout);
    
    //loop to identify all operands
    for (i = 0; i < strlen(expr); i++){
        
        if (expr[i] == '('){ p_count++; }
        if (expr[i] == ')'){ p_count--; }
        if (expr[i] == ' ' && p_count == 1){ //found child, call fork
            
            //determine subexpression
            char subexpr[128];
            if (expr[i+1] == '('){ //large subexpression
                int j = 0;
                int start = i+1;
                int p_count2 = 0;
                while (1){
                    subexpr[j] = expr[start+j];
                    if (expr[start+j] == '('){ p_count2++; }
                    if (expr[start+j] == ')'){ p_count2--; }
                    if (p_count2 == 0){ break; } //reached closing parenthesis
                    j++;
                }
                subexpr[j+1] = '\0';
            }
            else{ //single value subexpression
                int j = 0;
                int start = i+1;
                while (expr[start+j] != ' ' && expr[start+j] != '(' && expr[start+j] != ')'){
                    subexpr[j] = expr[start+j];
                    j++;
                }
                subexpr[j] = '\0';
            }
            
            //create the pipe
            int rc = pipe(pipes[childcount]);
            if (rc == -1){ //check if pipe failed
                fprintf(stderr, "ERROR: pipe failed\n");
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
            
            //fork into child processes
            pid_t pid = fork();
            if (pid == -1){ //check if fork failed
                fprintf(stderr, "ERROR: fork failed\n");
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
            
            //child process
            if (pid == 0){
                
                //check if dividing by zero
                char zero[2];
                zero[0] = '0';
                zero[1] = '\0';
                if (operate == '/' && childcount > 0 && strcmp(subexpr, zero) == 0){
                    exit(6); //special exit case (divide by zero)
                }
                
                int mypid = getpid();
                char* result = process_expr(subexpr);
                
                if (subexpr[0] != '('){ printf("PID %d: Sending \"%s\" on pipe to parent\n", mypid, subexpr); fflush(stdout); }
                else{ printf("PID %d: Processed \"%s\"; sending \"%s\" on pipe to parent\n", mypid, subexpr, result); fflush(stdout); }
                
                //write result to pipe
                close(pipes[childcount][0]); //close the read end of the pipe
                pipes[childcount][0] = -1;
                write(pipes[childcount][1], result, strlen(result));
                
                exit(EXIT_SUCCESS);
            }

            //store child pid
            children[childcount] = pid;
            childcount++;
        }
    }

    int results[childcount]; //used to store results of all children
    
    //loop through children from left to right
    int j;
    for (j = 0; j < childcount; j++){
        
        //check for not enough operands
        if (childcount < 2){
            printf("PID %d: ERROR: not enough operands; exiting\n", parent_pid);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        
        //wait for children
        int status;
        pid_t child_pid = waitpid(children[j], &status, 0);
        
        //check child termination
        if (WIFSIGNALED(status)){
            printf("PID %d: child terminated abnormally [child pid %d]\n", parent_pid, child_pid);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        else if (WIFEXITED(status)){
            int child_return_value = WEXITSTATUS(status);
            if (child_return_value == 6){ //special exit case (divide by zero)
                printf("PID %d: ERROR: division by zero is not allowed; exiting\n", parent_pid);
                fflush(stdout);
                exit(EXIT_FAILURE);
            }
            if (child_return_value != 0){
                printf("PID %d: child terminated with nonzero exit status %d [child pid %d]\n", parent_pid, child_return_value, child_pid);
                fflush(stdout);
                exit(EXIT_FAILURE);
            }
        }
        
        //read result from pipe
        close(pipes[j][1]); //close the write end of the pipe
        pipes[j][1] = -1;
        char buffer[128];
        int bytes_read = read(pipes[j][0], buffer, 128);
        buffer[bytes_read] = '\0';
        results[j] = atoi(buffer);
    }
    
    //calculate expression depending on operator by looping through child results
    if (operate == '+'){
        int sum = 0;
        for (j = 0; j < childcount; j++){
            sum += results[j];
        }
        sprintf(ret, "%d", sum);
        return ret;
    }
    if (operate == '-'){
        int difference = results[0];
        for (j = 1; j < childcount; j++){
            difference -= results[j];
        }
        sprintf(ret, "%d", difference);
        return ret;
    }
    if (operate == '*'){
        int product = 1;
        for (j = 0; j < childcount; j++){
            product *= results[j];
        }
        sprintf(ret, "%d", product);
        return ret;
    }
    if (operate == '/'){
        int quotient = results[0];
        for (j = 1; j < childcount; j++){
            if (results[j] == 0){ //check for divide by zero
                printf("PID %d: ERROR: division by zero is not allowed; exiting\n", parent_pid);
                fflush(stdout);
                exit(EXIT_FAILURE);
            }
            quotient /= results[j];
        }
        sprintf(ret, "%d", quotient);
        return ret;
    }
    
    exit(EXIT_FAILURE); //should never get here
}


int main(int argc, char* argv[]){
    
    //checking number of arguments
    if (argc != 2){
        fprintf(stderr, "ERROR: Invalid arguments\nUSAGE: ./a.out <input-file>\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }

    //reading expression from file
    FILE* file;
    file = fopen(argv[1], "r");
    char expression[128];
    fgets(expression, sizeof(expression), file);
    
    //remove newline at end of expression
    if (expression[strlen(expression)-1] == '\n'){
        expression[strlen(expression)-1] = '\0';
    }
    
    //evaluate expression
    char* value = process_expr(expression);
    printf("PID %d: Processed \"%s\"; final answer is \"%s\"\n", getpid(), expression, value);
    fflush(stdout);
    
    //end of program
    fclose(file);
    return EXIT_SUCCESS;
}