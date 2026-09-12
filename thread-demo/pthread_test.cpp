// ***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2026
//
// Example of thread usage.
//
// Program require parameter to specify number of thread.
// Every thread will wait random time.
//
// ***************************************************************************

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cerrno>
#include <atomic>
#include <vector>
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
    "  Thread demo program.\n"
    "\n"
    "  Use: %s [-h] [-d ] number_of_threads\n"
    "\n"
    "    -h  this help\n"
    "    -d  debug mode \n"
    "\n"
    ;

// ***************************************************************************

// global safe counter for all threads
std::atomic<int> g_counter = 0;

void *demo_thread( void *t_par )
{
    int *l_ppar = ( int * ) t_par;
    int l_my_id = l_ppar[ 0 ];
    int l_secs = l_ppar[ 1 ];

    log_msg( LOG_INFO, "Thread my_id: %d started and will run %d seconds.", l_my_id, l_secs );

    usleep( l_my_id * 100000 ); // small start delay

    for ( int inx = 1; inx < l_secs; inx++ )
    {
        g_counter++;
        log_msg( LOG_INFO, "(%05d) Thread my_id: %d is still running ...", g_counter.load(), l_my_id );
        sleep( 1 );
    }

    log_msg( LOG_INFO, "Thread my_id: %d fished.", l_my_id );

    pthread_exit( ( void * ) ( ( intptr_t ) - l_my_id ) );
}

// ***************************************************************************

int main( int t_argc, char **t_argv )
{
    int l_numthreads = -1;

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
             if ( l_numthreads == -1 )
            {
                l_numthreads = atoi( t_argv[ inx ] );
            }
        }
    }

    if ( l_numthreads <= 0 )
    {
        log_msg( LOG_INFO, "Number of thread is missing or it is incorrect!" );
        printf( g_help, t_argv[ 0 ] );
        exit( EXIT_FAILURE );
    }

    log_msg( LOG_INFO, "%d thread will be created.", l_numthreads );

    std::vector<int[ 2 ]> l_thread_param( l_numthreads );   // threads parameters
    std::vector<pthread_t> l_thread_id( l_numthreads );     // threads identification
    std::vector<void*> l_thread_status( l_numthreads );     // return values from threads

    // thread initialization - thread id and random timeout
    srand( getuid() );
    for ( int inx = 0; inx < l_numthreads; inx++ )
    {
        l_thread_param[ inx ][ 0 ] = inx + 1;
        l_thread_param[ inx ][ 1 ] = rand() % 20 + 1;
    }

    // threads creation
    for ( int inx = 0; inx < l_numthreads; inx++ )
    {
        int l_err = pthread_create( &l_thread_id[ inx ], nullptr, demo_thread, l_thread_param[ inx ] );
        if ( l_err != 0 )
        {
            log_msg( LOG_INFO, "Unable to create thread %d.", inx );
        }
        else
        {
            log_msg( LOG_DEBUG, "Thread %d created - system id 0x%X.", inx, l_thread_id[ inx ] );
        }
    }

    // wait for all threads
    for ( int inx = 0; inx < l_numthreads; inx++ )
    {
        pthread_join( l_thread_id[ inx ], &l_thread_status[ inx ] );
    }

    log_msg( LOG_INFO, "All threads finished..." );

    for ( int inx = 0; inx < l_numthreads; inx++ )
    {
        log_msg( LOG_INFO, "Thread my_id: %d finished with return code: %d.",
          inx + 1, l_thread_status[ inx ] );
    }

} // main

