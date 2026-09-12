// ***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2026
//
// Example of socket server/client.
//
// This program is example of socket client.
// The mandatory arguments of program is IP address or name of server and
// a port number.
//
// ***************************************************************************

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cerrno>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define STR_CLOSE               "close"

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
    "  Socket client example.\n"
    "\n"
    "  Use: %s [-h] [-d] ip_or_name port_number\n"
    "\n"
    "    -d  debug mode \n"
    "    -h  this help\n"
    "\n"
    ;

// ***************************************************************************

int main( int t_argc, char **t_argv )
{
    int l_port = 0;
    char *l_host = nullptr;

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
            if ( l_host == nullptr  )
            {
                l_host = t_argv[ inx ];
            }
            else if ( l_port == 0 )
            {
                l_port = atoi( t_argv[ inx ] );
            }
        }
    }

    if ( l_host == nullptr || l_port <= 0 )
    {
        log_msg( LOG_INFO, "Host or port is missing!" );
        printf( g_help, t_argv[ 0 ] );
        exit( EXIT_FAILURE );
    }

    log_msg( LOG_INFO, "Connection to '%s':%d.", l_host, l_port );

    addrinfo l_ai_req, *l_ai_ans;
    bzero( &l_ai_req, sizeof( l_ai_req ) );
    l_ai_req.ai_family = AF_INET;
    l_ai_req.ai_socktype = SOCK_STREAM;

    int l_get_ai = getaddrinfo( l_host, nullptr, &l_ai_req, &l_ai_ans );
    if ( l_get_ai )
    {
        log_msg( LOG_ERROR, "Unknown host name!" );
        exit( EXIT_FAILURE );
    }

    // get the first address
    sockaddr_in l_cl_addr =  *( sockaddr_in * ) l_ai_ans->ai_addr;
    l_cl_addr.sin_port = htons( l_port );
    freeaddrinfo( l_ai_ans );

    // socket creation
    int l_sock_server = socket( AF_INET, SOCK_STREAM, 0 );
    if ( l_sock_server < 0 )
    {
        log_msg( LOG_ERROR, "Unable to create socket.");
        exit( EXIT_FAILURE );
    }

    // connect to server
    if ( connect( l_sock_server, ( sockaddr * ) &l_cl_addr, sizeof( l_cl_addr ) ) < 0 )
    {
        log_msg( LOG_ERROR, "Unable to connect server." );
        exit( EXIT_FAILURE );
    }

    uint l_lsa = sizeof( l_cl_addr );
    // my IP
    getsockname( l_sock_server, ( sockaddr * ) &l_cl_addr, &l_lsa );
    log_msg( LOG_INFO, "My IP: '%s'  port: %d",
             inet_ntoa( l_cl_addr.sin_addr ), ntohs( l_cl_addr.sin_port ) );
    // server IP
    getpeername( l_sock_server, ( sockaddr * ) &l_cl_addr, &l_lsa );
    log_msg( LOG_INFO, "Server IP: '%s'  port: %d",
             inet_ntoa( l_cl_addr.sin_addr ), ntohs( l_cl_addr.sin_port ) );

    log_msg( LOG_INFO, "Enter 'close' to close application." );

    // list of fd sources
    pollfd l_read_poll[ 2 ];

    l_read_poll[ 0 ].fd = STDIN_FILENO;
    l_read_poll[ 0 ].events = POLLIN;
    l_read_poll[ 1 ].fd = l_sock_server;
    l_read_poll[ 1 ].events = POLLIN;

    // go!
    while ( 1 )
    {
        char l_buf[ 128 ];

        // select from fds
        if ( poll( l_read_poll, 2, -1 ) < 0 ) break;

        // data on stdin?
        if ( l_read_poll[ 0 ].revents & POLLIN )
        {
            //  read from stdin
            int l_len = read( STDIN_FILENO, l_buf, sizeof( l_buf ) );
            if ( l_len == 0 )
            {
                log_msg( LOG_DEBUG, "Stdin closed." );
                break;
            }
            if ( l_len < 0 )
            {
                log_msg( LOG_ERROR, "Unable to read from stdin." );
                break;
            }
            else
            {
                log_msg( LOG_DEBUG, "Read %d bytes from stdin.", l_len );
            }

            // send data to server
            l_len = write( l_sock_server, l_buf, l_len );
            if ( l_len < 0 )
            {
                log_msg( LOG_ERROR, "Unable to send data to server." );
                break;
            }
            else
            {
                log_msg( LOG_DEBUG, "Sent %d bytes to server.", l_len );
            }
        }

        // data from server?
        if ( l_read_poll[ 1 ].revents & POLLIN )
        {
            // read data from server
            int l_len = read( l_sock_server, l_buf, sizeof( l_buf ) );
            if ( l_len == 0 )
            {
                log_msg( LOG_DEBUG, "Server closed socket." );
                break;
            }
            else if ( l_len < 0 )
            {
                log_msg( LOG_ERROR, "Unable to read data from server." );
                break;
            }
            else
            {
                log_msg( LOG_DEBUG, "Read %d bytes from server.", l_len );
            }

            // display on stdout
            l_len = write( STDOUT_FILENO, l_buf, l_len );
            if ( l_len < 0 )
            {
                log_msg( LOG_ERROR, "Unable to write to stdout." );
                break;
            }

            // request to close?
            if ( strncasecmp( l_buf, STR_CLOSE, strlen( STR_CLOSE ) ) == 0 )
            {
                log_msg( LOG_INFO, "Connection will be closed..." );
                break;
            }
        }
    }

    // close socket
    close( l_sock_server );

    return 0;
} // main

