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
void execPipe(int pos, char *lineArray[], int size);
void execRedirect(char *lineArray[], char *inFile, char *outFile, int inPosition, int outPosition);
void runCommand(char *execArray[], int pfd[], pid_t pid);

int numPipes = 0;
int pipePosition[5] = {0};
int inPosition = 0;
int outPosition = 0;

// bool input, output;
int input, output;

char *inFile, *outFile;

int main(void)
{
    int pos = 0;
    int pfd[3];



    pid_t pid;
    char line[MAX_LINE_CHARS];
    char *line_words[MAX_LINE_WORDS + 1];

    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
    while (fgets(line, MAX_LINE_CHARS, stdin))
    {
        int num_words = split_cmd_line(line, line_words);
        int stdin = dup(STDINPUT);
        int stdout = dup(STDOUTPUT);
        int cmd = 0;
        input = 0;
        output = 1;


        char* command[num_words];

        if(stdin == ERROR || stdout == ERROR){
            syserror("Could not copy stdin or stdout.");
        }
        // int numRedirect;
        for (int i = 0; i < num_words; i++)
        {
            if (*line_words[i] == '<')
            {
                // input = true;
                // inPosition = i;
                // inFile = line_words[i + 1];
                execInput(line_words[i+1]);
                i++;
            }
            else if (*line_words[i] == '>')
            {
                // output = true;
                // outPosition = i;
                // outFile = line_words[i + 1];
                execOutput(line_words[i + 1]);
                i++;
            }
            else if (*line_words[i] == '|')
            {
                int pfd[2];
                if(pipe(pfd) == ERROR){
                    syserror("Could not create pipe\n");
                }
                output = pfd[1];
                execCommand(command, cmd);
                cmd = 0;
                if(close(pfd[1])== -1  ){
                    syserror("Could not close pfd\n");

                }
                dup2(stdout, STDOUTPUT);
                output = 1;

                input = pfd[0];
            }
                // numPipes += 1;
                // pipePosition[pos] = i;
                // pos++;
            // }
            else{
                command[cmd + 1] = line_words[i];
            }
            
        }
        runCommand(command, cmd);
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

        return 0;
}
        


//         if (numPipes > 0)
//         {
//             if (numPipes == 1)
//             {
               
//                 execPipe(pipePosition[0], line_words, num_words);
//             }

//         }
//         else if ((input || output) && numPipes == 0)
//         {
//             execRedirect(line_words, inFile, outFile, inPosition, outPosition);
//         }

//         else
//         {
//             execvp(line_words[0], line_words);
//         }
//     }
// }


void createInput(char *inputFile){
    input = open (inputFile, O_RDONLY);
    if(input == -1){
        syserror("Could not open file.\n");
    }
}

void createOutput (char *outputfile){
    output = open(outputfile, O_CREAT | O_WRONLY | O_TRUNC, 0777);
    if(output == -1){
        syserror("Coudl not open output file.\n");
    }
}
void runCommand(char *execArray[], int pfd[], pid_t pid){
        if ( pipe (pfd) == -1 )
    syserror( "Could not create a pipe" );

    switch ( pid = fork() ) {
        case -1:
        syserror( "First fork failed" );
        
        case  0:
        if ( close( 0 ) == -1 )
            syserror( "Could not close stdin" );
        dup(pfd[0]);
        if ( close (pfd[0]) == -1 || close (pfd[1]) == -1 )
            syserror( "Could not close pfds from first child" );
        execvp(execArray[0], execArray);
        syserror( "Could not exec second command");
            if (close(pfd[0]) == -1)
        syserror( "Parent could not close stdin" );
    if (close(pfd[1]) == -1)
        syserror( "Parent could not close stdout" );
    while ( wait(NULL) != -1) ;
    }

}

void execPipe(int pos, char *lineArray[], int size)
{
    if(numPipes == 1){
        int pos = pipePosition[0];
    }
    else{
        int pos = pipePosition[0];
        int pos2 = pipePosition[1];
    }
    
    int secondHalfSize = (size - 1) - pos;
    char *firstPipe[pipePosition[0]];
    char *secondPipe[pipePosition[1] - pipePosition[0] ];
    char *lastPipe[size - pipePosition[1] - 1];

    int pfd[2];
    pid_t pid;

     firstPipe[pos] = (char*)NULL;
     secondPipe[secondHalfSize] = (char*)NULL;

    for (int i = 0; i < pipePosition[0]; i++)
    {
        firstPipe[i] = lineArray[i];
    }
    
    if(numPipes == 1){
        for (int j = pipePosition[0] + 1; j < size; j++)
        {
            secondPipe[j] = lineArray[j];
        }
    }
    else{
        for(int j = pipePosition[0]; j < pipePosition[1]; j++){
            secondPipe[j] = lineArray[j];
        }
        for(int k = pipePosition[1]; k < size; k++){
            lastPipe[k] = lineArray[k];
        }
    }
//  runCommand(lastPipe, pfd, pid);
//  runCommand(secondPipe, pfd, pid);
//  runCommand(firstPipe, pfd, pid);
     if ( pipe (pfd) == -1 )
    syserror( "Could not create a pipe" );

     switch ( pid = fork() ) {
        case -1:
        syserror( "First fork failed" );
        
        case  0:
        if ( close( 0 ) == -1 )
            syserror( "Could not close stdin" );
        dup(pfd[0]);
        if ( close (pfd[0]) == -1 || close (pfd[1]) == -1 )
            syserror( "Could not close pfds from first child" );
        execvp(lastPipe[0], lastPipe);
        syserror( "Could not exec second command");
    }
    switch ( pid = fork() ) {
        case -1:
        syserror( "First fork failed" );
        
        case  0:
        if ( close( 0 ) == -1 )
            syserror( "Could not close stdin" );
        dup(pfd[0]);
        if ( close (pfd[0]) == -1 || close (pfd[1]) == -1 )
            syserror( "Could not close pfds from first child" );
        execvp(secondPipe[0], secondPipe);
        syserror( "Could not exec second command");
    }
    //fprintf(stderr, "The first child's pid is: %d\n", pid);
    switch ( pid = fork() ) {
        case -1:
        syserror( "Second fork failed" );
        
        case  0:
        if ( close( 1 ) == -1 )
            syserror( "Could not close stdout" );
        dup(pfd[1]);
        if ( close (pfd[0]) == -1 || close (pfd[1]) == -1 )
            syserror( "Could not close pfds from second child" );
        execvp(firstPipe[0], firstPipe);
        syserror( "Could not exec first command" );
    }
    //fprintf(stderr, "The second child's pid is: %d\n", pid);
    if (close(pfd[0]) == -1)
        syserror( "Parent could not close stdin" );
    if (close(pfd[1]) == -1)
        syserror( "Parent could not close stdout" );
    while ( wait(NULL) != -1) ;
    
}


void execRedirect(char *lineArray[], char *inFile, char *outFile, int inPosition, int outPosition){
    char *in[MAX_LINE_CHARS];
    char *out[MAX_LINE_CHARS];

    if(input){
        for(int i = 0; i < inPosition - 1; i++){
            in[i] = lineArray[i];
        }
        int openInFile = open(inFile, O_RDONLY, 0777);
        dup2(openInFile, 0);
        if(close(openInFile) == -1){
            syserror("Could not close openfile");
        }

        if(outFile){
            in[outPosition] = (char*)NULL;
            for(int i = 0; i < outPosition - 1; i++){
                out[i] = lineArray[i];
            }
            int openOutFile = open(outFile, O_CREAT | O_WRONLY | O_TRUNC, 0777);
            dup2(openOutFile, 1);
            if(close(openOutFile) == -1){
                syserror("Could not close Output File");
            }
        }
        if(output && !(input)){
            execvp(out[0], out);
            syserror("Error with output redirection");
        }

        else{
            execvp(in[0], in);
            syserror("Error with both flags redirection");
        }
         input = NULL;
        output = NULL;

    }
}

void syserror(const char *s)
{
    extern int errno;

    fprintf(stderr, "%s\n", s);
    fprintf(stderr, " (%s)\n", strerror(errno));
    exit(1);
}
