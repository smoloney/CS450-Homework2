#include<stdio.h> 
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<errno.h>
#include<string.h>
#include "constants.h"
#include "parsetools.h"

void syserror( const char * );

int main(void)
{
    int pfd[2];
    pid_t pid;
    // char line[MAX_LINE_CHARS];
    // char* line_words[MAX_LINE_WORDS + 1];

    // // Loop until user hits Ctrl-D (end of input)
    // // or some other input error occurs
    // while( fgets(line, MAX_LINE_CHARS, stdin) ) {
    //     int num_words = split_cmd_line(line, line_words);

    //     // Just for demonstration purposes
    //     for (int i=0; i < num_words; i++)
    //         printf("%s\n", line_words[i]);

    

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
            // execvp(line_words[0], line_words);
            // execlp("wc", "wc", "pipe_demo.c", NULL);
            syserror( "Could not exec wc");
    }
    fprintf(stderr, "The first child's pid is: %d\n", pid);
    switch ( pid = fork() ) {
        case -1: 
            syserror( "Second fork failed" );
        case  0: 
            if ( close( 1 ) == -1 )
                syserror( "Could not close stdout" );
            dup(pfd[1]);
            if ( close (pfd[0]) == -1 || close (pfd[1]) == -1 )
                syserror( "Could not close pfds from second child" );
            execlp("who", "who", NULL);
            syserror( "Could not exec who" );
    }
    fprintf(stderr, "The second child's pid is: %d\n", pid);
    if (close(pfd[0]) == -1)
        syserror( "Parent could not close stdin" );
    if (close(pfd[1]) == -1)
        syserror( "Parent could not close stdout" );
    while ( wait(NULL) != -1) ;
    } 


void syserror(const char *s)
{
    extern int errno;

    fprintf( stderr, "%s\n", s );
    fprintf( stderr, " (%s)\n", strerror(errno) );
    exit( 1 );
}

