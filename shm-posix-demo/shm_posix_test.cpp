// ***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2026
//
// Example of shared memory.
// Every process increments global counter.
// Number of process is counted in shared memory.
//
// ***************************************************************************

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cerrno>
#include <csignal>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

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
    "  Share memory example.\n"
    "\n"
    "  Use: %s [-d -h -r]\n"
    "\n"
    "    -h  this help\n"
    "    -d  debug mode \n"
    "    -r  clean shared memory \n"
    "\n"
    ;

// ***************************************************************************

#define SHM_NAME        "/shm_example"

// data structure for shared memory
struct shm_data_t
{
    int num_of_process;
    int counter;
};

// pointer to shared memory
shm_data_t *g_glb_data = nullptr;

// ***************************************************************************
// cleaning function

void clean( void )
{
    log_msg( LOG_INFO, "Final cleaning ..." );

    if ( g_glb_data == nullptr ) 
    {
        return;
    }

    int l_num_proc = -1;

    if ( g_glb_data != nullptr )
    {
        g_glb_data->num_of_process--;
        l_num_proc = g_glb_data->num_of_process;
    }

    log_msg( LOG_INFO, "Shared memory releasing..." );
    int l_ret = munmap( g_glb_data, sizeof( *g_glb_data ) );
    if ( l_ret < 0 )
    {
        log_msg( LOG_ERROR, "Unable to release shared memory!" );
    }
    else
    {
        log_msg( LOG_INFO, "Share memory released." );
    }

    if ( l_num_proc == 0  )
    {
        log_msg( LOG_INFO, "This process is last (pid %d).", getpid() );
        if ( shm_unlink( SHM_NAME ) < 0 )
        {
            log_msg( LOG_ERROR, "Unable to remove shared memory!" );
        }
        else
        {
            log_msg( LOG_INFO, "Shared memory removed." );
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
    // parse arguments
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
            if ( shm_unlink( SHM_NAME ) < 0 )
            {
                log_msg( LOG_ERROR, "Unable to remove share memory!" );
                exit( EXIT_FAILURE );
            }

            log_msg( LOG_INFO, "Shared memory cleaned." );
            exit( EXIT_SUCCESS );
        }
    }

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
        ftruncate( l_fd, sizeof( shm_data_t ) );
        log_msg( LOG_INFO, "File created, this process is the first." );
        l_first = 1;
    }

    // share memory allocation
    g_glb_data = ( shm_data_t * ) mmap( nullptr, 
                                      sizeof( shm_data_t ), 
                                      PROT_READ | PROT_WRITE,
                                      MAP_SHARED, 
                                      l_fd, 
                                      0 );

    if ( g_glb_data == nullptr )
    {
        log_msg( LOG_ERROR, "Unable to attach shared memory!" );
        exit( EXIT_FAILURE );
    }
    else
    {
        log_msg( LOG_INFO, "Shared memory attached.");
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

    // first process initialize shared memory
    if ( l_first != 0 )
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
        {
            log_msg( LOG_INFO, "Another process changed global counter. Difference=%d", g_glb_data->counter - l_old );
        }

        l_old = ++g_glb_data->counter;

        printf( "New value of global counter %d\r", g_glb_data->counter );
        fflush( stdout );

        if ( ( g_glb_data->counter % 100 ) == 0 ) 
        {
            usleep( 250000 );
        }
    }

    return 0;
} // main

