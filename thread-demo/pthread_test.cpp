//***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2021
//
// Example of thread usage.
//
// Program require parameter to specify number of thread.
// Every thread will wait random time.
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
#include <stdint.h>
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
        "  Thread demo program.\n"
        "\n"
        "  Use: %s [-d -h] number_of_threads\n"
        "\n"
        "    -h  this help\n"
        "    -d  debug mode \n"
        "\n", t_name );

    exit( 0 );
}

//***************************************************************************

// global variable for all threads
int g_counter = 0;

void *demo_thread( void *t_par )
{
    int *l_ppar = ( int * ) t_par;
    int l_my_id = l_ppar[ 0 ];
    int l_secs = l_ppar[ 1 ];

    log_msg( LOG_INFO, "Thread my_id: %d started and will run %d seconds.", l_my_id, l_secs );

    usleep( l_my_id * 100000 ); // small start delay

    sleep( 1 );

    for ( int i = 1; i < l_secs; i++ )
    {
        g_counter++;
        log_msg( LOG_INFO, "(%05d) Thread my_id: %d is still running ...", g_counter, l_my_id );
        sleep( 1 );
    }

    log_msg( LOG_INFO, "Thread my_id: %d fished.", l_my_id );

    pthread_exit( ( void * ) ( ( intptr_t ) - l_my_id ) );
}

//***************************************************************************

int main( int argn, char **arg )
{
    if ( argn <= 1 ) help( *arg );

    int numthread = 0;
    for ( int i = 1; i < argn; i++ )
    {
        if ( !strcmp( arg[ i ], "-d" ) )
            g_debug = LOG_DEBUG;

        if ( !strcmp( arg[ i ], "-h" ) )
            help( *arg );

        if ( *arg[ i ] != '-' && !numthread )
        {
            numthread = atoi( arg[ i ] );
            break;
        }
    }

    if ( numthread <= 0 )
    {
        log_msg( LOG_INFO, "Number of thread is missing or it is incorrect!" );
        help( *arg);
    }

    log_msg( LOG_INFO, "%d thread will be created.", numthread );


    int thread_param[ numthread ][ 2 ];   // threads parameters
    pthread_t thread_id[ numthread ];     // threads identification
    void* thread_status[ numthread ];     // return values from threads


    // thread initialization - thread id and random timeout
    srand( getuid() );
    for ( int i = 0; i < numthread; i++ )
    {
        thread_param[ i ][ 0 ] = i + 1;
        thread_param[ i ][ 1 ] = rand() % 20 + 1;
    }

    // threads creation
    for ( int i = 0; i < numthread; i++ )
    {
        int err = pthread_create( &thread_id[ i ], nullptr, demo_thread, thread_param[ i ] );
        if ( err )
            log_msg( LOG_INFO, "Unable to create thread %d.", i );
        else
            log_msg( LOG_DEBUG, "Thread %d created - system id 0x%X.", i, thread_id[ i ] );
    }


    // wait for all threads
    for ( int i = 0; i < numthread; i++ )
        pthread_join( thread_id[ i ], &thread_status[ i ] );


    log_msg( LOG_INFO, "All threads finished..." );

    for ( int i = 0; i < numthread; i++ )
        log_msg( LOG_INFO, "Thread my_id: %d finished with return code: %d.",
          i + 1, thread_status[ i ] );
}
