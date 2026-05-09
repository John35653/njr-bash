#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> // pid_t was not resolving so had to use this to get the compiler to quit complaining

extern char cwd[1024]; // had to do this so the compiler knows that there is an array of this type and doesn't throw a fit when make is ran

// Now this is cool. So when a child process is created it's signal handlers are all reset when the child is given execvp. 
// So here I could catch the signal and do nothing with it and when the child is created and handed execvp, all of it's 
// signal handlers are reset. This means I should be able to run something like htop in my shell, type Ctrl + C and not 
// crash my own shell when I send the child SIGINT. (Does work and is actually really cool how Linux handles this).
void handleSigint(int sig){
    if(getcwd(cwd,sizeof(cwd))!=NULL){  
        write(STDOUT_FILENO,"\n",1);    
        
    }
    else{
        write(STDOUT_FILENO,"\n",1);
        write(STDOUT_FILENO,":njr-bash> ",11); // if cwd fails, at the very least the shell wont crash and the user can still enter input
    } 
}


void cwdHandler(){
    if(getcwd(cwd,sizeof(cwd))!=NULL){   
        fprintf(stdout, "%s:njr-bash> ",cwd);
    }
    else{
        fprintf(stdout, "njr-bash> "); // doing this just in case getcwd fails, don't want the terminal to be filled up with logic the user may not understand
    }
    fflush(stdout);
}


void pid_check(pid_t pid){                   
    if(pid<0){                              
        perror("Cound not create child!");  
        exit(EXIT_FAILURE);
    }
}