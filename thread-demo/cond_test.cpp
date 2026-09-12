// ***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2026
//
// Example of 'pthread condition" between threads.
//
// Two independent threads unlock asynchronously condition in main.
// One thread works as timer, the second one read data from keyboard.
//
// ***************************************************************************

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cerrno>
#include <pthread.h>
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
    "  Thread demo program with pthread condition.\n"
    "  Output can be e.g. 'tty' of another terminal.\n"
    "\n"
    "  Use: %s [-h] [-d] output_file_device_name\n"
    "\n"
    "    -h  this help\n"
    "    -d  debug mode \n"
    "\n"
    ;

//***************************************************************************

// mutex and condition
static pthread_mutex_t g_mutex_4_cond = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_condition = PTHREAD_COND_INITIALIZER;

enum cond_event_t 
{
    ce_none         = 0,
    ce_timeout,
    ce_keyboard,
};

// global variables
FILE *g_output = nullptr;       // output file name
cond_event_t g_event = ce_none; // event type
char g_kbdline[ 128 ];          // string from keyboard

void *time_thread( void * )
{
    log_msg( LOG_INFO, "Thread with timeout started..." );

    while ( 1 )
    {
        sleep( 2 );
        pthread_cond_signal( &g_condition );
        g_event = ce_timeout;
    }

    return nullptr;
}

void *kbd_thread( void * )
{
    log_msg( LOG_INFO, "Thread for keyboard started..." );

    while ( 1 )
    {
        printf( "Enter string: " );
        fgets( g_kbdline, sizeof( g_kbdline ) - 1, stdin );
        g_kbdline[ strlen( g_kbdline ) - 1 ] = 0; // replace '\n'
        g_event = ce_keyboard;

        pthread_cond_signal( &g_condition );
    }
}

//***************************************************************************

int main( int t_argc, char **t_argv )
{
    char *l_output_name = nullptr;

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
        if ( *t_argv[ inx ] != '-' )
        {
            if ( !l_output_name )
            {
                l_output_name = t_argv[ inx ];
            }
        }
    }

    if ( l_output_name == nullptr )
    {
        log_msg( LOG_INFO, "Enter file or device name for output!" );
        printf( g_help, t_argv[ 0 ] );
        exit( EXIT_FAILURE );
    }

    g_output = fopen( l_output_name, "w+" );
    if ( g_output == nullptr )
    {
        log_msg( LOG_ERROR, "Unable to open '%s' for output!", l_output_name );
        exit( EXIT_FAILURE );
    }

    log_msg( LOG_INFO, "Terminate program with <CTRL-C>." );

    pthread_t l_time_id, l_kbd_id;

    // creation of thread for keyboard
    int l_err = pthread_create( &l_kbd_id, nullptr, kbd_thread, nullptr );
    if ( l_err != 0 )
    {
        log_msg( LOG_INFO, "Unable to create thread for keyboard." );
    }

    // creation of thread for timer
    l_err = pthread_create( &l_time_id, nullptr, time_thread, nullptr );
    if ( l_err )
    {
        log_msg( LOG_INFO, "Unable to create timer thread." );
    }

    while ( 1 )
    {
        pthread_cond_wait( &g_condition, &g_mutex_4_cond );

        fprintf( g_output, "Condition signal! " );

        switch ( g_event )
        {
            case ce_keyboard:
                fprintf( g_output, "Data from keyboard '%s'.\n", g_kbdline );
                break;

            case ce_timeout:
                fprintf( g_output, "Timer event.\n" );
                break;

            default:
                break;
        }

        g_event = ce_none;
    } // while
    
} // main

