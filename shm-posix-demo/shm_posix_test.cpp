//***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2021
//
// Example of shared memory.
// Every process increments global counter.
// Number of process is counted in shared memory.
//
//***************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#define SHM_NAME        "/shm_example"

// data structure for shared memory
struct shm_data
{
  int num_of_process;
  int counter;
};

// pointer to shared memory
shm_data *g_glb_data = nullptr;

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
// cleaning function

void clean( void )
{
    log_msg( LOG_INFO, "Final cleaning ..." );

    if ( !g_glb_data ) return;

    int l_num_proc = -1;

    if ( g_glb_data != nullptr )
    {
        g_glb_data->num_of_process--;
        l_num_proc = g_glb_data->num_of_process;
    }

    log_msg( LOG_INFO, "Shared memory releasing..." );
    int l_ret = munmap( g_glb_data, sizeof( *g_glb_data ) );
    if ( l_ret )
        log_msg( LOG_ERROR, "Unable to release shared memory!" );
    else
        log_msg( LOG_INFO, "Share memory released." );

    if ( l_num_proc == 0  )
    {
        log_msg( LOG_INFO, "This process is last (pid %d).", getpid() );
        shm_unlink( SHM_NAME );
    }
}

// catch signal
void catch_sig( int sig )
{
  exit( 1 );
}

//***************************************************************************
// help

void help( int t_narg, char **t_args )
{
    if ( t_narg <= 1 ) return;

    if ( !strcmp( t_args[ 1 ], "-h" ) )
    {
        printf(
            "\n"
            "  Share memory use example.\n"
            "\n"
            "  Use: %s [-d -h -r]\n"
            "\n"
            "    -h  this help\n"
            "    -d  debug mode \n"
            "    -r  clean shared memory \n"
             "\n", t_args[ 0 ] );

        exit( 0 );
    }

    if ( !strcmp( t_args[ 1 ], "-d" ) )
        g_debug = LOG_DEBUG;

    if ( !strcmp( t_args[ 1 ], "-r" ) )
    {
        shm_unlink( SHM_NAME );
        log_msg( LOG_INFO, "Shared memory cleaned." );
    }
}

//***************************************************************************

int main( int t_narg, char **t_args )
{
    help( t_narg, t_args );

    int l_first = 0;

    int l_fd = shm_open( SHM_NAME, O_RDWR, 0660 );
    if ( l_fd < 0 )
    {
        log_msg( LOG_ERROR, "Unable to open file for shared memory." );
        l_fd = shm_open( SHM_NAME, O_RDWR | O_CREAT, 0660 );
        if ( l_fd < 0 )
        {
            log_msg( LOG_ERROR, "Unable to create file for shared memory." );
            exit( 1 );
        }
        ftruncate( l_fd, sizeof( shm_data ) );
        log_msg( LOG_INFO, "File created, this process is first" );
        l_first = 1;
    }

    // share memory allocation
    g_glb_data = ( shm_data * ) mmap( nullptr, sizeof( shm_data ), PROT_READ | PROT_WRITE,
            MAP_SHARED, l_fd, 0 );

    if ( !g_glb_data )
    {
        log_msg( LOG_ERROR, "Unable to attach shared memory!" );
        exit( 1 );
    }
    else
        log_msg( LOG_INFO, "Shared memory attached.");

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

    // first process initialize shared memory
    if ( l_first )
    {
        g_glb_data->counter = 0;
        g_glb_data->num_of_process = 0;
    }

    // increase number of processes
    g_glb_data->num_of_process++;

    int l_old = g_glb_data->counter;
    log_msg( LOG_DEBUG, "Current global counter is %d.", g_glb_data->counter );

    while ( 1 )
    {
        if ( l_old != g_glb_data->counter )
            log_msg( LOG_INFO, "Another process changed global counter. Difference=%d", g_glb_data->counter - l_old );

        l_old = ++g_glb_data->counter;

        printf( "New value of global counter %d\r", g_glb_data->counter );
        fflush( stdout );

        if ( !( g_glb_data->counter % 100 ) ) usleep( 250000 );
    }

    return 0;
}

