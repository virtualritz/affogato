/** Job manager class.
 *
 *  A job describes a dependency graph for commands to be run
 *  to create the final image.
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
#include <fstream>
#include <string>
#include <sstream>

// XSI headers
#include <xsi_application.h>

// affogato headers
#include "affogatoExecute.hpp"
#include "affogatoGlobals.hpp"
#include "affogatoHelpers.hpp"
#include "affogatoJob.hpp"


namespace affogato {

	using namespace std;

	job::job( const string &aTitle, const string &aCommand ) {
		title = aTitle;
		if( "" != aCommand )
			addCommand( aCommand );
	}

	job::job( const job &aJob ) {
		commands.clear();
		cleanUpCommands.clear();
		subJobPtrMap.clear();
		job *jobPtr = const_cast< job* >( &aJob );
		for( map< string, boost::shared_ptr< job > >::iterator it = jobPtr->subJobPtrMap.begin(); it != jobPtr->subJobPtrMap.end(); it++ ) {
			subJobPtrMap[ it->second->title ] = boost::shared_ptr< job >( new job( *( it->second ) ) );
		}
		for( vector< string >::iterator it = jobPtr->commands.begin(); it < jobPtr->commands.end(); it++ ) {
			commands.push_back( *it );
		}
		for( vector< string >::iterator it = jobPtr->cleanUpCommands.begin(); it < jobPtr->cleanUpCommands.end(); it++ ) {
			cleanUpCommands.push_back( *it );
		}
		title = aJob.title;
	}

	void job::merge( const job &aJob ) {
		job *jobPtr = const_cast< job* >( &aJob );
		for( map< string, boost::shared_ptr< job > >::iterator it = jobPtr->subJobPtrMap.begin(); it != jobPtr->subJobPtrMap.end(); it++ ) {
			subJobPtrMap[ it->second->title ] = boost::shared_ptr< job >( new job( *( it->second ) ) );
		}
		for( vector< string >::iterator it = jobPtr->commands.begin(); it < jobPtr->commands.end(); it++ ) {
			commands.push_back( *it );
		}
		for( vector< string >::iterator it = jobPtr->cleanUpCommands.begin(); it < jobPtr->cleanUpCommands.end(); it++ ) {
			cleanUpCommands.push_back( *it );
		}
	}

	void job::setTitle( const string &aTitle ) {
		title = aTitle;
	}

	string job::getTitle() const {
		return title;
	}

	void job::addCommand( const string &aCommand ) {
		commands.push_back( aCommand );
	}

	void job::addCleanUpCommand( const string &aCommand ) {
		cleanUpCommands.push_back( aCommand );
	}

	void job::addSubJob( const job &subJob ) {
		subJobPtrMap[ subJob.title ] = boost::shared_ptr< job >( new job( subJob ) );
	}

	void job::runIt( bool wait, indentHelper indent ) {
		Application app;
		globals& g = const_cast< globals& >( globals::access() );

		if( !commands.empty() ) {
			for( vector< string >::iterator it = commands.begin(); it < commands.end(); it++ ) {
				message( stringToCString( indent.get() ) + L"Running job '" + stringToCString( *it ) + L"'", messageInfo );
				debugMessage( L"Command: " + stringToCString( *it ) );
				//if( "" != g.jobGlobal.hosts )
				//	execute( g.renderer.command, " -ssh -hosts " + g.jobGlobal.hosts + " " + ( *it ) + " 2>/var/tmp/affogatoout.log", g.directories.base, wait );
				//else
#ifndef DEBUG
				size_t splitPos = it->find( ' ' );
#ifdef unix
				execute( it->substr( 0, splitPos ), it->substr( splitPos ) + " 2>/var/tmp/affogatoout.log", g.directories.base.native_directory_string(), wait );

				ifstream logFile( "/var/tmp/affogatoout.log" );
				if( logFile.is_open() ) {
					char *buffer = new char[ 256 ];
					while( !logFile.eof() ) {
						logFile.getline( buffer, 255 );
						if( ( '#' != buffer[ 0 ] ) && ( strlen( buffer ) > 1 )  )
							message( L"  " + charToCString( buffer ), messageError );
					}
					delete[] buffer;
				}
#else
				execute( it->substr( 0, splitPos ), it->substr( splitPos ), g.directories.base.native_directory_string(), wait );
#endif
#endif
			}
		}

		if( !cleanUpCommands.empty() ) {
			for( vector< string >::iterator it = cleanUpCommands.begin(); it < cleanUpCommands.end(); it++ ) {
				message( stringToCString( indent.get() ) + L"  Cleaning up job '" + stringToCString( *it ) + L"'", messageInfo );
				//if( "" != g.jobGlobal.hosts )
				//	execute( g.renderer.command, " -ssh -hosts " + g.jobGlobal.hosts + " " + ( *it ) + " 2>/var/tmp/affogatoout.log", g.directories.base, wait );
				//else
#ifndef DEBUG
				size_t splitPos = it->find( ' ' );
#ifdef unix
				execute( it->substr( 0, splitPos ), it->substr( splitPos ) + " 2>/var/tmp/affogatoout.log", ".", wait );

				ifstream logFile( "/var/tmp/affogatoout.log" );
				if( logFile.is_open() ) {
					char *buffer = new char[ 256 ];
					while( !logFile.eof() ) {
						logFile.getline( buffer, 255 );
						if( ( '#' != buffer[ 0 ] ) && ( strlen( buffer ) > 1 )  )
							message( L"  " + charToCString( buffer ), messageError );
					}
					delete[] buffer;
				}
#else
				execute( it->substr( 0, splitPos ), it->substr( splitPos ), ".", wait );
#endif
#endif
			}
		}
	}

	void job::run( unsigned launchSubLevel, bool wait ) {
		globals& g = const_cast< globals& >( globals::access() );

		static indentHelper indent;

		if( ( launchSubLevel > 0 ) && !subJobPtrMap.empty() ) {
			message( stringToCString( indent.get() ) + L"Running subjobs for '" + stringToCString( title ) + L"'", messageInfo );
			indent++;
			for( map< string, boost::shared_ptr< job > >::iterator it = subJobPtrMap.begin(); it != subJobPtrMap.end(); it++ )
				it->second->run( launchSubLevel - 1, true );
			--indent;
			message( stringToCString( indent.get() ) + L"Subjobs for '" + stringToCString( title ) + L"' done", messageInfo );
		}

		if( g.jobGlobal.launch ) {
			runIt( indent.getDepth() > 1, indent ); // run ourself
			fflush( stdout );
		}
		else
			message( stringToCString( indent.get() ) + L"NOT running job '" + stringToCString( title ) + L"'", messageInfo );
	}

	/**
	 * Creates XML from a job.
	 *
	 * The result looks like this:
	 *
	 *	<task title="blahblah">
	 *	  <substasks>
	 *      ...
	 *	  </substasks>
	 *	  <commands>
	 *      <command cpus="2">renderdl myrib.rib</command>
	 *    </commands>
	 * 	</task>
	 *
	 */
	string job::getXML( indentHelper indent ){
		globals& g = const_cast< globals& >( globals::access() );

		std::stringstream jobStr;

		jobStr << indent++ << "<task";
		jobStr << " title=\"" << title << "\"";
		jobStr << ">" << endl;

		if( !subJobPtrMap.empty() ) {
			jobStr << indent++ << "<subtasks>" << endl;
			for( map< string, boost::shared_ptr< job > >::iterator it = subJobPtrMap.begin(); it != subJobPtrMap.end(); it++ )
				jobStr << it->second->getXML( indent );
			jobStr << --indent << "</subtasks>" << endl;
		}

		if( !commands.empty() ) {
			jobStr << indent++ << "<commands>" << endl;
			for( vector< string >::iterator it = commands.begin(); it < commands.end(); it++ ) {
				jobStr << indent << "<command";
				jobStr << " cpus=\"" << CStringToString( CValue( ( long )g.jobGlobal.numCPUs ).GetAsText() ) << "\"";
				jobStr << ">";
				jobStr << *it;
				jobStr << "</command>" << endl;
			}
			jobStr << --indent << "</commands>" << endl;
		}

		if( !cleanUpCommands.empty() ) {
			jobStr << indent++ << "<cleanup>" << endl;
			for( vector< string >::iterator it = commands.begin(); it < commands.end(); it++ ) {
				jobStr << indent << "<command";
				jobStr << " cpus=\"" << CStringToString( CValue( ( long )g.jobGlobal.numCPUs ).GetAsText() ) << "\"";
				jobStr << ">";
				jobStr << *it;
				jobStr << "</command>" << endl;
			}
			jobStr << --indent << "</cleanup>" << endl;
		}

		jobStr << --indent << "</task>" << endl;

		return jobStr.str();
	}

	/**
	 * Writes an XML job script.
	 *
	 * The result looks like this:
	 *
	 *  <?xml version="1.0"?>
	 *  <jobscript title="blahblah">
     *    <substasks>
	 *      <task title="blahblah">
	 *        <substasks>
	 *          ...
	 *	      </substasks>
	 *	      <commands>
	 *          <command cpus="2">renderdl myrib.rib</command>
	 *        </commands>
	 * 	    </task>
	 *	  </substasks>
	 *    <commands>
	 *      <command cpus="2">renderdl myrib.rib</command>
	 *    </commands>
	 *  </jobscript>
	 *
	 */
	void job::writeXML( const string &destination ) {
		static indentHelper indent;

		globals& g = const_cast< globals& >( globals::access() );

		ofstream outXML( destination.c_str() );
		outXML << "<?xml version=\"1.0\"?>" << endl;
		outXML << indent++ << "<jobscript title=\"" + title + "\" type=\"render\" version=\"1.0.1\">" << endl;
		if( !subJobPtrMap.empty() ) {
			outXML << indent++ << "<subtasks>" << endl;
			for( map< string, boost::shared_ptr< job > >::iterator it = subJobPtrMap.begin(); it != subJobPtrMap.end(); it++ )
				outXML << it->second->getXML( indent );
			outXML << --indent << "</subtasks>" << endl;
		}
		if( !commands.empty() ) {
			outXML << indent++ << "<commands>" << endl;
			for( vector< string >::iterator it = commands.begin(); it < commands.end(); it++ ) {
				outXML << indent << "<command";
				outXML << " cpus=\"" << CStringToString( CValue( ( long )g.jobGlobal.numCPUs ).GetAsText() ) << "\"";
				outXML << ">";
				outXML << *it;
				outXML << "</command>" << endl;
			}
			outXML << --indent << "</commands>" << endl;
		}
		if( !cleanUpCommands.empty() ) {
			outXML << indent++ << "<cleanup>" << endl;
			for( vector< string >::iterator it = commands.begin(); it < commands.end(); it++ ) {
				outXML << indent << "<command";
				outXML << " cpus=\"" << CStringToString( CValue( ( long )g.jobGlobal.numCPUs ).GetAsText() ) << "\"";
				outXML << ">";
				outXML << *it;
				outXML << "</command>" << endl;
			}
			outXML << --indent << "</cleanup>" << endl;
		}
		outXML << --indent << "</jobscript>" << endl;
	}

	string job::getDumpXML( indentHelper indent ){
		globals& g = const_cast< globals& >( globals::access() );

		std::stringstream jobStr;

		jobStr << indent << "<ribTask";
		jobStr << " title=\"" << title << "\"";
		jobStr << ">" << endl;

		indent++;

		if( !subJobPtrMap.empty() ) {
			jobStr << indent << "<ribDependencies>" << endl;
			indent++;
			for( map< string, boost::shared_ptr< job > >::iterator it = subJobPtrMap.begin(); it != subJobPtrMap.end(); it++ )
				jobStr << it->second->getDumpXML( indent );
			jobStr << --indent << "</ribDependencies>" << endl;
		}

		if( !commands.empty() ) {
			for( vector< string >::iterator it = commands.begin(); it < commands.end(); it++ ) {
				jobStr << indent << "<ribFile>";
				size_t pos = it->rfind( " " );
				if( string::npos == pos )
					jobStr << *it;
				else
					jobStr << it->substr( pos + 1 );
				jobStr << "</ribFile>" << endl;
			}
		}

		jobStr << --indent << "</ribTask>" << endl;

		return jobStr.str();
	}

	void job::writeDumpXML( const string &destination ) {
		static indentHelper indent( 0 );
		string tmp;

		globals& g = const_cast< globals& >( globals::access() );

		ofstream outXML( destination.c_str() );

		outXML << "<?xml version=\"1.0\"?>" << endl;
		outXML << "<ribDump name=\"" + title + "\">" << endl;

		indent++;

		if( !subJobPtrMap.empty() ) {
			for( map< string, boost::shared_ptr< job > >::iterator it = subJobPtrMap.begin(); it != subJobPtrMap.end(); it++ )
				outXML << it->second->getDumpXML( indent.getDepth() );
		}
		if( !commands.empty() ) {
			for( vector< string >::iterator it = commands.begin(); it < commands.end(); it++ ) {
				tmp = *it;

				outXML << indent << "<ribFile>";
				size_t pos = it->rfind( " " );
				if( string::npos == pos )
					outXML << *it;
				else
					outXML << it->substr( pos + 1 );
				outXML << "</ribFile>" << endl;
			}
		}

		outXML << --indent << "</ribDump>" << endl;
	}
}
