// #include <stdio.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <sys/wait.h>
// #include <sys/types.h>
// #include <errno.h>
// #include <string.h>

// #include "constants.h"
// #include "parsetools.h"

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
void syserror(const char *s);
void execPipe(int pos, char *lineArray[], int size);
void execRedirect(char *lineArray[], char *inFile, char *outFile, int inPosition, int outPosition);

int numPipes = 0;
int pipePosition[1000];
int inPosition = 0;
int outPosition = 0;

bool input, output;

char *inFile, *outFile;

int main(void)
{
    int pos = 0;
    int pfd[2];
    pid_t pid;
    char line[MAX_LINE_CHARS];
    char *line_words[MAX_LINE_WORDS + 1];

    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
    while (fgets(line, MAX_LINE_CHARS, stdin))
    {
        int num_words = split_cmd_line(line, line_words);

        int numRedirect;
        for (int i = 0; i < num_words; i++)
        {
            if (*line_words[i] == '<')
            {
                input = true;
                inPosition = i;
                inFile = line_words[i + 1];
            }
            if (*line_words[i] == '>')
            {
                output = true;
                outPosition = i;
                outFile = line_words[i + 1];
            }
            if (*line_words[i] == '|')
            {
                numPipes += 1;
                pipePosition[pos] = i;
                pos++;
            }
        }

        if (numPipes > 0)
        {
            if (numPipes == 1)
            {
               
                execPipe(pipePosition[0], line_words, num_words);
            }
        }
        else if ((input || output) && numPipes == 0)
        {
            execRedirect(line_words, inFile, outFile, inPosition, outPosition);
        }

        else
        {
            execvp(line_words[0], line_words);
        }
    }
}

void execPipe(int pos, char *lineArray[], int size)
{
    int secondHalfSize = (size - 1) - pos;
    char *firstHalf[pos];
    char *secondHalf[secondHalfSize];

    int pfd[2];
    pid_t pid;

     firstHalf[pos] = (char*)NULL;
     secondHalf[secondHalfSize] = (char*)NULL;

    for (int i = 0; i < pos - 1; i++)
    {
        firstHalf[i] = lineArray[i];
    }
    for (int j = pos + 1; j < size; j++)
    {
        secondHalf[j] = lineArray[j];
    }
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
        execvp(secondHalf[0], secondHalf);
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
        execvp(firstHalf[0], firstHalf);
        syserror( "Could not exec first command" );
    }
    //fprintf(stderr, "The second child's pid is: %d\n", pid);
    if (close(pfd[0]) == -1)
        syserror( "Parent could not close stdin" );
    if (close(pfd[1]) == -1)
        syserror( "Parent could not close stdout" );
    while ( wait(NULL) != -1) ;
    
}
// 



void execRedirect(char *lineArray[], char *inFile, char *outFile, int inPosition, int outPosition){
    char *in[MAX_LINE_CHARS];
    char *out[MAX_LINE_CHARS];

    if(input){
        // in[inPosition] = (char*)NULL;
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

        // if(input && !(output)){
        //     execvp(in[0], in);
        //     syserror("Error with input redirection");

        // }
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
