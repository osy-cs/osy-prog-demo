// ***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2026
//
// Example of posix semaphores.
// The first process will creates two semaphores.
// One semaphore will protect artificial critical section.
// The second semaphore is used as process number counter.
// The process which exits last will clean semaphores.
//
// ***************************************************************************

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <csignal>
#include <cstdarg>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>

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
    "  Posix semaphore example.\n"
    "\n"
    "  Use: %s [-h] [-d] [-r]\n"
    "\n"
    "    -h  this help\n"
    "    -d  debug mode \n"
    "    -r  remove semaphores only\n"
    "\n"
    ;

// ***************************************************************************

#define SEM_MUTEX_NAME      "/sem_mutex"
#define SEM_COUNTER_NAME    "/sem_counter"

sem_t *g_sem_mutex = nullptr;
sem_t *g_sem_counter = nullptr;

// ***************************************************************************
// cleaning function

void clean( void )
{
    log_msg( LOG_INFO, "Final cleaning ..." );

    // decrease number of processes
    if ( sem_trywait( g_sem_counter ) < 0 )
    {
        log_msg( LOG_ERROR, "Unable to decrease number of processes." );
        return;
    }

    int l_val;
    if ( sem_getvalue( g_sem_counter, &l_val ) < 0 )
    {
        log_msg( LOG_ERROR, "Unable to get number of processes." );
        return;
    }

    log_msg( LOG_INFO, "Number of remaining processes is %d", l_val );

    if ( l_val == 0 )
    {
        log_msg( LOG_INFO, "This process is last. Clean semaphores ..." );
        if ( sem_unlink( SEM_COUNTER_NAME ) < 0 ||
             sem_unlink( SEM_MUTEX_NAME ) < 0 )
        {
            log_msg( LOG_ERROR, "Unable to remove semaphores!" );
        }
        else
        {
            log_msg( LOG_INFO, "Semaphores removed." );
        }
    }
}

// catch signal
void catch_sig( int )
{
    exit( EXIT_FAILURE );
}

// ***************************************************************************

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
        if ( strcmp( t_argv[ inx ], "-r" ) == 0 )
        {
            log_msg( LOG_INFO, "Clean semaphores." );
            if ( sem_unlink( SEM_COUNTER_NAME ) < 0 ||
                 sem_unlink( SEM_MUTEX_NAME ) < 0 ) 
            {
                log_msg( LOG_ERROR, "Unable to remove semaphores!" );
                exit( EXIT_FAILURE );
            }
            log_msg( LOG_ERROR, "Semaphores removed." );
            exit( EXIT_SUCCESS );
        }
    }

    g_sem_counter = sem_open( SEM_COUNTER_NAME, O_RDWR );
    if ( g_sem_counter == nullptr )
    {
        // semaphore probably not created yet
        log_msg( LOG_ERROR, "Unable to open semaphore. Create new one." );

        // semaphores creation
        g_sem_mutex = sem_open( SEM_MUTEX_NAME, O_RDWR | O_CREAT, 0660, 1 );
        g_sem_counter = sem_open( SEM_COUNTER_NAME, O_RDWR | O_CREAT, 0660, 0 );
        if ( g_sem_counter == nullptr || g_sem_mutex == nullptr )
        {
            log_msg( LOG_ERROR, "Unable to create two semaphores!" );
            exit( EXIT_FAILURE );
        }
        log_msg( LOG_INFO, "Semaphores created." );
    }
    else
    {
        g_sem_mutex = sem_open( SEM_MUTEX_NAME, O_RDWR );
    }

    if ( g_sem_mutex == nullptr || g_sem_counter == nullptr )
    {
        log_msg( LOG_INFO, "Semaphores not available!" );
        exit( EXIT_FAILURE );
    }

    log_msg( LOG_DEBUG, "Increase number of processes." );
    if ( sem_post( g_sem_counter ) < 0 )
    {
        log_msg( LOG_ERROR, "Unable to increase number of semaphores!" );
    }

    struct sigaction l_sa;
    bzero( &l_sa, sizeof( l_sa ) );
    l_sa.sa_handler = catch_sig;
    sigemptyset( &l_sa.sa_mask );
    l_sa.sa_flags = 0;

    // catch sig <CTRL-C>
    sigaction( SIGINT, &l_sa, nullptr );
    // catch SIG_PIPE
    sigaction( SIGPIPE, &l_sa, nullptr );

    // clean at exit
    atexit( clean );

    while ( 1 )
    {
        // try to enter into critical section
        log_msg( LOG_DEBUG, "Try to enter into critical section ..." );
        if ( sem_trywait( g_sem_mutex ) < 0 )
        {
            log_msg( LOG_DEBUG,  "Critical section is occupied now. Wait for it ..." );

            if ( sem_wait( g_sem_mutex ) < 0 )
            {
                log_msg( LOG_ERROR, "Unable to enter into critical section!" );
                exit( EXIT_FAILURE );
            }
        }

        log_msg( LOG_DEBUG, "... process is now in critical section..." );

        // work in critical section
        printf( "CRITICAL SECTION:  " );
        for ( int cnt = 0; cnt < 50; cnt++ )
        {
            printf( "-" );
            fflush( stdout );
            usleep( 20000 );
        }
        printf( "\n" );

        log_msg( LOG_DEBUG, "... leaving critical section ..." );

        // unlock critical section
        if ( sem_post( g_sem_mutex ) < 0 )
        {
            log_msg( LOG_ERROR, "Unable to unlock critical section!" );
            exit( EXIT_FAILURE );
        }

        log_msg( LOG_DEBUG, "Critical section leaved." );

        printf( "Counting      : " );
        for ( int cnt = 0; cnt < 50; cnt++ )
        {
            printf( "+" );
            fflush( stdout );
            usleep( 20000 );
        }
        printf( "\n" );
    } // while

    return 0;

} // main

