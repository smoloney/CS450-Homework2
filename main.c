#include <stdio.h>
#include "constants.h"
#include "parsetools.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#define STDINPUT 0
#define STDOUTPUT 1
#define ERROR -1

void syserror(const char *s);
void createInput(char *inputFile);
void createOutput(char *outputfile);

void runCommand(char **execArray, int size);
int input, output;

int main(void)
{

    char line[MAX_LINE_CHARS];
    char *line_words[MAX_LINE_WORDS + 1];

    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
    while (fgets(line, MAX_LINE_CHARS, stdin))
    {
        int num_words = split_cmd_line(line, line_words);
        int stdin = dup(STDINPUT);
        int stdout = dup(STDOUTPUT);
        int idx = 0;
        input = 0;
        output = 1;


        char* inputCommand[num_words];

        if(stdin == ERROR || stdout == ERROR){
            syserror("Could not copy stdin or stdout.");
        }
        for (int i = 0; i < num_words; i++)
        {
            if ((strcmp(line_words[i], "<")) == 0){
            
                createInput(line_words[i+1]);
                i++;
            }
            else if ((strcmp(line_words[i], ">")) == 0)
            {

                createOutput(line_words[i + 1]);
                i++;
            }
            else if ((strcmp(line_words[i], "|")) == 0)
            {
                int pfd[2];
                if(pipe(pfd) == ERROR){
                    syserror("Could not create pipe\n");
                }
                output = pfd[1];

                runCommand(inputCommand, idx);
                idx = 0;
                if(close(pfd[1])== -1  ){
                    syserror("Could not close pfd\n");

                }
                dup2(stdout, STDOUTPUT);
                output = 1;

                input = pfd[0];
            }
            else{
                inputCommand[idx++] = line_words[i];
            }
            
        }
        runCommand(inputCommand, idx);
        if(input != 0){
            dup2(stdin, STDINPUT);
        }
        if(output != 1){
            dup2(stdout, STDOUTPUT);
         }
        if(close (stdout) == -1 || close(stdin) == -1){
            syserror("Error closing stdin or stdout\n");
        }
    }
        wait(NULL);
    if(close(STDINPUT) == -1){
        syserror("Error closing stdinput\n");
    }

        return 0;
}
    

void createInput(char *inputFile){
    input = open (inputFile, O_RDONLY);
    if(input == -1){
        syserror("Could not open file.\n");
    }
}

void createOutput (char *outputfile){
    output = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if(output == -1){
        syserror("Coudl not open output file.\n");
    }
}


void runCommand(char ** execArray, int size){
    int pid = fork();
    if(pid < 0){
        syserror("Error with creating a fork\n");
    }
    execArray[size] = (char *)NULL;
    if(pid == 0){
        if(input != 0){
            dup2(input, STDINPUT);
            if(close(input) == ERROR){
                syserror("Error closing the input");
            }
        }

        if(output != STDOUTPUT){
            dup2(output, STDOUTPUT);
            if(close(output) == ERROR){
                syserror("ERROR CLOSING OUTPUT\n");
            }
        }
        if(size > 1){
            execvp(execArray[0], execArray);
        }
        else{
            execlp(execArray[0], execArray[0], (char *) NULL);
        }
    }
   
       
    }
      

void syserror(const char *s)
{
    extern int errno;

    fprintf(stderr, "%s\n", s);
    fprintf(stderr, " (%s)\n", strerror(errno));
    exit(1);
}
