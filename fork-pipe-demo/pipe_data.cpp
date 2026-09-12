//***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2026
//
// The example of using pipe() and fork() functions.
// Parent process creates child process and
// parent passes data to child via pipe.
//
// ***************************************************************************

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cstdarg>
#include <unistd.h>

// ***************************************************************************
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

    if ( t_log_level && t_log_level > g_debug ) 
    {
        return;
    }

    char l_buf[ 4096 ];
    va_list l_arg;
    va_start( l_arg, t_form );

    vsnprintf( l_buf, sizeof( l_buf ), t_form, l_arg );

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

// ***************************************************************************
// help

const char g_help[] = 
    "\n"
    "  Fork() and pipe() example.\n"
    "\n"
    "  Use: %s [-h] [-d]\n"
    "\n"
    "    -h  this help\n"
    "    -d  debug mode \n"
    "\n"
    ;

// ***************************************************************************

void producer( int t_handle )
// The producer writes in infinite loop integers to pipe.
// Information about production is sent to stdout.
{
    int l_count = 0, l_total = 0;
    char l_buf[ 128 ];

    while ( 1 )
    {
        sprintf( l_buf, " %d", l_count++ );

        int l_ret = write( t_handle, l_buf, strlen( l_buf ) );

        if ( l_ret < 0 )
        {
            log_msg( LOG_ERROR, "Unable to write to pipe (fd %d)!", t_handle );
            exit( EXIT_FAILURE );
        }
        else
        {
            log_msg( LOG_DEBUG, "Function writes to pipe (fd %d) %d bytes", t_handle, l_ret );
        }

        l_total += l_ret;

        fprintf( stdout, "Generated data: '%s', total %d bytes.\n", l_buf, l_total );
    }
} // producer


void consumer( int t_handle )
// The consumer reads data from pipe and displays them on screen.
// Information about consumer activity is sent to stderr.
{
    int l_total = 0;
    char l_buf[ 128 ];

    while ( 1 )
    {
        int l_ret = read( t_handle, l_buf, sizeof( l_buf ) );

        if ( l_ret < 0 )
        {
            log_msg( LOG_ERROR, "Unable to read from pipe - (fd %d)!", t_handle );
            exit( EXIT_FAILURE );
        }
        else if ( l_ret == 0 )
        { 
            log_msg( LOG_INFO, "Pipe closed. Why?" );
            exit( EXIT_SUCCESS );
        }
        else
        {
            log_msg( LOG_DEBUG, "Functions read (fd %d) %d bytes", t_handle, l_ret );
        }

        // string termination
        l_buf[ l_ret ] = '\0';

        l_total += l_ret;

        fprintf( stderr, "          Received '%s', consumed %d bytes.\n", l_buf, l_total );
    }
} // consumer


int main( int t_argc, char **t_argv )
{

    for ( int inx = 1; inx < t_argc; inx++ )
    {
        if ( strcmp( t_argv[ inx ], "-h" ) == 0 )
        {
            printf( g_help, t_argv[ 0 ] );
            exit( EXIT_SUCCESS );
        }
        if ( strcmp( t_argv[ inx ], "-d" ) == 0 )
        {
            g_debug = 1;
        }
    }

    int l_mypipe[ 2 ];

    // create pipe
    if ( pipe( l_mypipe ) < 0 )
    {
        log_msg( LOG_ERROR, "Unable to create pipe!" );
        exit( EXIT_FAILURE );
    }

    // create child process
    int l_child = fork();

    if ( l_child < 0 )
    {
        log_msg( LOG_ERROR, "Unable to create new process!" );
        exit( EXIT_FAILURE );
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

    return EXIT_SUCCESS;
} // main

