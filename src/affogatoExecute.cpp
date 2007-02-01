/** Abstract data container base class.
 *
 *  @file
 *
 *  @par License:
 *  Copyright (C) 2006 Rising Sun Pictures Pty. Ltd.
 *  @par
 *  This plugin is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later
 *  version.
 *  @par
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  @par
 *  You should have received a copy of the GNU Lesser General Public
 *  License (http://www.gnu.org/licenses/lgpl.txt) along with this
 *  library; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  @author Moritz Moeller (moritz.moeller@rsp.com.au)
 *
 *  @par Disclaimer:
 *  Rising Sun Pictures Pty. Ltd., hereby disclaims all copyright
 *  interest in the plugin 'Affogato' (a plugin to translate 3D
 *  scenes to a 3D renderer) written by Moritz Moeller.
 *  @par
 *  Any one who uses this code does so completely at their own risk.
 *  Rising Sun Pictures doesn't warrant that this code does anything
 *  at all but if it does something and you don't like it, then we
 *  are not responsible.
 *  @par
 *  Have a nice day!
 */


// Standard headers
#include <string>

// XSI Headers
#include <xsi_application.h>

// Affogato headers
#include "affogatoExecute.hpp"
#include "affogatoHelpers.hpp"


namespace affogato {
	using namespace std;

	/** Linux implementation of execute()
	 */
#ifdef unix

	// Standard headers
	#include <sys/types.h>
	#include <unistd.h>
	#include <stdlib.h>

	bool execute( const string &command, const string &arguments, const string &path, const bool wait ) {
		Application app;
		chdir( path.c_str() );

		string cmd( command + " " + arguments + ( wait || !app.IsInteractive() ? "" : "&" ) );
		debugMessage( stringToCString( "Launching: '" + cmd + "'" + ( "" == path ? "from '" + path : "'" ) ) );
		int returnCode = system( cmd.c_str() );
		return( returnCode != -1 );
	}
#endif // LINUX


	/** Win32 implementation of execute()
	 */
#ifdef _WIN32

	// Windows headers
	#include <windows.h>

	bool execute( const string &command, const string &arguments, const string &path, const bool wait )	{
		Application app;
		message( stringToCString( "Launching: '" + command + " " + arguments + ( !path.empty() ? "' from '" + path + "'" : "'" ) ), messageInfo );
		if ( !wait && app.IsInteractive() ) {
			ShellExecute( NULL, NULL, ( LPCTSTR ) command.c_str(), ( LPCTSTR ) arguments.c_str(), ( LPCTSTR ) path.c_str(), SW_SHOWNORMAL );
			return true;
		} else {
			PROCESS_INFORMATION pinfo;
			STARTUPINFO sinfo;
			HANDLE hErrReadPipe, hErrReadPipeDup;
			HANDLE hErrWritePipe;
			int ret;

			ZeroMemory( &pinfo, sizeof( PROCESS_INFORMATION) );
			ZeroMemory( &sinfo, sizeof( STARTUPINFO) );

			sinfo.cb = sizeof( STARTUPINFO );

			sinfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
			sinfo.wShowWindow = SW_HIDE; // ; SW_SHOWNORMAL

			SECURITY_ATTRIBUTES saAttr;

			saAttr.nLength = sizeof( SECURITY_ATTRIBUTES );
			saAttr.bInheritHandle = true;
			saAttr.lpSecurityDescriptor = NULL;

			HANDLE hSaveStdin = GetStdHandle( STD_INPUT_HANDLE );
			HANDLE hSaveStdout = GetStdHandle( STD_OUTPUT_HANDLE );
			HANDLE hSaveStderr = GetStdHandle( STD_ERROR_HANDLE );
			ret = SetStdHandle( STD_ERROR_HANDLE, hSaveStdout );

			ret = CreatePipe( &hErrReadPipe, &hErrWritePipe, &saAttr, 0L );
			ret = SetStdHandle( STD_ERROR_HANDLE, hErrWritePipe );

			ret = DuplicateHandle(	GetCurrentProcess(), hErrReadPipe,
						GetCurrentProcess(), &hErrReadPipeDup , 0,
						false,
						DUPLICATE_SAME_ACCESS );

			  CloseHandle( hErrReadPipe );

			sinfo.hStdInput = GetStdHandle( STD_INPUT_HANDLE );
			sinfo.hStdError = GetStdHandle( STD_ERROR_HANDLE );
			sinfo.hStdOutput = GetStdHandle( STD_ERROR_HANDLE );

			string cmdline = command + " " + arguments;
			ret = CreateProcess(
					NULL,                     // name of executable module
					(char *)cmdline.c_str(),  // command line string
					NULL,                     // SD lpProcessAttributes
					NULL,                     // SD lpThreadAttributes
					true,                     // handle inheritance option
					CREATE_NO_WINDOW,         // CREATE_NEW_CONSOLE, //      // creation flags
					NULL,                     // new environment block
					NULL,                     // current directory name
					&sinfo,                   // startup information
					&pinfo                    // process information
				  );
			if ( ret ) {

				SetStdHandle( STD_ERROR_HANDLE, hSaveStderr ); // restore saved Stderr
				SetStdHandle( STD_OUTPUT_HANDLE, hSaveStdout ); // restore saved Stderr

				// Wait until child process exits.
				WaitForSingleObject( pinfo.hProcess, INFINITE );

				// Close process and thread handles.
				CloseHandle( pinfo.hProcess );
				CloseHandle( pinfo.hThread );

				CloseHandle( hErrWritePipe );
				for ( ;; ) {
					char buff[ 4096 ];
					DWORD readed, written;
					if( !ReadFile( hErrReadPipeDup, buff, 4096, &readed, NULL) || readed == 0 )
					  break;
					if( !WriteFile( hSaveStdout, buff, readed, &written, NULL) )
					  break;
				}
				fflush( stdout );
				}

			return ( ret )? true : false ;
		}
	}
#endif // _WIN32
}
