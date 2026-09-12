// ***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2026
//
// Example of using exec..() function.
// The current process will ask user for command (program) and
// this program will be executed in child process.
//
// ***************************************************************************

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cstdarg>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

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
    "  Example of using exec..() function.\n"
    "\n"
    "  Use: %s [-h] [-d]\n"
    "\n"
    "    -h  this help\n"
    "    -d  debug mode \n"
    "\n"
    ;

// ***************************************************************************

#define CMD_SEP         " \t\n"

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
    }

    while ( 1 )
    {
        char **l_argv_exec = nullptr;
        std::vector< char * >l_arg_vec;

        char l_line[ 1024 ];

        printf( "Enter command > " );
        fgets( l_line, sizeof( l_line ), stdin );

        char *l_start = l_line;

        while ( l_start != nullptr )
        {
            // find next separator
            char *l_tok = strsep( &l_start, CMD_SEP );
            if ( l_tok != nullptr && strlen( l_tok ) > 0 )
            {
                l_arg_vec.push_back( l_tok );
            }
        }

        if ( l_arg_vec.size() > 0 )
        { // non-empty result
            l_arg_vec.push_back( nullptr );
            l_argv_exec = l_arg_vec.data();
        }

        if ( l_argv_exec == nullptr )
        {
            log_msg( LOG_INFO, "No command to execute!" );
            continue;
        }

        int l_pid = fork();
        if ( l_pid < 0 )
        {
            log_msg( LOG_ERROR, "Unable to create new process!" );
            continue;
        }

        if ( l_pid == 0 )
        { // child
            // current program in child process will be replaced by new program
            int l_ret = execvp( l_argv_exec[ 0 ], l_argv_exec );

            // current program continues only if exec failed
            if ( l_ret < 0 )
            {
                log_msg( LOG_ERROR, "Unable to execute command!" );
            }

            // process exit with status 1
            exit( EXIT_FAILURE );
        }
        else
        { // parent
            // wait for child
            int l_status;
            waitpid( l_pid, &l_status, 0 );
            log_msg( LOG_INFO, "Status code of child: %d.\n", WEXITSTATUS( l_status ) );
        }
    } // end while
} // main


