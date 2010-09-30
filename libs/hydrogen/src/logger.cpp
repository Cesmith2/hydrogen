/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "hydrogen/config.h"
#include "hydrogen/logger.h"

#include <iostream>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

unsigned Logger::__log_level = 0;
Logger* Logger::__instance=0;
const char* Logger::__levels[] = { "None", "Error", "Warning", "Info", "Debug" };

pthread_t loggerThread;

void* loggerThread_func( void* param )
{
	if ( param == NULL ) {
		// ??????
		return NULL;
	}

#ifdef WIN32
	::AllocConsole();
//	::SetConsoleTitle( "Hydrogen debug log" );
	freopen( "CONOUT$", "wt", stdout );
#endif

	Logger *pLogger = ( Logger* )param;

	FILE *pLogFile = NULL;
	if ( pLogger->__use_file ) {
#ifdef Q_OS_MACX
		QString sLogFilename = QDir::homePath().append( "/Library/Hydrogen/hydrogen.log" );
#else
		QString sLogFilename = QDir::homePath().append( "/.hydrogen/hydrogen.log" );
#endif

		pLogFile = fopen( sLogFilename.toLocal8Bit(), "w" );
		if ( pLogFile == NULL ) {
			std::cerr << "Error: can't open log file for writing..." << std::endl;
		} else {
			fprintf( pLogFile, "Start logger" );
		}
	}

	while ( pLogger->__running ) {
#ifdef WIN32
		Sleep( 1000 );
#else
		usleep( 999999 );
#endif

		Logger::queue_t& queue = pLogger->__msg_queue;
		Logger::queue_t::iterator it, last;
		QString tmpString;
		for( it = last = queue.begin() ; it != queue.end() ; ++it ) {
			last = it;
			printf( "%s", it->toLocal8Bit().data() );
			if( pLogFile ) {
				fprintf( pLogFile, "%s", it->toLocal8Bit().data() );
				fflush( pLogFile );
			}
		}
		// See Object.h for documentation on __mutex and when
		// it should be locked.
		queue.erase( queue.begin(), last );
		pthread_mutex_lock( &pLogger->__mutex );
		if( ! queue.empty() ) queue.pop_front();
		pthread_mutex_unlock( &pLogger->__mutex );
	}

	if ( pLogFile ) {
		fprintf( pLogFile, "Stop logger" );
		fclose( pLogFile );
	}
#ifdef WIN32
	::FreeConsole();
#endif


#ifdef WIN32
	Sleep( 1000 );
#else
	usleep( 100000 );
#endif

	pthread_exit( NULL );
	return NULL;
}

void Logger::create_instance()
{
	if ( __instance == 0 ) {
		__instance = new Logger;
	}
}

/**
 * Constructor
 */
Logger::Logger()
		: __use_file( false )
		, __running( true )
{
	__instance = this;
	pthread_attr_t attr;
	pthread_attr_init( &attr );
	pthread_mutex_init( &__mutex, NULL );
	pthread_create( &loggerThread, &attr, loggerThread_func, this );
}

/**
 * Destructor
 */
Logger::~Logger()
{
	__running = false;
	pthread_join( loggerThread, NULL );

}

void Logger::log( unsigned level,
		  const char* funcname,
		  const QString& class_name,
		  const QString& msg )
{
	if( level == None ) return;

	const char* prefix[] = { "", "(E) ", "(W) ", "(I) ", "(D) " };
#ifdef WIN32
	const char* color[] = { "", "", "", "", "" };
#else
	const char* color[] = { "", "\033[31m", "\033[36m", "\033[32m", "\033[35m" };
#endif // WIN32

	int i;
	switch(level) {
	case None:
		assert(false);
		i = 0;
		break;
	case Error:
		i = 1;
		break;
	case Warning:
		i = 2;
		break;
	case Info:
		i = 3;
		break;
	case Debug:
		i = 4;
		break;
	default:
		i = 0;
		break;
	}

	QString tmp = QString("%1%2%3\t%4 %5 \033[0m\n")
		.arg(color[i])
		.arg(prefix[i])
		.arg(class_name)
		.arg(funcname)
		.arg(msg);

	pthread_mutex_lock( &__mutex);
	__msg_queue.push_back( tmp );
	pthread_mutex_unlock( &__mutex );
}

unsigned Logger::parse_log_level(const char* level)
{
	unsigned log_level;
	if( 0 == strncasecmp( level, __levels[0], sizeof(__levels[0]) ) ) {
		log_level = Logger::None;
	} else if ( 0 == strncasecmp( level, __levels[1], sizeof(__levels[1]) ) ) {
		log_level = Logger::Error;
	} else if ( 0 == strncasecmp( level, __levels[2], sizeof(__levels[2]) ) ) {
		log_level = Logger::Error | Logger::Warning;
	} else if ( 0 == strncasecmp( level, __levels[3], sizeof(__levels[3]) ) ) {
		log_level = Logger::Error | Logger::Warning | Logger::Info;
	} else if ( 0 == strncasecmp( level, __levels[4], sizeof(__levels[4]) ) ) {
		log_level = Logger::Error | Logger::Warning | Logger::Info | Logger::Debug;
	} else {
#ifdef HAVE_SSCANF
		int val = sscanf(level,"%x",&log_level);
		if( val != 1 ) {
			log_level = Logger::Error;
	    }
#else
        int log_level = hextoi( level, -1 );
        if( log_level==-1 ) {
			log_level = Logger::Error;
        }
#endif
	}
    return log_level;
}

#ifndef HAVE_SSCANF
int Logger::hextoi(const char* str, long len)
{
    long pos = 0;
    char c = 0;
    int v = 0;
    int res = 0;
    bool leading_zero = false;

    while(1) {
        if((len!=-1) && (pos>=len) ) {
            break;
        }
        c = str[pos];
        if(c==0) {
            break;
        } else if(c=='x' || c=='X') {
            if ( (pos==1) && leading_zero ) {
                assert( res == 0 );
                pos++;
                continue;
            } else {
                return -1;
            }
        } else if( c>='a' ) {
            v = c-'a'+10;
        } else if( c>='A' ) {
            v = c-'A'+10;
        } else if( c>='0' ) {
            if ( (c=='0') && (pos==0) ) {
                leading_zero = true;
            }
            v = c-'0';
        } else {
            return -1;
        }
        if(v>15) { return -1; }
        //assert( v == (v & 0xF) );
        res = (res << 4) | v;
        assert( (res & 0xF) == (v & 0xF) );
        pos++;
    }
    return res;
}
#endif // HAVE_SSCANF
