//***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2021
//
// Example of mutex usage for thread synchronization.
//
// Program requite two arguments: file name for output and number of threads.
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
        "  Thread demo program with mutexes.\n"
        "\n"
        "  Use: %s [-d -h] output_file_name number_of_threads\n"
        "\n"
        "    -h  this help\n"
        "    -d  debug mode \n"
        "\n", t_name );

    exit( 0 );
}

//***************************************************************************

// mutex for critical section
static pthread_mutex_t g_lock_critical_section = PTHREAD_MUTEX_INITIALIZER;

FILE *g_output = nullptr;

void *demo_thread( void *t_par )
{
    int l_my_id = * ( int *) t_par;

    while ( 1 )
    {
        // try lock critical section
        log_msg( LOG_INFO, "Thread my_id: %d will try to enter into critical section..." );
        pthread_mutex_lock( &g_lock_critical_section );

        log_msg( LOG_INFO, "Thread my_id: %d entered to critical section ...", l_my_id );

        // critical section
        int l_max = 10 + rand() % 35;
        fprintf( g_output, "Critical section in thread my_id: %d ", l_my_id );
        for ( int cnt = 0; cnt < l_max; cnt++ )
        {
            fprintf( g_output, "." );
            fflush( g_output );
            usleep( 20000 );
        }
        fprintf( g_output, "%d end.\n", l_my_id );

        // unlock critical section
        pthread_mutex_unlock( &g_lock_critical_section );

        log_msg( LOG_INFO, "Thread my_id: %d leaved critical section.", l_my_id );

        usleep( 100000 * ( rand() % 20 ) );
    }
}

//***************************************************************************

int main( int t_narg, char **t_args )
{
    if ( t_narg <= 1 ) help( *t_args );

    // parsing argument
    int l_numthread = 0;
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
            else if ( !l_numthread )
                l_numthread = atoi( t_args[ i ] );
        }
    }

    if ( !l_device || l_numthread <= 0 )
    {
        log_msg( LOG_INFO, "Invalid arguments!" );
        help( *t_args );
    }

    g_output = fopen( l_device, "w+" );
    if ( !g_output )
    {
        log_msg( LOG_ERROR, "Unable to open file '%s' for output.", l_device );
        exit( 0 );
    }

    log_msg( LOG_INFO, "%d threads will be created.", l_numthread );
    log_msg( LOG_INFO, "Terminate program with <CTRL-C>." );

    int l_thread_param[ l_numthread ][ 2 ];   // threads parameters
    pthread_t l_thread_id[ l_numthread ];     // threads identification
    void* l_thread_status[ l_numthread ];     // return values from threads


    // thread initialization - thread id and random timeout
    srand( getuid() );
    for ( int i = 0; i < l_numthread; i++ )
    {
        l_thread_param[ i ][ 0 ] = i + 1;
        l_thread_param[ i ][ 1 ] = rand() % 20 + 1;
    }

    // threads creation
    for ( int i = 0; i < l_numthread; i++ )
    {
        int l_err = pthread_create( &l_thread_id[ i ], nullptr, demo_thread, l_thread_param[ i ] );
        if ( l_err )
            log_msg( LOG_INFO, "Unable to create thread %d.", i );
        else
            log_msg( LOG_DEBUG, "Thread %d created - system id 0x%X.", i, l_thread_id[ i ] );
    }


    // wait for all threads
    for ( int i = 0; i < l_numthread; i++ )
        pthread_join( l_thread_id[ i ], &l_thread_status[ i ] );


    log_msg( LOG_INFO, "All threads finished..." );

    for ( int i = 0; i < l_numthread; i++ )
        log_msg( LOG_INFO, "Thread my_id: %d finished with return code: %d.",
          i + 1, l_thread_status[ i ] );
}
