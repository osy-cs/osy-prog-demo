// ***************************************************************************
//
// Program example for labs in subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2026
//
// Example of socket server with asynchronous I/O (aio).
// See 'man aio'. 
// Book: Richard Stevens, Advanced Programming in the Unix Environment.
//
// This program is implementation of chat between socket clients using asynchronous I/O.
// Beware: sending data to all connected clients is implemented very ugly! See below. 
//
// The mandatory argument of program is port number for listening.
//
// ***************************************************************************

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <csignal>
#include <cerrno>
#include <vector>
#include <unistd.h>
#include <aio.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define STR_CLOSE   "close"
#define STR_QUIT    "quit"

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
    "  Socket server example.\n"
    "\n"
    "  Use: %s [-h] [-d] port_number\n"
    "\n"
    "    -d  debug mode \n"
    "    -h  this help\n"
    "\n"
    ;

// ***************************************************************************

#define BUFLEN  512                     // buffer size
#define MAXFD   1024                    // limit of file descriptors

// vector of buffers for aiocb structures
std::vector< char[ BUFLEN ] > g_aio_buffers( MAXFD );

// vector of aiocb structures
std::vector< aiocb > g_aio_cbs( MAXFD );


// signal handler
void sig_aio( int t_sig, siginfo_t *t_si, void * )
{
    (void) t_sig;

    // pointer to aiocb from siginfo_t structure, see aiocb init below
    aiocb *lp_cb = ( aiocb * ) t_si->si_value.sival_ptr;

    // get result of aio
    int l_ret = aio_return( lp_cb );

    if ( l_ret < 0 )
    { // error
        log_msg( LOG_ERROR, "Unable to read from socket %d.", lp_cb->aio_fildes );
        close( lp_cb->aio_fildes );
        lp_cb->aio_fildes = -1;
        return;
    }

    if ( l_ret == 0 )
    { // socket closed
        log_msg( LOG_INFO, "Socket %d closed.", lp_cb->aio_fildes );
        close( lp_cb->aio_fildes );
        lp_cb->aio_fildes = -1;
        return;
    }

    if ( l_ret > 0 )
    {
        // pointer to data
        char *lp_buf = ( char * ) lp_cb->aio_buf;

        if ( lp_cb->aio_fildes == STDIN_FILENO )
        { // data from stdin (keyboard)
            if ( strncmp( lp_buf, STR_QUIT, strlen( STR_QUIT ) ) == 0 )
            {
                log_msg( LOG_INFO, "Request to 'quit'." );
                exit( EXIT_SUCCESS );
            }
        }
        // print received data to stdout (terminal)
        write( STDOUT_FILENO, lp_buf , l_ret );

#if 1
        // Reimplement!!!
        // The solution works, but it is very ugly.
        // send data to all connected client in signal handler
        for ( int inx = STDERR_FILENO + 1; inx < MAXFD; inx++ )
        {
            if ( g_aio_cbs[ inx ].aio_fildes != -1 )
            {
                write( g_aio_cbs[ inx ].aio_fildes, lp_buf , l_ret );
            }
        }
#endif
        // register aio request again
        l_ret = aio_read( lp_cb );
        if ( l_ret < 0 )
        {
            log_msg( LOG_ERROR, "aio_read for file descriptor %d failed!", lp_cb->aio_fildes );
            log_msg( LOG_INFO, "File descriptor %d closed!", lp_cb->aio_fildes );
            close( lp_cb->aio_fildes );
        }
    }
}


// cleanup on exit, registered in main
void clean()
{
    log_msg( LOG_INFO, "Closing all connections." );
    for ( int inx = 1; inx < MAXFD; inx++ )
    {
        if ( g_aio_cbs[ inx ].aio_fildes == -1 )
        {
            close( g_aio_cbs[ inx ].aio_fildes );
        }
    }
}


// well known code from socket server
int main( int t_argc, char **t_argv )
{
    int l_port = 0;

    // parsing arguments
    for ( int inx = 1; inx < t_argc; inx++ )
    {
        if ( strcmp( t_argv[ inx ], "-d" ) == 0 )
        {
            g_debug = LOG_DEBUG;
        }

        if ( strcmp( t_argv[ inx ], "-h" ) == 0 )
        {
            printf( g_help, t_argv[ 0 ] );
            exit( EXIT_SUCCESS );
        }

        if ( *t_argv[ inx ] != '-' )
        {
            if ( l_port == 0 )
            {
                l_port = atoi( t_argv[ inx ] );
            }
        }
    }

    if ( l_port <= 0 )
    {
        log_msg( LOG_INFO, "Bad or missing port number %d!", l_port );
        printf( g_help, t_argv[ 0 ] );
        exit( EXIT_FAILURE );
    }

    log_msg( LOG_INFO, "Server will listen on port: %d.", l_port );

    // socket creation
    int l_sock_listen = socket( AF_INET, SOCK_STREAM, 0 );
    if ( l_sock_listen < 0 )
    {
        log_msg( LOG_ERROR, "Unable to create socket.");
        exit( EXIT_FAILURE );
    }

    in_addr l_addr_any = { INADDR_ANY };
    sockaddr_in l_srv_addr;
    l_srv_addr.sin_family = AF_INET;
    l_srv_addr.sin_port = htons( l_port );
    l_srv_addr.sin_addr = l_addr_any;

    // Enable the port number reusing
    int l_opt = 1;
    if ( setsockopt( l_sock_listen, SOL_SOCKET, SO_REUSEADDR, &l_opt, sizeof( l_opt ) ) < 0 )
    {
        log_msg( LOG_ERROR, "Unable to set socket option!" );
    }

    // assign port number to socket
    if ( bind( l_sock_listen, (const sockaddr * ) &l_srv_addr, sizeof( l_srv_addr ) ) < 0 )
    {
        log_msg( LOG_ERROR, "Bind failed!" );
        close( l_sock_listen );
        exit( EXIT_FAILURE );
    }

    // listening on set port
    if ( listen( l_sock_listen, 1 ) < 0 )
    {
        log_msg( LOG_ERROR, "Unable to listen on given port!" );
        close( l_sock_listen );
        exit( EXIT_FAILURE );
    }

    log_msg( LOG_INFO, "Enter 'quit' to quit server." );

    // aio code
    // cleanup registration
    atexit( clean );

    // register signal action
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset( &sa.sa_mask );
    sa.sa_sigaction = sig_aio;
    sigaction( SIGIO, &sa, nullptr );

    // initialization of all aiocb structures without file descriptor
    // notification method: signal
    for ( int inx = 0; inx < MAXFD; inx++ )
    {
        aiocb &cb = g_aio_cbs[ inx ];

        cb.aio_fildes = -1;
        cb.aio_offset = 0;
        cb.aio_buf = g_aio_buffers[ inx ];
        cb.aio_nbytes = BUFLEN;
        cb.aio_reqprio = 0;
        cb.aio_lio_opcode = LIO_NOP;

        cb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
        cb.aio_sigevent.sigev_signo = SIGIO;
        cb.aio_sigevent.sigev_value.sival_ptr = &cb;
    }

    // register aio request for stdin
    g_aio_cbs[ STDIN_FILENO ].aio_fildes = STDIN_FILENO;
    aio_read( &g_aio_cbs[ STDIN_FILENO ] );

    // go!
    while ( 1 )
    {
        sockaddr_in l_rsa;
        int l_rsa_size = sizeof( l_rsa );
        // new connection
        int l_sock_client = accept( l_sock_listen, ( sockaddr * ) &l_rsa, ( socklen_t * ) &l_rsa_size );
        if ( l_sock_client < 0 )
        {
            log_msg( LOG_ERROR, "Unable to accept new client." );
            close( l_sock_listen );
            exit( EXIT_FAILURE );
        }
        uint l_lsa = sizeof( l_srv_addr );
        // my IP
        getsockname( l_sock_client, ( sockaddr * ) &l_srv_addr, &l_lsa );
        log_msg( LOG_INFO, "My IP: '%s'  port: %d",
                inet_ntoa( l_srv_addr.sin_addr ), ntohs( l_srv_addr.sin_port ) );
        // client IP
        getpeername( l_sock_client, ( sockaddr * ) &l_srv_addr, &l_lsa );
        log_msg( LOG_INFO, "Client IP: '%s'  port: %d",
                inet_ntoa( l_srv_addr.sin_addr ), ntohs( l_srv_addr.sin_port ) );

        // register aio for new client
        g_aio_cbs[ l_sock_client ].aio_fildes = l_sock_client;
        aio_read( &g_aio_cbs[ l_sock_client ] );

    } // while ( 1 )

    return 0;
} // main

