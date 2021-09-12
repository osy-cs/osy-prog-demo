//***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2021
//
// The example of using pipe(), fork(), dup2() and exec() functions.
// The parent create pipe and child. 
// Child redirect stdout to pipe and exec "ls". 
// Parent read file names of current directory. 
//
//***************************************************************************//***************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <sys/types.h>

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
            "  Stdio and stdout redirection. \n"
            "\n"
            "  Use: %s\n"
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
    help( t_narg, t_args );

    int l_mypipe[ 2 ];

    if ( pipe( l_mypipe ) < 0 )
    {
        log_msg( LOG_ERROR, "Unable to create pipe!" );
        exit( 1 );
    }

    int l_child = fork();

    if ( l_child < 0 )
    {
        log_msg( LOG_ERROR, "Unable to create new process!" );
        exit( 1 );
    }

    if ( l_child == 0 )
    { // child
        close( l_mypipe[ 0 ] );

        // redirection of stdin from pipe
        dup2( l_mypipe[ 1 ], STDOUT_FILENO );
        // reopen required for FILE* stdin, stdout and stderr 
        freopen( nullptr, "r", stdin );
        // close useless pipe
        close( l_mypipe[ 1 ] );

        execlp( "ls", "ls", nullptr );

        log_msg( LOG_ERROR, "Function exec... failed" );

        exit( 1 );
    }
    else
    { // parent

        close( l_mypipe[ 1 ] ); 
       
        log_msg( LOG_INFO, "Content of current directory: \n" );
        // read from pipe
        while ( 1 )
        {
            char l_buf[ 1024 ];
            int l_len = read( l_mypipe[ 0 ], l_buf, sizeof( l_buf ) );
            if ( l_len < 0 )
            {
                log_msg( LOG_ERROR, "Function read failed!" );
            }
            if ( l_len <= 0 )
            {
                break;
            }

            int l_ret = write( STDOUT_FILENO, l_buf, l_len );
            if ( l_ret < 0 )
            {
                log_msg( LOG_ERROR, "Function write failed!" );
                break;
            }
        }

        close( l_mypipe[ 0 ] ); 

        wait( nullptr );

        log_msg( LOG_INFO, "Child process finished." );
    }

    return 0;
}
