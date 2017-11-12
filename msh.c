/*
    Maverick Shell
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#define BFR 512
#define MAX_ARGS 10
#define EXIT_SUCCESS 0
#define DELIMITER " \n\t"

/* Code to hande keyboard  interrupt
   Ctrl-C and Ctrl-Z
*/

static void handle_signal(int sig)
{
    if(sig == 2)
        printf("\nExiting process..\n");
    else {
        printf("\n Process suspended..\n");
    }
}

// The Main Function

int main(void)
{
    /*cmd character pointer to accept input
      from user
    */
    char* cmd = (char*)malloc(BFR);

    /*code to declare ProcessID, status
      array of processIds,history buffer
      to store history of commands and
      counter
    */

    pid_t pid;
    int status;
    pid_t pidarr[10];
    int i = 0;
    int c = 0;
    char history[15][BFR];
    memset(&history, NULL, sizeof(history));

    /* Code that declares a sigaction struct
       whose structure object is used to call
       the signal handler code above
       returns error if less than 0
    */
    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = &handle_signal;
    if(sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction: ");
        return 1;
    } else if(sigaction(SIGTSTP, &act, NULL) < 0) {
        perror("sigaction: ");
        return 1;
    }

    // Code to declare a character array to parse/tokenize the command

    char* parse[MAX_ARGS];

    /*Function to tokenize the given command by the specified
      delimiter whitespace and tab
      Using strsep function to sperate strings by whitespace characters
    */
    void parseCommand(char* cmdStr)
    {
        char* sep;
        int parseCount = 0;
        char* working_str = strdup(cmdStr);
        while((sep = strsep(&working_str, DELIMITER)) != NULL && (parseCount < MAX_ARGS)) {
            parse[parseCount] = strndup(sep, BFR);
            if(strlen(parse[parseCount]) == 0) {
                parse[parseCount] = NULL;
            }
            parseCount++;
        }
    }
    /* Function to display the last 10 processIDs
       when showpids command is entered by the user
    */
    void showpids()
    {
        int k;
        if((i < 10) && (pidarr[9] == NULL)) {
            for(k = 0; k < i; k++) {
                printf("\n%d. %d", k + 1, pidarr[k]);
            }
        } else {
            for(k = 0; k < 10; k++) {
                printf("\n%d. %d", k + 1, pidarr[k]);
            }
        }
        printf("\n");
    }
    /* Code to handle change directory
       function, like cd, cd.. using
       chdir() function.
    */
    void handlecd()
    {
        if(parse[1] == NULL) {
            fprintf(stderr, "msh> Folder Name \"cd\"\n");
        } else if(chdir(parse[1]) != 0) {
            perror("msh>");
        }
    }
    
    /*Code to hande history command:
      checks if the history buffer is not 
      full.If it is not full it executes the first for
      loop, else executes the second for loop.
      The for loop iterates through the history buffer
      and prints the last 15 commands (max) if present
    */

    void handleHistory()
    {
        int x;
        if((c < 15) && (*history[14] == NULL || *history[14] == '\0')) {
            for(x = 0; x < c; x++) {
                printf("\n%d. %s", x + 1, history[x]);
            }
        } else {
            for(x = 0; x < 15; x++) {
                printf("\n%d. %s", x + 1, history[x]);
            }
        }

        printf("\n");
    }

    /*Infinitely loops through the program,
      accepts input from the user
    */
    while(1) {
        int f = 0;
        // prints out the shell promt "msh>"

        printf("msh>");
        fgets(cmd, BFR, stdin);

        // if exit or quit exit the shell

        if(!(strcmp(cmd, "exit\n") && strcmp(cmd, "quit\n")))
            exit(1);

        /* Copy all commands into history buffer except
           newline character
        */
        if(c > 14)
            c = 0;
        if(strcmp(cmd, "\n") /*&& strcmp(cmd, "history\n") && strchr(cmd, '!') == NULL*/) {
            strncpy(history[c], cmd, BFR);
            c++;
        }

        /*if newline character, go to next
          line and prompt
        */

        if(!strcmp(cmd, "\n"))
            continue;
            
        /*if the command is showpids
          call showpids function
        */

        if(!strcmp(cmd, "showpids\n")) {

            showpids();
            continue;
        }
        
        /*if the comman entered is history
          call handleHistory function
        */

        if(!strcmp(cmd, "history\n")) {
            handleHistory();
            continue;
        }
        
        /*if the command is !n where n
          is between 1 and fifteen from
          the history, execute that cmd,
          else display command not found
          in history.
        */

        if(strchr(cmd, '!') != NULL) {
            
            
            int n = 0;

            if(strlen(cmd) - 1 == 3)
                

                n = (cmd[1] - '0') * 10 + (cmd[2] - '0');
                
            else
                
                n = cmd[1] - '0';
                
            if((*history[n] == NULL) || (*history[n] == '\0')) {
                
                printf("\n Command not found in history\n");
                
                continue;
            }

            printf("you choose option %d so executing %s", n, history[n - 1]);

            if(!strcmp(history[n - 1], "showpids\n")) {
                
                showpids();
                
                continue;
                
            } else if(!strncmp(history[n - 1], "cd", 2)) {
                
                parseCommand(history[n - 1]);
                
                handlecd();
                
                continue;
                
            } else if(!strcmp(history[n - 1], "history\n")) {
                
                handleHistory();
                
                continue;
            }
            
            parseCommand(history[n - 1]);
            
        } else {
            
            parseCommand(cmd);
        }
        
        /*if the command is cd call
          the handlecd function.
        */

        if(!strncmp(cmd, "cd", 2)) {
            
            parseCommand(cmd);
            
            handlecd();
            
            continue;
        }
        
        /*We are forking a process,
          if it is a child process,
          call the execvp function
          to execute the commands.
          if execvp fails, print
          command not found.The else 
          part is the parent block,it 
          waits for the child process 
          to finish and returns.Meanwhile
          the processIDs are copied into the
          processID array.
        */

        if((pid = fork()) < 0) {
            
            exit(1);
            
        }

        else if(pid == 0) {
            if(execvp(parse[0], parse) < 0) {

                printf("Command not found: %s", cmd);

                exit(1);
            }
        }

        else {

            while(wait(&status) != pid)
                ;
            if(i > 9)
                i = 0;
            pidarr[i] = pid;
            i++;
        }
    }

    return 0;
}
