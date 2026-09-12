// ***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2026
//
// Example of using exec..() function.
// The current process will ask user for command (program) and
// this program will replace current program in process.
//
// ***************************************************************************

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cstdarg>
#include <vector>
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
    "  Example of using exec..() function. Two parsing methods are implemented.\n"
    "\n"
    "  Use: %s [-h] [-d] strsep|strpbrk\n"
    "\n"
    "    -h  this help\n"
    "    -d  debug mode \n"
    "\n"
    ;

// ***************************************************************************

#define PARSING_STRSEP  "strsep"
#define PARSING_STRPBRK "strpbrk"

#define CMD_SEP         " \t\n"

int main( int t_argc, char **t_argv )
{
    char *l_parsing_mode = nullptr;

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
            if ( l_parsing_mode == nullptr )
            {
                l_parsing_mode = t_argv[ inx ];
            }
        }
    }

    if ( l_parsing_mode == nullptr ||
         ( strcmp( l_parsing_mode, PARSING_STRSEP ) != 0 &&
           strcmp( l_parsing_mode, PARSING_STRPBRK ) != 0 ) )
    {
        log_msg( LOG_INFO, "Select the correct parsing method." );
        printf( g_help, t_argv[ 0 ] );
        exit( EXIT_FAILURE );
    }

    char **l_argv_exec = nullptr;
    char *l_arg_man[ 1024 ];
    std::vector< char * >l_arg_vec;

    char l_line[ 1024 ];

    printf( "Enter command > " );
    fgets( l_line, sizeof( l_line ), stdin );

    if ( strcmp( l_parsing_mode, PARSING_STRSEP ) == 0 )
    { // parsing with strsep()
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
    }
    else
    { // manual parsing with strpbrk()
        char *l_start = l_line;
        int l_arg_num = 0;

        while ( l_start != nullptr )
        {
            // find next separator
            char *l_tok = strpbrk( l_start, CMD_SEP );
            if ( l_tok != nullptr )
            {
                *l_tok = '\0';
                l_tok++;
            }
            if ( strlen( l_start ) > 0 )
            {
                l_arg_man[ l_arg_num++ ] = l_start;
            }
            l_start = l_tok;
        }

        if ( l_arg_num > 0 )
        { // non-empty result
            l_arg_man[ l_arg_num ] = nullptr;
            l_argv_exec = l_arg_man;
        }
    }

    if ( l_argv_exec == nullptr )
    {
        log_msg( LOG_INFO, "No command to execute!" );
    }

    int l_ret = execvp( l_argv_exec[ 0 ], l_argv_exec );

    // current program continues only if exec failed
    if ( l_ret < 0 )
    {
        log_msg( LOG_ERROR, "Unable to execute command!" );
    }

    // process exit with status 1
    exit( EXIT_FAILURE );

} // main


