#include <errno.h> // for things like errno, EINTR
#include <fcntl.h> // stands for file control and is for this like open() and close()
#include <signal.h> // this is for signal handling
#include <stdio.h> // for things like printf, fprintf, fgets, clearerr
#include <stdlib.h> // for use of things like exit() and other system commands
#include <string.h> // for things like strcmp, strtok, strchr, strstr
#include <sys/types.h> // for pid_t with use of getpid(), only useful when coding on Windows
#include <unistd.h> // for things like getpid(), pid_t, chdir when on linux, although I had to include <sys/types.h> in the func.c file since the editor was not recognizing <unistd.h>
#include <sys/wait.h> // used for wait and waitpid
#include "func.h" // this is where all of the function prototypes go. for now i will keep them in the main file but before i turn them in i will remove them and this comment


// Globals
char cwd[1024];

int main(){

    //input buffer
    char cliBuffer[1024]; // this buffer along with the later use of memset from the string.h library will work with fgets to get the string that will be parsed and used 
                          //for parsing a string for arguments
    char *pointers[16]={}; // this is an array of pointers for use with strtok with the last element intended to be NULL
    pointers[15] = NULL; // this is for exec family of functions
    int pointC=0; // this is going to be the counter for each element in the pointer array after it has been parsed by strtok and put into the array of pointers



                                                                                    // GETTING COMMANDS AND COMMAND LINE ARGUMENTS FROM THE USER




    // ----------------------------------------------------------------------------------  ALL OF THIS IS TO JUST GET THE USER INPUT FIRST  -------------------------------------------------------------------------------------------
    fprintf(stdout, "Welcome to njr-bash\n");
    int running = 1; // psuedo boolean to control the continuous while loop although it's now seeming I don't need this
    while(running !=0){
        
        cwdHandler();
        signal(SIGINT, handleSigint);
        

        if(fgets(cliBuffer, sizeof(cliBuffer), stdin) == NULL){ // gets the potential commands and command line arguments
            if(errno == EINTR){ // fgets returns NULL if the SIGINT is called stopping fgets from returning anything which means I have to handle 
                clearerr(stdin); // the error so that I am allowed back into the while loop, otherwise all of the parsing logic breaks down 
                errno = 0; // and fgets constantly repeats trying to get input from the user
                continue;
            }
            break;
        }

        char *findNewline = strchr(cliBuffer, '\n'); // seeing if the user put it the right amount of characters
                                                     // if they did this will remove the newline character and replace it with the null terminator at the end of this if-else statement
        if(findNewline == NULL){  // findnewline will point to null if it did not find the newline character, which means we have
                                    // a buffer overflow and I will have to remove the left-overs so the next fgets doesn't get messed up
            int character; // a place to store all of the chewed up chars that I don't need that are left in the input buffer
            while (1){
                character = getchar();
                if(character == '\n'){ // will break the loop if we finally find the newline so as to not read or possibly write 
                                         //into the wrong space of memory
                    break;
                }
                if(character == EOF){ // same as above
                    break;
                }
            }
        } 
        else {
            *findNewline = '\0'; // If there is a newline in the cliBuffer, then it is just replaced
            
        }
        // Detect pipe at beginning. This is supposed to combat things like <space> | ls    since you cannot pipe "nothing" into ls
        char *tempBuffer = cliBuffer;
        while (*tempBuffer == ' ') 
        tempBuffer++; // skip leading spaces
        if(*tempBuffer == '|'){
            fprintf(stdout, "njr-bash: syntax error near unexpected token '|'\n");
            continue;
        }

        // This does a check for | from the end of the sentence to combat things like ls |   since you cannot pipe ls into "nothing"
        char *lastChar = cliBuffer + strlen(cliBuffer) - 1;
        while (lastChar > cliBuffer && *lastChar == ' ') lastChar--; // skip trailing spaces
        if(*lastChar == '|'){
            fprintf(stdout, "njr-bash: syntax error: pipe at end of line\n");
            continue;
        }
        // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        


        // ----------------------------------------------------------------- THIS IS TO FIRST CHECK IF THE USER IS TRYING TO REDIRECT INPUT OR PIPE INTO OTHER COMMANDS  --------------------------------------------------------------
        // -------------------------------------------------------------------------------- THEN MOVES ON TO THE REST OF THE COMMAND PARSING  -----------------------------------------------------------------------------------------
        
        // Splitting the line based on pipes | that are present
        char *pipeSegments[10];
        int numCmds = 0;
        char *savePtr;
        char *pToken = strtok_r(cliBuffer, "|", &savePtr); // I used strtok_r when I was having trouble with strtok. Here was what I found about a better option the _r. strtok_r (The Better Version): 
        // This version asks you to provide the "bookmark" (a pointer variable often called saveptr). Because you hold the bookmark, you can chop up as many different sentences as you want 
        // at the same time without them getting mixed up. 
        
        while (pToken != NULL && numCmds < 10){
            pipeSegments[numCmds++] = pToken;
            pToken = strtok_r(NULL, "|", &savePtr);
        }

        int inputFd = 0; // The "baton" passed between piped processes
        pid_t pids[10];  // Using this to keep track of all of the children that my shell creates from user input

        for(int i = 0; i < numCmds; i++){
            int pipeFds[2];
            if(i < numCmds - 1){ pipe(pipeFds); } // Only need pipe if there's a next command

            // This allows for grep hello < in.txt | wc -l > out.txt
            char *currOut = NULL, *currIn = NULL, *currErr = NULL;
            char *errP = strstr(pipeSegments[i], "2>");
            char *outP = strstr(pipeSegments[i], ">");
            char *inP = strstr(pipeSegments[i], "<");

            if(errP != NULL && outP == (errP + 1)) outP = NULL; // Determining if > is part of 2> or by itself

            if(outP){ 
                *outP = '\0'; 
                currOut = strtok(outP + 1, " "); 
                if(currOut == NULL){ 
                    fprintf(stderr, "njr-bash: syntax error near unexpected token `newline'\n"); 
                    break; 
                }
            } 
            if(inP){ //                                          These three checks are to break up the redirection commands and if there is something like ls >     it will catch the syntax error
                *inP = '\0'; 
                currIn = strtok(inP + 1, " ");
                if(currIn == NULL){ 
                    fprintf(stderr, "njr-bash: syntax error near unexpected token `newline'\n"); 
                    break; 
                } 
            }
            if(errP){ 
                *errP = '\0'; 
                currErr = strtok(errP + 2, " "); 
                if(currErr == NULL){ 
                    fprintf(stderr, "njr-bash: syntax error near unexpected token `newline'\n"); 
                    break; 
                }
            }

            // Parsing the cliBuffer after redirection and piping have been taken into account
            pointC = 0;
            char *pch = strtok(pipeSegments[i], " ");
            while (pch != NULL && pointC < 15){
                pointers[pointC++] = pch;
                pch = strtok(NULL, " ");
            }
            pointers[pointC] = NULL;
            if(pointers[0] == NULL){
                continue;
            } 

            // Checking for my own shell specific built in commands before passing everything over to execvp
            
            // To exit the shell
            if(strcmp(pointers[0], "exit") == 0){
                fprintf(stdout, "Thank you for using njr-bash!\n");
                exit(0);
            } 
            // To change working directories
            else if(strcmp(pointers[0], "cd") == 0){
                // If only "cd" was typed, go to the user's home directory
                if (pointC == 1) {
                    char *homeDir = getenv("HOME");
                    if (homeDir != NULL) {
                        chdir(homeDir);
                    }
                    continue;
                }
            
                // Rebuilding the path to support folders with spaces like <School Project> , kind of hacky but still works
                char fullPath[1024] = "";
                for(int j = 1; j < pointC; j++){
                    strcat(fullPath, pointers[j]);
                    if(j < pointC - 1){
                        strcat(fullPath, " ");
                    } 
                }
                if(chdir(fullPath) == -1) 
                    perror("cd failed");
                continue;
            }
            // To print help which explains shell specific built-ins as well as to know that you can use Linux commands as well
            else if(strcmp(pointers[0], "help") == 0){
                fprintf(stdout, "\nWelcome to njr-bash, the new shell on the block!\n\n    Built-in commands: cd, exit, help, pwd.\n\n");
                fprintf(stdout, "cd - Allows you to change directories and follows the same syntax as the linux built in minus the need for \" \" around directories with spaces.\n");
                fprintf(stdout, "exit - Stops and exits njr-bash.\n");
                fprintf(stdout, "help - Lists the njr-bash built-ins.\n");
                fprintf(stdout, "pwd - Prints the working directory that the user is currently in.\n");
                fprintf(stdout, "Otherwise use all of the built in linux commands, arguments, and syntax.\n\n");
                continue;
            }
            // To print the working directory
            else if(strcmp(pointers[0], "pwd") == 0){
                if(getcwd(cwd, sizeof(cwd))) 
                fprintf(stdout, "%s\n", cwd);
                continue;
            }

            // Using this for all of the linux shell commands
            pids[i] = fork();
            if(pids[i] == 0){
                // Restore default Ctrl+C behavior in the child process
                signal(SIGINT, SIG_DFL);

                // Pipe plumbing
                if(inputFd != 0)
                { 
                    dup2(inputFd, STDIN_FILENO); 
                    close(inputFd); 
                }
                if(i < numCmds - 1){ 
                    dup2(pipeFds[1], STDOUT_FILENO); 
                    close(pipeFds[0]); close(pipeFds[1]); 
                }

                // File Redirection 
                if(currIn){ // While I hate this kind of shorthand, if I don't use it I mess with the internal bookmark of strtok. This right here, while im too lazy to change it, things 
                    if(strlen(currIn) == 0){ // like this are the reason why I looked for a better alternative and found strtok_r
                        fprintf(stderr, "njr-bash: syntax error: missing file for <\n"); 
                        exit(1); 
                    }
                    int fd = open(currIn, O_RDONLY);
                    if(fd < 0)
                    { 
                        perror("in open"); 
                        exit(1); 
                    }
                    dup2(fd, STDIN_FILENO); close(fd);
                }
                if(currOut){ // While I hate this kind of shorthand, if I don't use it I mess with the internal bookmark of strtok. this right here, while im too lazy to change it, 
                             // things like this are the reason why I looked for a better alternative and found strtok_r
                    if(strlen(currOut) == 0){ 
                        fprintf(stderr, "njr-bash: syntax error: missing file for >\n"); 
                        exit(1); 
                    }
                    int fd = open(currOut, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if(fd < 0)
                    { perror("out open"); 
                        exit(1); 
                    }
                    dup2(fd, STDOUT_FILENO); close(fd);
                }
                if(currErr){ // While I hate this kind of shorthand, if I don't use it I mess with the internal bookmark of strtok. this right here, while im too lazy to change it, 
                             // things like this are the reason why I looked for a better alternative and found strtok_r . Yes, I'm repeating this because this caused me way too much pain.
                    int fd = open(currErr, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if(fd < 0)
                    { 
                        perror("err open"); 
                        exit(1); 
                    }
                    dup2(fd, STDERR_FILENO); close(fd);
                }

                execvp(pointers[0], pointers);
                // If we reach here, execvp failed.
                // Use fprintf(stderr...) to match how Linux shells report errors.
                if (errno == ENOENT) {
                    fprintf(stderr, "njr-bash: %s: command not found\n", pointers[0]);
                } else {
                    fprintf(stderr, "njr-bash: %s: %s\n", pointers[0], strerror(errno));
                }

                exit(127); // 127 is the standard Linux exit code for "command not found" so why not use it here
            } else {
                // Parent side of the pipeline
                if(inputFd != 0)
                {
                    close(inputFd); // Close old baton
                } 
                if(i < numCmds - 1){
                    close(pipeFds[1]); // Close write end
                    inputFd = pipeFds[0]; // Next command reads from this pipe
                }
            }
        }
        // Using a for loop to make sure that I have reaped all of the children
        for(int i = 0; i < numCmds; i++){
            if(pids[i] > 0){
                waitpid(pids[i], NULL, 0);
            }
            
        }
        // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------



        // -----------------------------------------------------------------------------------------   BUFFERS, POINTERS, AND ARG COUNT RESET  ----------------------------------------------------------------------------------------
            memset(cliBuffer,0,sizeof(cliBuffer)); // clearing the input buffer 
            memset(cwd, 0, sizeof(cwd));
            for(int i = 0;i<pointC;i++){ // clearing the pointers array using a for loop to iterate through the whole array
                pointers[i]=NULL;
            }
            pointC = 0; // putting the argument counter for pointers array back to 0
        }
        // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    
    //end of main
    return 0;
}
