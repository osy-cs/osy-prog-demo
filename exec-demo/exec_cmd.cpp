//***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2021
//
// Example of using exec..() function.
// The current process will ask user for command (program) and
// this program will be executed in child process.
//
//***************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

//***************************************************************************
// log messages

#define LOG_ERROR               0       // errors
#define LOG_INFO                1       // information and notifications
#define LOG_DEBUG               2       // debug messages

// debug flag
int g_debug = LOG_INFO;

void log_msg( int t_log_level, const char *t_form, ... )
{
    const char *out_fmt[] = {
            "ERR: (%d-%s) %s\n",
            "INF: %s\n",
            "DEB: %s\n" };

    if ( t_log_level && t_log_level > g_debug ) return;

    char l_buf[ 1024 ];
    va_list l_arg;
    va_start( l_arg, t_form );
    vsprintf( l_buf, t_form, l_arg );
    va_end( l_arg );

    switch ( t_log_level )
    {
    case LOG_INFO:
    case LOG_DEBUG:
        fprintf( stdout, out_fmt[ t_log_level ], l_buf );
        break;

    case LOG_ERROR:
        fprintf( stderr, out_fmt[ t_log_level ], errno, strerror( errno ), l_buf );
        break;
    }
}

//***************************************************************************
// help

void help( int t_num, char **t_arg )
{
    if ( t_num <= 1 ) return;

    if ( !strcmp( t_arg[ 1 ], "-h" ) )
    {
        printf(
            "\n"
            "  Example of using exec..() function.\n"
            "\n"
            "  Use: %s [-h -d]\n"
            "\n"
            "    -h  this help\n"
            "    -d  debug mode \n"
            "\n", t_arg[ 0 ] );

        exit( 0 );
    }

    if ( !strcmp( t_arg[ 1 ], "-d" ) )
        g_debug = LOG_DEBUG;
}
//***************************************************************************

int main( int t_narg, char **t_args )
{
    char *l_arg[ 1024 ];
    char l_line[ 1024 ];

    help( t_narg, t_args );

    while ( 1 )
    {
        printf( "Enter command > " );
        fgets( l_line, sizeof( l_line ), stdin );
    
        int i = 0;
        int n = 0;
        int l_len = strlen( l_line );
        while ( i < l_len )
        { // split line to array of words
            while ( i < l_len && isspace( l_line[ i ] ) ) l_line[ i++ ] = '\0';
            if ( i >= l_len ) break;
    
            l_arg[ n++ ] = &l_line[ i ];
    
            while ( i < l_len && !isspace( l_line[ i ] ) ) i++;
        }
    
        l_arg[ n ] = nullptr;
    
        if ( !n )
        {
            log_msg( LOG_INFO, "No command to execute!" );
            continue;
        }

        int l_pid = fork();
        if ( l_pid < 0 )
        {
            log_msg( LOG_ERROR, "Unable to create new process!" );
            continue;
        }

        if ( l_pid == 0 )
        { // child
            // current program in child process will be replaced by new program
            n = execvp( l_arg[ 0 ], l_arg );
    
            // current program continues only if exec failed
            if ( n < 0 )
                log_msg( LOG_ERROR, "Unable to execute command!" );

            // process exit with status 1
            exit( 1 );
        }
        else
        { // parent
            // wait for child
            int l_status;
            waitpid( l_pid, &l_status, 0 );
            printf( "\n\nStatus code of child: %d.\n", WEXITSTATUS( l_status ) );
        }
    }
}
