// ***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2026
//
// Example of mutex usage for thread synchronization.
//
// Program requite two arguments: file name for output and number of threads.
//
// ***************************************************************************

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cerrno>
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
    "  Thread demo program with mutex.\n"
    "\n"
    "  Use: %s [-h] [-d] output_file_device_name number_of_threads\n"
    "\n"
    "    -h  this help\n"
    "    -d  debug mode \n"
    "\n"
    ;

//***************************************************************************

// mutex for critical section
static pthread_mutex_t g_lock_critical_section = PTHREAD_MUTEX_INITIALIZER;

FILE *g_output = nullptr;

void *demo_thread( void *t_par )
{
    int l_my_id = ( ( int * ) t_par )[ 0 ];
    int l_tout = ( ( int * ) t_par )[ 1 ]; 

    while ( l_tout > 0 )
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

        l_tout--;
    }

    return nullptr;
}

//***************************************************************************

int main( int t_argc, char **t_argv )
{
    // parsing argument
    int l_numthread = -1;
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
            if ( l_output_name == nullptr )
            {
                l_output_name = t_argv[ inx ];
            }
            else if ( l_numthread == -1 )
            {
                l_numthread = atoi( t_argv[ inx ] );
            }
        }
    }

    if ( !l_output_name || l_numthread <= 0 )
    {
        log_msg( LOG_INFO, "Invalid arguments!" );
        printf( g_help, t_argv[ 0 ] );
        exit( EXIT_FAILURE );
    }

    g_output = fopen( l_output_name, "w+" );
    if ( !g_output )
    {
        log_msg( LOG_ERROR, "Unable to open file or device '%s' for output.", l_output_name );
        exit( EXIT_FAILURE );
    }

    log_msg( LOG_INFO, "%d threads will be created.", l_numthread );
    log_msg( LOG_INFO, "Terminate program with <CTRL-C>." );

    std::vector<int[ 2 ]>  l_pthread_param( l_numthread );  // threads parameters
    std::vector<pthread_t> l_pthread_id( l_numthread );     // threads identification
    std::vector<void*>     l_pthread_status( l_numthread ); // return values from threads

    // thread initialization - thread id and random timeout
    srand( getuid() );
    for ( int inx = 0; inx < l_numthread; inx++ )
    {
        l_pthread_param[ inx ][ 0 ] = inx + 1;
        l_pthread_param[ inx ][ 1 ] = rand() % 20 + 1;
    }

    // threads creation
    for ( int inx = 0; inx < l_numthread; inx++ )
    {
        int l_err = pthread_create( &l_pthread_id[ inx ], nullptr, demo_thread, l_pthread_param[ inx ] );
        if ( l_err != 0 )
        {
            log_msg( LOG_INFO, "Unable to create thread %d.", inx );
        }
        else
        {
            log_msg( LOG_DEBUG, "Thread %d created - system id 0x%X.", inx, l_pthread_id[ inx ] );
        }
    }

    // wait for all threads
    for ( int inx = 0; inx < l_numthread; inx++ )
    {
        pthread_join( l_pthread_id[ inx ], &l_pthread_status[ inx ] );
    }


    log_msg( LOG_INFO, "All threads finished..." );

    for ( int inx = 0; inx < l_numthread; inx++ )
    {
        log_msg( LOG_INFO, "Thread my_id: %d finished with return code: %d.",
            inx + 1, l_pthread_status[ inx ] );
    }

} // main

