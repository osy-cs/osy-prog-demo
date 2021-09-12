//***************************************************************************
//
// Program example for labs Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2021
//
// Posix message queue example.
//
// The first process creates message queue and it start to work as producer.
// All others processes will connect to message queue and they will consume
// data from queue.
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
#include <sys/stat.h>
#include <sys/time.h>
#include <mqueue.h>

#define MQ_NAME                 "/mq_example"

// message queue fd
mqd_t g_glb_msg_fd = -1;

// first process?
int g_glb_first = 0;

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

    if ( g_glb_msg_fd < 0 ) return;

    if ( g_glb_first )
    {
        log_msg( LOG_INFO, "This process was first and now it will try remove queue ..." );

        if ( mq_unlink( MQ_NAME ) < 0 )
            log_msg( LOG_ERROR, "Unable to delete message queue!" );
        else
            log_msg( LOG_INFO, "Message queue deleted." );
    }
}

// catch signals
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
            "  Message queue example.\n"
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
        log_msg( LOG_INFO, "Clean message queue." );
        mq_unlink( MQ_NAME );
        exit( 0 );
    }
}


//***************************************************************************

void producer()
{
    static int l_count = 0;

    printf( "Produced %d\r", ++l_count );
    fflush( stdout );

    int l_data = rand() % 1000;

    log_msg( LOG_DEBUG, "Producer will send message with timeout ..." );

    timeval l_curtime;
    gettimeofday( &l_curtime, nullptr );
    timespec l_tout = { l_curtime.tv_sec + 1, l_curtime.tv_usec * 1000 };

    int l_ret = mq_timedsend( g_glb_msg_fd, ( const char * ) &l_data, sizeof( l_data ), 0, &l_tout );
    if ( l_ret < 0 )
    {
        if ( errno == ETIMEDOUT )
        {
            printf( "\n" );
            log_msg( LOG_INFO, "Message queue is full. Send without timeout ..." );
            l_ret = mq_send( g_glb_msg_fd, ( const char * ) &l_data, sizeof( l_data ), 0 );
        }
        else
        {
            log_msg( LOG_ERROR, "Unable to send message!" );
            exit( 1 );
        }
    }
}


void consumer()
{
    static int l_count = 0;

    int l_data;

    log_msg( LOG_DEBUG, "Receive message with timeout ..." );

    timeval l_curtime;
    gettimeofday( &l_curtime, nullptr );
    timespec l_tout = { l_curtime.tv_sec + 1, l_curtime.tv_usec * 1000 };

    int l_ret = mq_timedreceive( g_glb_msg_fd, ( char * ) &l_data, sizeof( l_data ), nullptr, &l_tout );

    if ( l_ret < 0 )
    {
        if ( errno == ETIMEDOUT )
        {
            printf( "\n" );
            log_msg( LOG_INFO, "No message. Wait for message without timeout ..." );
            usleep( 500000 );
            l_ret = mq_receive( g_glb_msg_fd, ( char * ) &l_data, sizeof( l_data ), nullptr );
        }
        else
        {
            log_msg( LOG_ERROR, "Unable to receive message!" );
            exit( 1 );
        }
    }

    printf( "Consumed %d messages\r", ++l_count );
    fflush( stdout );
}

int main( int t_narg, char **t_args )
{
    help( t_narg, t_args );

    // open message queue
    g_glb_msg_fd = mq_open( MQ_NAME, O_RDWR );

    if ( g_glb_msg_fd < 0 )
    {
        // message queue probably not created yet
        log_msg( LOG_ERROR, "Unable to open message queue. Create new one...." );

        mq_attr l_mqa;
        bzero( &l_mqa, sizeof( l_mqa ) );
        l_mqa.mq_flags =
        l_mqa.mq_maxmsg = 8;
        l_mqa.mq_msgsize = sizeof( int );

        // message queue creation
        g_glb_msg_fd = mq_open( MQ_NAME, O_RDWR | O_CREAT, 0660, &l_mqa );
        if ( g_glb_msg_fd < 0 )
        {
            log_msg( LOG_ERROR, "Unable to create message queue!" );
            return 1;
        }

        // this process created mq and it will delete it at exit.
        g_glb_first = 1;
        log_msg( LOG_INFO, "This process created message queue and it will work as 'producer'." );
        log_msg( LOG_INFO, "The message queue will be deleted at exit." );
    }

    log_msg( LOG_DEBUG, "FD of message queue is %d", g_glb_msg_fd );

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
        if ( g_glb_first )
            producer();
        else
            consumer();
    }

    return 0;
}

