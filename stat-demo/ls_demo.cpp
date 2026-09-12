//***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2026
//
// The example of using stat* functions. 
//
// ***************************************************************************

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cstdarg>
#include <vector>
#include <sys/param.h>
#include <sys/stat.h>
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
    "  Fork() and pipe() example.\n"
    "\n"
    "  Use: %s [-h] [-d] [-n] [-s]\n"
    "\n"
    "    -h  this help\n"
    "    -d  debug mode \n"
    "    -n  sort output by file name \n"
    "    -s  sort output by file size \n"
    "\n"
    ;

// ***************************************************************************

struct file_info_t 
{
    char m_file_name[ MAXPATHLEN ];
    struct stat m_file_stat;
};

// compare for quick sort 
int cmp_file_names( const void *tp_f1, const void *tp_f2 )
{
    file_info_t *lp_f1 = ( file_info_t * ) tp_f1;
    file_info_t *lp_f2 = ( file_info_t * ) tp_f2;

    return strcmp( lp_f1->m_file_name, lp_f2->m_file_name );
}

// compare for quick sort 
int cmp_file_sizes( const void *tp_f1, const void *tp_f2 )
{
    file_info_t *lp_f1 = ( file_info_t * ) tp_f1;
    file_info_t *lp_f2 = ( file_info_t * ) tp_f2;

    if ( lp_f1->m_file_stat.st_size < lp_f2->m_file_stat.st_size ) 
    {
        return -1;
    }
    if ( lp_f1->m_file_stat.st_size > lp_f2->m_file_stat.st_size ) 
    {
        return 1;
    }
    return 0;
}


int main( int t_argc, char **t_argv )
{
    std::vector< file_info_t > l_file_list;

    int l_sort_names = 0;
    int l_sort_size = 0;


    for ( int inx = 1; inx < t_argc; inx++ )
    {
        if ( strcmp( t_argv[ inx ], "-h" ) == 0 )
        {
            printf( g_help, t_argv[ 0 ] );
            exit( EXIT_SUCCESS );
        }
        if ( strcmp( t_argv[ inx ], "-d" ) == 0 )
        {
            g_debug = LOG_DEBUG;
        }
        if ( strcmp( t_argv[ inx ], "-n" ) == 0 )
        {
            l_sort_names = 1;
        }
        if ( strcmp( t_argv[ inx ], "-s" ) == 0 )
        {
            l_sort_size = 1;
        }

        if ( *t_argv[ inx ] != '-' )
        {
            // add next file name into l_file_list
            file_info_t l_finfo;
            memset( &l_finfo, 0, sizeof( l_finfo ) );
            strcpy( l_finfo.m_file_name, t_argv[ inx ] );
            l_file_list.push_back( l_finfo );
        }
    }

    // file_list empty?
    if ( l_file_list.size() == 0 )
    {
        log_msg( LOG_INFO, "No file specified!" );
        exit( EXIT_FAILURE );
    }

    // get stat info for all file names
    for ( auto finfo = l_file_list.begin(); finfo != l_file_list.end(); )
    {
        if ( stat( finfo->m_file_name, &finfo->m_file_stat ) < 0 )
        {
            log_msg( LOG_ERROR, "File '%s' not found.", finfo->m_file_name );
            finfo = l_file_list.erase( finfo );
        }
        else
        {
            finfo++;
        }
    }

    // sort by name?
    if ( l_sort_names != 0 )
    {
        qsort( l_file_list.data(), l_file_list.size(), sizeof( file_info_t ), cmp_file_names );
    }

    // sort by size?
    if ( l_sort_size != 0 )
    {
        qsort( l_file_list.data(), l_file_list.size(), sizeof( file_info_t ), cmp_file_sizes );
    }

    // output
    for ( size_t inx = 0; inx < l_file_list.size(); inx++ )
    {
        auto &finfo = l_file_list[ inx ];
        printf( "%16lu  %s\n", finfo.m_file_stat.st_size, finfo.m_file_name );
    }
     
    return EXIT_SUCCESS;
} // main

