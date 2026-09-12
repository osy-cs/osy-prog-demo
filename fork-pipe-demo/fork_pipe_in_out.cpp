// ***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2026
//
// The example of using pipe(), fork() and dup2() functions.
// The parent redirects stdout to pipe and the child redirects stdin from pipe.
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
    "  Stdin and stdout redirection.\n"
    "\n"
    "  Use: %s [-h] [-d]\n"
    "\n"
    "    -h  this help\n"
    "    -d  debug mode \n"
    "\n"
    ;

// ***************************************************************************

void producer()
// Producer sends data to stdout.
{
    int l_count = 0;

    while ( 1 )
    {
        fprintf( stdout, "%d\n", l_count++ );
    }
}


void consumer()
// consumer reads data from stdin
{
    while ( 1 )
    {
        int num;
        fscanf( stdin, "%d", &num );
        fprintf( stderr, "Consumer read: %d\n", num );
    }
}


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

    if ( pipe( l_mypipe ) < 0 )
    {
        log_msg( LOG_ERROR, "Unable to create pipe!" );
        exit( EXIT_FAILURE );
    }

    int l_child = fork();

    if ( l_child < 0 )
    {
        log_msg( LOG_ERROR, "Unable to create new process!" );
        exit( EXIT_FAILURE );
    }

    if ( l_child != 0 )
    { // parent
        // redirection of stdin from pipe
        dup2( l_mypipe[ 1 ], STDOUT_FILENO );
        // reopen required for FILE* stdin, stdout and stderr 
        freopen( nullptr, "r", stdin );
        // close useless pipe
        close( l_mypipe[ 0 ] );
        close( l_mypipe[ 1 ] );
        
        producer();
    }
    else
    { // child
        // redirection of stdout to pipe
        dup2( l_mypipe[ 0 ], STDIN_FILENO );
        // reopen required for FILE* stdin, stdout and stderr 
        freopen( nullptr, "w", stdout );
        // close useless pipe
        close( l_mypipe[ 0 ] );
        close( l_mypipe[ 1 ] );

        consumer();
    }

    return EXIT_SUCCESS;
} // main

