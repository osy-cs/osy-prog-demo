//***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2021
//
// Example of 'condition" between threads.
//
// Two independent threads unlock asynchronously condition in main.
// One thread works as timer, the second one read data from keyboard.
//
// Compile program with -pthread" option.
//
//***************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>

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

void help( char *t_name )
{
    printf(
        "\n"
        "  Thread demo program with condition.\n"
        "\n"
        "  Use: %s [-d -h] output_file_name\n"
        "\n"
        "    -h  this help\n"
        "    -d  debug mode \n"
        "\n", t_name );

    exit( 0 );
}

//***************************************************************************

// mutex and condition
static pthread_mutex_t g_mutex_4_cond = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_condition = PTHREAD_COND_INITIALIZER;

// global variables
FILE *g_output = nullptr;   // output file name
int g_event = 0;            // event type
char g_kbdline[ 128 ];      // string from keyboard

void *time_thread( void * )
{
    fprintf( g_output, "Thread with timeout started...\n" );

    while ( 1 )
    {
        sleep( 2 );
        pthread_cond_signal( &g_condition );
    }
}

void *kbd_thread( void * )
{
    fprintf( g_output, "Thread for keyboard started...\n" );

    while ( 1 )
    {
        printf( "Enter string: " );
        fgets( g_kbdline, sizeof( g_kbdline ), stdin );
        g_kbdline[ strlen( g_kbdline ) - 1 ] = 0;
        g_event = 1;

        pthread_cond_signal( &g_condition );
    }
}

//***************************************************************************

int main( int t_narg, char **t_args )
{
    if ( t_narg <= 1 ) help( *t_args );

    char *l_device = nullptr;

    for ( int i = 1; i < t_narg; i++ )
    {
        if ( !strcmp( t_args[ i ], "-d" ) )
            g_debug = 1;

        if ( !strcmp( t_args[ i ], "-h" ) )
            help( *t_args );

        if ( *t_args[ i ] != '-' )
        {
            if ( !l_device )
                l_device = t_args[ i ];
        }
    }

    if ( !l_device )
    {
        log_msg( LOG_INFO, "Enter file name for output!" );
        help( *t_args );
    }

    g_output = fopen( l_device, "w+" );
    if ( !g_output )
    {
        log_msg( LOG_ERROR, "Unable to open '%s' for output!", l_device );
        exit( 1 );
    }

    log_msg( LOG_INFO, "Terminate program with <CTRL-C>." );

    pthread_t l_time_id, l_kbd_id;

    // creation of thread for keyboard
    int l_err = pthread_create( &l_kbd_id, nullptr, kbd_thread, nullptr );
    if ( l_err )
        log_msg( LOG_INFO, "Unable to create thread for keyboard." );

    // creation of thread for timer
    l_err = pthread_create( &l_time_id, nullptr, time_thread, nullptr );
    if ( l_err )
        log_msg( LOG_INFO, "Unable to create timer thread." );


    while ( 1 )
    {
        pthread_cond_wait( &g_condition, &g_mutex_4_cond );

        fprintf( g_output, "Condition signal! " );

        if ( g_event )
            fprintf( g_output, "Data from keyboard '%s'.\n", g_kbdline );
        else
            fprintf( g_output, "Timer event.\n" );

        g_event = 0;
    }
}
