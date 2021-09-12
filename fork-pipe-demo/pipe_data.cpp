//***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2021
//
// The example of using pipe() and fork() functions.
// Parent process creates child process and
// parent passes data to child via pipe.
//
//***************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
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
            "  Fork() and pipe() example.\n"
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

void producer( int t_handle )
// The producer writes in infinite loop integers to pipe.
// Information about production is sent to stdout.
{
    int l_count = 0, l_total = 0;
    char l_buf[ 128 ];

    while ( 1 )
    {
        sprintf( l_buf, "%d\n", l_count++ );

        int ret = write( t_handle, l_buf, strlen( l_buf ) );
        if ( ret < 0 )
        {
            log_msg( LOG_ERROR, "Unable to write to pipe (fd %d)!", t_handle );
            exit( 1 );
        }
        else
            log_msg( LOG_DEBUG, "Function writes to pipe (fd %d) %d bytes", t_handle, ret );

        l_total += ret;

        fprintf( stdout, "Generated %d bytes.\n", l_total );
    }
}


void consumer( int t_handle )
// The consumer reads data from pipe and displays them on screen.
// Information about consumer activity is sent to stderr.
{
    int l_total = 0;
    char l_buf[ 128 ];

    while ( 1 )
    {
        int ret = read( t_handle, l_buf, sizeof( l_buf ) );
        if ( ret < 0 )
        {
            log_msg( LOG_ERROR, "Unable to read from pipe - (fd %d)!", t_handle );
            exit( 1 );
        }
        else
            log_msg( LOG_DEBUG, "Functions read (fd %d) %d bytes", t_handle, ret );

        l_total += ret;

        fprintf( stderr, "          Consumed %d bytes.\n", l_total );
    }
}

int main( int t_num, char **t_arg )
{
    help( t_num, t_arg );

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

    if ( l_child != 0 )
    { // parent
        close( l_mypipe[ 0 ] );
        producer( l_mypipe[ 1 ] );
    }
    else
    { // child
        close( l_mypipe[ 1 ] );
        consumer( l_mypipe[ 0 ] );
    }

    return 0;
}
