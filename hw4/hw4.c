//includes
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#define BUFFER_SIZE 1024

//compare function for sorting strings
int string_compare(const void* a, const void* b){
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
}

//main program
int main(int argc, char* argv[]){
    
    //check for 2 arguments
    if (argc != 2){
        fprintf(stderr, "ERROR invalid number of arguments\n");
        return EXIT_FAILURE;
    }
    
    //create socket
    int sd = socket(PF_INET, SOCK_STREAM, 0); //create the listener socket as TCP socket
    if (sd == -1){
        fprintf(stderr, "ERROR socket() failed\n");
        return EXIT_FAILURE;
    }
    
    //socket structures
    struct sockaddr_in server;
    server.sin_family = PF_INET;
    server.sin_addr.s_addr = INADDR_ANY;  //allow any IP address to connect
    unsigned short port = atoi(argv[1]);
    server.sin_port = htons(port); //host to network short for data marshalling
    int len = sizeof(server);
    
    //bind socket
    if (bind(sd, (struct sockaddr*)&server, len) == -1){
        fprintf(stderr, "ERROR bind() failed\n");
        return EXIT_FAILURE;
    }
    
    //change to storage directory
    int rc = chdir("storage");
    if (rc == -1){
        fprintf(stderr, "ERROR could not open storage directory\n");
        return EXIT_FAILURE;
    }
    
    //identify this port as a listener port
    if (listen(sd, 5) == -1){
        fprintf(stderr, "ERROR listen() failed\n");
        return EXIT_FAILURE;
    }
    printf("Started server\nListening for TCP connections on port: %d\n", port);
    fflush(stdout);
    
    struct sockaddr_in client;
    int fromlen = sizeof(client);
    
    int n;
    pid_t pid;
    char buffer[BUFFER_SIZE];
    
    //server running loop
    while (1){
        
        //blocked on accept
        int newsd = accept(sd, (struct sockaddr*)&client, (socklen_t*)&fromlen);
        printf("Rcvd incoming TCP connection from: %s\n", inet_ntoa(client.sin_addr));
        fflush(stdout);
        
        //handle the accepted new client connection in a child process
        pid = fork();
        if (pid == -1){
            fprintf(stderr, "ERROR fork() failed\n");
        }
        
        //parent process
        if (pid > 0){
            close(newsd); //close new client socket because child will handle that connection
        }
        //child process
        else{
            do{
                //receive data from client
                n = recv(newsd, buffer, BUFFER_SIZE, 0);
                if (n == -1){
                    fprintf(stderr, "ERROR recv() failed\n");
                }
                else if (n == 0){ //client closes socket
                    printf("[child %d] Client disconnected\n", getpid());
                    fflush(stdout);
                }
                else{ //client sent data
                    
                    //TODOprint receive statement
                    char temp[1024];
                    int c = 0;
                    while (buffer[c] != '\n'){
                        temp[c] = buffer[c];
                        c++;
                    }
                    temp[c] = '\0';
                    printf("[child %d] Received %s\n", getpid(), temp);
                    fflush(stdout);
                    
                    int err = 0; //boolean that we got an error
                    
                    ////////PUT COMMAND////////
                    if (buffer[0] == 'P' && buffer[1] == 'U' && buffer[2] == 'T' && buffer[3] == ' '){
                        
                        char filename[32];
                        char bytes[32];
                        
                        //parse message
                        int i = 4;
                        int j = 0;
                        //determine filename
                        while (buffer[i] != ' ' && err == 0){
                            if (buffer[i] == '\n'){ //too few arguments
                                n = send(newsd, "ERROR INVALID REQUEST\n", 22, 0);
                                printf("[child %d] Sent ERROR INVALID REQUEST\n", getpid());
                                fflush(stdout);
                                err = 1;
                                break;
                            }
                            filename[j] = buffer[i];
                            i++;
                            j++;
                        }
                        filename[j] = '\0';
                        i++;
                        j = 0;
                        //determine bytes
                        while (buffer[i] != '\n' && err == 0){
                            if (buffer[i] == ' '){ //too many arguments
                                n = send(newsd, "ERROR INVALID REQUEST\n", 22, 0);
                                printf("[child %d] Sent ERROR INVALID REQUEST\n", getpid());
                                fflush(stdout);
                                err = 1;
                                break;
                            }
                            if (!isdigit(buffer[i]) && err == 0){ //invalid bytes
                                n = send(newsd, "ERROR INVALID REQUEST\n", 22, 0);
                                printf("[child %d] Sent ERROR INVALID REQUEST\n", getpid());
                                fflush(stdout);
                                err = 1;
                                break;
                            }
                            bytes[j] = buffer[i];
                            i++;
                            j++;
                        }
                        bytes[j] = '\0';
                        i++;
                        //check if bytes = 0
                        if (atoi(bytes) == 0 && err == 0){
                            n = send(newsd, "ERROR INVALID REQUEST\n", 22, 0);
                            printf("[child %d] Sent ERROR INVALID REQUEST\n", getpid());
                            fflush(stdout);
                            err = 1;
                        }
                        //check if file already exists in directory
                        if (access(filename, F_OK) != -1 && err == 0){
                            n = send(newsd, "ERROR FILE EXISTS\n", 18, 0);
                            printf("[child %d] Sent ERROR FILE EXISTS\n", getpid());
                            fflush(stdout);
                            err = 1;
                        }
                        
                        //store file in storage
                        if (err == 0){
                            int fd = open(filename, O_WRONLY | O_CREAT, 0660);
                            if (fd == -1){
                                fprintf(stderr, "ERROR open() failed\n");
                            }
                            
                            //TODOread the rest of the buffer
                            char temp[BUFFER_SIZE];
                            int t;
                            for (t = 0; i < n; t++, i++){
                                temp[t] = buffer[i];
                            }
                            write(fd, temp, t);
                            
                            //loop to read all bytes if larger than buffer
                            int bytesread = t;
                            while (bytesread < atoi(bytes)){
                                //read contents from client
                                n = recv(newsd, buffer, BUFFER_SIZE, 0);
                                if (n == -1){
                                    fprintf(stderr, "ERROR recv() failed");
                                }
                                //write contents to file
                                i = 0;
                                while (i < n){
                                    write(fd, &buffer[i], 1);
                                    i++;
                                    bytesread++;
                                    if (bytesread == atoi(bytes)){ //read enough bytes
                                        break;
                                    }
                                }
                            }
                            printf("[child %d] Stored file \"%s\" (%s bytes)\n", getpid(), filename, bytes);
                            fflush(stdout);
                        }
                        
                        //send ACK message to client
                        if (err == 0){
                            n = send(newsd, "ACK\n", 4, 0);
                            printf("[child %d] Sent ACK\n", getpid());
                            fflush(stdout);
                        }
                    }
                    
                    ////////GET COMMAND////////
                    else if (buffer[0] == 'G' && buffer[1] == 'E' && buffer[2] == 'T' && buffer[3] == ' '){
                        
                        char filename[32];
                        char byteoffset[32];
                        char length[32];
                        
                        //parse message
                        int i = 4;
                        int j = 0;
                        //determine filename
                        while (buffer[i] != ' ' && err == 0){
                            if (buffer[i] == '\n'){ //too few arguments
                                n = send(newsd, "ERROR INVALID REQUEST\n", 22, 0);
                                printf("[child %d] Sent ERROR INVALID REQUEST\n", getpid());
                                fflush(stdout);
                                err = 1;
                                break;
                            }
                            filename[j] = buffer[i];
                            i++;
                            j++;
                        }
                        filename[j] = '\0';
                        i++;
                        j = 0;
                        //determine byteoffset
                        while (buffer[i] != ' ' && err == 0){
                            if (buffer[i] == '\n'){ //too few arguments
                                n = send(newsd, "ERROR INVALID REQUEST\n", 22, 0);
                                printf("[child %d] Sent ERROR INVALID REQUEST\n", getpid());
                                fflush(stdout);
                                err = 1;
                                break;
                            }
                            if (!isdigit(buffer[i]) && err == 0){ //invalid byteoffset
                                n = send(newsd, "ERROR INVALID REQUEST\n", 22, 0);
                                printf("[child %d] Sent ERROR INVALID REQUEST\n", getpid());
                                fflush(stdout);
                                err = 1;
                                break;
                            }
                            byteoffset[j] = buffer[i];
                            i++;
                            j++;
                        }
                        byteoffset[j] = '\0';
                        i++;
                        j = 0;
                        //determine length
                        while (buffer[i] != '\n' && err == 0){
                            if (buffer[i] == ' '){ //too many arguments
                                n = send(newsd, "ERROR INVALID REQUEST\n", 22, 0);
                                printf("[child %d] Sent ERROR INVALID REQUEST\n", getpid());
                                fflush(stdout);
                                err = 1;
                                break;
                            }
                            if (!isdigit(buffer[i]) && err == 0){ //invalid length
                                n = send(newsd, "ERROR INVALID REQUEST\n", 22, 0);
                                printf("[child %d] Sent ERROR INVALID REQUEST\n", getpid());
                                fflush(stdout);
                                err = 1;
                                break;
                            }
                            length[j] = buffer[i];
                            i++;
                            j++;
                        }
                        length[j] = '\0';
                        //check if length = 0
                        if (atoi(length) == 0 && err == 0){
                            n = send(newsd, "ERROR INVALID REQUEST\n", 22, 0);
                            printf("[child %d] Sent ERROR INVALID REQUEST\n", getpid());
                            fflush(stdout);
                            err = 1;
                        }
                        //check if file does not exist in directory
                        if (access(filename, F_OK) == -1 && err == 0){
                            n = send(newsd, "ERROR NO SUCH FILE\n", 19, 0);
                            printf("[child %d] Sent ERROR NO SUCH FILE\n", getpid());
                            fflush(stdout);
                            err = 1;
                        }
                        //check for invalid byte range
                        struct stat s;
                        stat(filename, &s);
                        if (s.st_size < atoi(byteoffset) + atoi(length) && err == 0){
                            n = send(newsd, "ERROR INVALID BYTE RANGE\n", 25, 0);
                            printf("[child %d] Sent ERROR INVALID BYTE RANGE\n", getpid());
                            fflush(stdout);
                            err = 1;
                        }
                        
                        //send excerpt to client
                        if (err == 0){
                            //open file and read excerpt
                            FILE* f = fopen(filename, "r"); //open
                            unsigned char excerpt[atoi(length)]; //buffer
                            fseek(f, atoi(byteoffset), 1); //offset
                            fread(excerpt, sizeof(unsigned char), atoi(length), f); //read
                            
                            //send ACK message and excerpt to client
                            if (err == 0){
                                int size = strlen(length) + atoi(length) + 5;
                                char str[size];
                                sprintf(str, "ACK %s\n%s", length, excerpt);
                                n = send(newsd, str, size, 0);
                                printf("[child %d] Sent ACK %s\n", getpid(), length);
                                fflush(stdout);
                                printf("[child %d] Sent %d bytes of \"%s\" from offset %d\n", getpid(), atoi(length), filename, atoi(byteoffset));
                                fflush(stdout);
                            }
                        }
                    }
                    
                    ////////LIST COMMAND////////
                    else if (buffer[0] == 'L' && buffer[1] == 'I' && buffer[2] == 'S' && buffer[3] == 'T' && buffer[4] == '\n'){
                        
                        //count number of files in directory
                        int num = 0;
                        DIR* d;
                        struct dirent* dir;
                        d = opendir(".");
                        while ((dir = readdir(d)) != NULL){
                            if (dir->d_type == DT_REG){
                                num++;
                            }
                        }
                        closedir(d);
                        
                        //put filenames into array and sort
                        char* files[num];
                        int i = 0;
                        d = opendir(".");
                        while ((dir = readdir(d)) != NULL){
                            if (dir->d_type == DT_REG){
                                files[i] = dir->d_name;
                                i++;
                            }
                        }
                        closedir(d);
                        qsort(files, num, sizeof(char*), string_compare); //sort array
                        
                        //send list to client
                        char numstr[32];
                        sprintf(numstr, "%d", num);
                        n = send(newsd, numstr, strlen(numstr), 0);
                        printf("[child %d] Sent %d", getpid(), num);
                        fflush(stdout);
                        i = 0;
                        while (i < num){
                            n = send(newsd, " ", 1, 0);
                            n = send(newsd, files[i], strlen(files[i]), 0);
                            printf(" %s", files[i]);
                            fflush(stdout);
                            i++;
                        }
                        n = send(newsd, "\n", 1, 0);
                        printf("\n");
                        fflush(stdout);
                    }
                    
                    //invalid command
                    else{
                        n = send(newsd, "ERROR INVALID REQUEST\n", 22, 0);
                        printf("[child %d] Sent ERROR INVALID REQUEST\n", getpid());
                        fflush(stdout);
                    }
                }
            }
            while (n > 0);
            
            //terminate process
            close(newsd);
            exit(EXIT_SUCCESS);
        }
    }
    return EXIT_SUCCESS;
}