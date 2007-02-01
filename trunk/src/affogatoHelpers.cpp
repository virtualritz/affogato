/** Global helper functions.
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
#include <math.h>
#include <memory>
#include <string>
#include <sstream>
#include <wchar.h>

// Boost headers
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>

// XSI headers
#ifdef __XSI_PLUGIN
	#include <xsi_application.h>
	#include <xsi_geometry.h>
	#include <xsi_matrix4.h>
	#include <xsi_primitive.h>
	#include <xsi_string.h>
#endif

// Affogato headers
#include "affogato.hpp"
#include "affogatoGlobals.hpp"
#include "affogatoHelpers.hpp"
#include "affogatoTokenValue.hpp"


namespace affogato {

	using namespace std;
	using namespace boost;

#ifdef __XSI_PLUGIN
	using namespace XSI;

	bool isVisible( const X3DObject& obj ) {
		CRefArray props( obj.GetProperties() );
		props.Filter( L"visibility", CStringArray(), CString(), props );

		return ( bool )Property( props[ 0 ] ).GetParameterValue( L"rendvis", const_cast< globals& >( globals::access() ).animation.time );
	}

	/** Returns a frame sequence from a string object.
	 */
	vector< float > getSequence( const string& seq ) {
		std::set< float > theSeq;
		vector< float > theFrames;

		typedef tokenizer< char_separator< char > > tokenizer;

		char_separator< char > comma( "," );
		tokenizer frames( to_lower_copy( seq ), comma );

		for( tokenizer::iterator it( frames.begin() ); it != frames.end(); it++ ) {
			size_t pos( it->find( "-" ) );
			if( string::npos == pos ) {
				float f( atof( it->c_str() ) );
				if( theSeq.end() == theSeq.find( f ) ) {
					theSeq.insert( f );
					theFrames.push_back( f );
				}
			} else {
				float startFrame( atof( it->substr( 0, pos ).c_str() ) );
				float endFrame, frameStep;
				// Support both RSP- & Shake frame sequence syntax
				size_t pos2( it->find( "@" ) );
				if( string::npos == pos2 ) {
					endFrame = ( float )atof( it->substr( pos + 1 ).c_str() );
					frameStep = 1;
				}
				else {
					endFrame = ( float )atof( it->substr( pos + 1, pos2 - pos ).c_str() );
					frameStep = ( float )fabs( atof( it->substr( pos2 + 1 ).c_str() ) );
				}
				if( startFrame < endFrame ) {
					for( float f( startFrame ); f <= endFrame; f += frameStep ) {
						if( theSeq.end() == theSeq.find( f ) ) {
							theSeq.insert( f );
							theFrames.push_back( f );
						}
					}
					if( theSeq.end() == theSeq.find( endFrame ) )
						theFrames.push_back( endFrame );
				}
				else {
					for( float f( startFrame ); f >= endFrame; f -= frameStep ) {
						if( theSeq.end() == theSeq.find( f ) ) {
							theSeq.insert( f );
							theFrames.push_back( f );
						}
					}
					if( theSeq.end() == theSeq.find( endFrame ) )
						theFrames.push_back( endFrame );
				}
			}
		}

		return theFrames;
	}

	vector< float > getMotionSamples( const unsigned short motionsamples ) {
		const globals& g( globals::access() );
		vector< float > sampletimes( motionsamples );
		float step, start;
		if( g.motionBlur.subFrame ) {
			start = g.motionBlur.shutterOpen;
			step  = fabs( g.motionBlur.shutterClose - start );
		} else {
			start = floor( g.motionBlur.shutterOpen );
			step  = fabs( ceil( g.motionBlur.shutterClose ) - start );
		}
		step /= motionsamples - 1;
		for( unsigned short motion = 0; motion < motionsamples; motion++ )
			sampletimes[ motion ] = g.animation.time + start + motion * step;
		return sampletimes;
	}

	vector< float > remapMotionSamples( const vector< float >& motionsamples ) {
		const globals& g( globals::access() );
		float offset( g.motionBlur.shutterOffset );
		if( globals::motionBlur::shutterStationary == g.motionBlur.shutterConfiguration )
			offset -= g.animation.time;
		vector< float > sampletimes( motionsamples.size() );
		for( unsigned short motion = 0; motion < motionsamples.size(); motion++ )
			sampletimes[ motion ] = motionsamples[ motion ] + offset;
		return sampletimes;
	}

	bool CStringToChar( const CString &theString, char *dest ) {
		const wchar_t* buffer( theString.GetWideString() );
		wcstombs( dest, buffer, theString.Length() + 1 );
		return true;
	}

	string CStringToString( const CString &theString ) {
#ifdef _WIN32
		return string( theString.GetAsciiString() );
#else
		const wchar_t* buffer( theString.GetWideString() );
		char *dest( new char[ theString.Length() + 1 ] );
		wcstombs( dest, buffer, theString.Length() + 1 );
		string result( dest );
		delete[] dest;
		return result;
#endif
	}

	CString charToCString( const char *theString ) {
		int length( strlen( theString ) + 1 );
		wchar_t* buffer( new wchar_t[ length ] );

		mbstowcs( buffer, theString, length );

		CString result( buffer );
		delete[] buffer;

		return CString( result );
	}

	CString stringToCString( const string &theString ) {
#ifdef _WIN32
		CString tmp;
		tmp.PutAsciiString( theString.c_str() );
		return tmp;
#else
		return charToCString( theString.c_str() );
#endif
	}

	const vector< float>& CMatrix4ToFloat( const MATH::CMatrix4 &in ) {
		static vector< float > transform( 16 );
		for( unsigned i( 0 ); i < 16; i++ )
			transform[ i ] = ( float )in.GetValue( i / 4, i % 4 );
		return transform;
	}


	void debugMessage( const CString &msg ) {
#ifndef _WIN32
		FILE *f( fopen( "/tmp/affogato.log", "a" ) );
		fprintf( f, "  #DEBUG: %s\n", CStringToString( msg ).c_str() );
		fflush( f );
		fclose( f );
//#else
//		FILE *f( fopen( "c:\tmp\affogato.log", "a" ) );
//		fprintf( f, "  #DEBUG: %s\n", CStringToString( msg ).c_str() );
//		fflush( f );
//		fclose( f );
#endif
		Application app;
		app.LogMessage( msg, siVerboseMsg );
	}


	void message( const CString &msg, messageType level ) {
#ifdef DEBUG
#ifndef _WIN32
		//extern ofstream debugStream;
		FILE *f;
		f = fopen( "/tmp/affogato.log", "a" );

		switch( level ) {
			case messageInfo:
				//debugStream << "   #INFO: ";
				fprintf( f, "   #INFO: %s\n", CStringToString( msg ).c_str() );
				break;
			case messageWarning:
				//debugStream << "#WARNING: ";
				fprintf( f, "#WARNING: %s\n", CStringToString( msg ).c_str() );
				break;
			case messageError:
				//debugStream << "! #ERROR: ";
				fprintf( f, "! #ERROR: %s\n", CStringToString( msg ).c_str() );
				break;
			case messageForce:
				//debugStream << " #FORCED: ";
				fprintf( f, " #FORCED: %s\n", CStringToString( msg ).c_str() );
				break;
			case messageDebug:
			default:
				//debugStream << "  #DEBUG: ";
				fprintf( f, "  #DEBUG: %s\n", CStringToString( msg ).c_str() );
		}
		fflush( f );
		fclose( f );
		//debugStream << CStringToString( msg ) << endl;
		//debugStream.flush();
#endif
#endif

		const globals& g( globals::access() );
		static Application app;

		CString message( CString( L"Affogato: " ) + msg );

		if( messageForce == level ) {
			app.LogMessage( message );
		} else {
			switch( g.feedback.verbosity ) {
				case globals::feedback::verbosityAll:
					if( messageInfo == level )
						app.LogMessage( message, siInfoMsg );
				case globals::feedback::verbosityWarningsAndErrors:
					if( messageWarning == level )
						app.LogMessage( message, siWarningMsg );
				case globals::feedback::verbosityErrors:
					if( messageError == level )
						app.LogMessage( message, siErrorMsg );
			}
		}
	}

#endif

	filesystem::path getCacheFilePath( const filesystem::path& toCache, bool cache ) {
		filesystem::path tmp;
		if( cache ) {
			string newPath( toCache.string() );
			replace_all( newPath, string( "/" ), string( "%" ) );
			try {
				tmp = globals::access().directories.cache / filesystem::path( newPath, filesystem::no_check );
			} catch( filesystem::filesystem_error ) {
				message( L"Ill-formed cache path; cache might not work as expected.", messageWarning );
			}
		} else {
			tmp = toCache;
		}

		return tmp;
	}


	string sanitizeWindowsMultiPath( const string& paths ) {
		typedef tokenizer< char_separator< char > > tokenizer;

		char_separator< char > sep( ":" );
		tokenizer tokens( paths, sep );

		vector< string > allPaths;

		for( tokenizer::iterator it( tokens.begin() ); it != tokens.end(); it++ ) {
			string path( *it );
			tokenizer::iterator jt( it );
			++jt;
			if( ( 1 == path.length() ) && ( jt != tokens.end() ) ) {
				allPaths.push_back( path + ":" + *jt );
				it = jt;
			} else {
				allPaths.push_back( path );
			}
		}

/*string newPath;
				try {
					newPath = filesystem::path( *jt ).string();
				} catch( filesystem::filesystem_error ) {
					newPath = "/" + *jt;
				}
				allPaths.push_back( "//" + path + newPath );



			} else {
				try {
					allPaths.push_back( filesystem::path( path ).string() );
				} catch( filesystem::filesystem_error ) {
					allPaths.push_back( path );
				}
			}*/

		string returnString;
		for( vector< string >::iterator it( allPaths.begin() ); it < allPaths.end(); it++ ) {
			Application app;
			app.LogMessage( stringToCString( *it ) );
			if( ':' == ( *it )[ 1 ] )
				returnString += "//" + it->substr( 0, 1 ) + replace_all_copy( *it, "\\", "/" ).substr( 2 );
			else
				returnString += *it;
			returnString += ":";
		}

		if( !returnString.empty() )
			returnString = returnString.substr( 0, returnString.length() - 1 );

		return returnString;
	}

    /** Cleans out Affogato's disk cache.
	 */
	bool tidyUpCache() {
		const globals& g( globals::access() );
		bool success( true );
		if( g.directories.caching.size && !g.directories.cache.empty() ) { // A zero size cache is unlimited, return sucess
			map< time_t, filesystem::path > files;
			map< time_t, intmax_t > sizes;
			intmax_t totalSize( 0 );
			// FSind all files and their sizes
			filesystem::directory_iterator end;
			for( filesystem::directory_iterator it( g.directories.cache ); it != end; it++ ) {
				if( !is_directory( *it ) ) {
					time_t t( filesystem::last_write_time( *it ) );
					files[ t ] = *it;
					sizes[ t ] = filesystem::file_size( *it );
					totalSize += sizes[ t ];
				}
			}
			// We need to tidy up
			if( totalSize > g.directories.caching.size ) {
				success = false;
				map< time_t, filesystem::path >::const_iterator f( files.begin() );
				map< time_t, intmax_t >::const_iterator s( sizes.begin() );
				// Delete oldes files first (aka those with youngest date)
				while( f != files.end() ) {
					filesystem::remove( f->second );
					totalSize -= s->second;
					if( totalSize < g.directories.caching.size ) {
						success = true;
						break;
					}
					f++;
					s++;
				}
			}
		}
		return success;
	}

	/** Parses a string and substitutes global- & environment variables.
	 */
	string parseString( const string &inputString, int frameNumber ) {

		const globals& g( globals::access() );

		size_t start( inputString.find_first_not_of( " \t\n\r" ) );
		if( string::npos == start )
			start = 0;
		size_t end( inputString.find_last_not_of( " \t\n\r" ) );
		if( string::npos == end )
			end = inputString.length() - 1;

		end -= start - 1;

		string trimmedInputString = inputString.substr( start, end );

		string constructedString;
		string tokenString;
		bool inToken( false );
		int sLength( trimmedInputString.length() );

		int frame;
		if( frameNumber == -9999999 ) {
			frame = ( int )floor( g.animation.time );
		} else {
			frame = frameNumber;
		}

		for( unsigned i = 0; i < sLength; i++ ) {
			bool escaped( i ? trimmedInputString[ i - 1 ] == '\\' : false );
			if( trimmedInputString[ i ] == '$' ) {
				tokenString.clear();
				inToken = true;
			} else if ( inToken ) {
				tokenString += trimmedInputString[ i ];
				if ( tokenString == "F" ) {
					constructedString += frame;
					inToken = false;
					tokenString.clear();
				} else
				/*if( tokenString == "SCN" ) {
					constructedString += liqglo_sceneName;
					inToken = false;
					tokenString.clear();
				} else
				if( tokenString == "IMG" ) {
					constructedString += liqglo_DDimageName[0];
					inToken = false;
					tokenString.clear();
				} else */
				if( tokenString == "PDIR" ) {
					constructedString += g.directories.base.string();
					inToken = false;
					tokenString.clear();
				} else
				if( ( tokenString == "DDIR" ) || ( tokenString == "RDIR" ) ) {
					constructedString += g.directories.data.string();
					inToken = false;
					tokenString.clear();
				}
				else {
					constructedString += '$';
					constructedString += tokenString;
					tokenString.clear();
					inToken = false;
				}
			} else if( !escaped && ( trimmedInputString[ i ] == '#' ) ) {
				int paddingSize( 0 );
				while( i < sLength && trimmedInputString[ i ] == '#' ) {
					paddingSize++;
					i++;
				}
				i--;
				if( paddingSize == 1 ) {
					paddingSize = 4;
				}
				if( paddingSize > 20 ) {
					paddingSize = 20;
				}
				char paddedFrame[ 21 ];

				sprintf( paddedFrame, "%0*ld", paddingSize, frame );
				constructedString += paddedFrame;
			} else if( !escaped && ( trimmedInputString[ i ] == '%' ) ) {
				string	envString;
				char*	envVal = NULL;

				i++;
				// loop through the string looking for the closing %
				if( i < sLength ) {
					while ( i < sLength && trimmedInputString[ i ] != '%' ) {
						envString += trimmedInputString[ i ];
						i++;
					}

					constructedString += getEnvironment( envString );
					// else environment variable doesn't exist.. do nothing
				}
			// else early exit: % was the last character in the string.. do nothing
			} else if( escaped && ( trimmedInputString[ i ] == 'n' ) ) {
				constructedString += "\n";
				i++;
			} else if( escaped && ( trimmedInputString[ i ] == 't' ) ) {
				constructedString += "\t";
				i++;
			} else {
				constructedString += trimmedInputString[ i ];
			}
		}

		return constructedString;
	}

	string checkEnvironmentForFile( string envName, string fileName ) {
		using namespace filesystem;

		string returnStr;

		string search( getEnvironment( envName ) );
		if( !search.empty() ) {
			typedef tokenizer< char_separator< char > > tokenizer;

			char_separator< char > sep( ":" );
			tokenizer tokens( search, sep );

			for( tokenizer::iterator it = tokens.begin(); it != tokens.end(); it++ ) {
				string path = *it;
				if( ( path.rfind( "/" ) != path.length() - 1 ) ) {
					path += "/";
				}

				if( exists( path + fileName ) ) {
					returnStr = path;
					break;
				}
			}
		}
		return returnStr;
	}

	string getParameterTypeAsString( tokenValue::parameterType type ) {
		switch( type ) {
			case tokenValue::typeFloat:
				return "float";
			case tokenValue::typeInteger:
				return "int";
			case tokenValue::typeColor:
				return "color";
			case tokenValue::typePoint:
				return "point";
			case tokenValue::typeHomogenousPoint:
				return "hpoint";
			case tokenValue::typeVector:
				return "vector";
			case tokenValue::typeNormal:
				return "normal";
			case tokenValue::typeMatrix:
				return "matrix";
			case tokenValue::typeString:
				return "string";
		}
		return "";
	}


	string getAffogatoName( const string &name ) {
		string objName( name );
		replace_all( objName, string( "." ), string( "|" ) );
		//objName = "|" + objName;
		return objName;
	}

	bool isAffogatoProperty( const Property& prop ) {
		return ( CStringToString( prop.GetName() ).substr( 0, 8 ) == "affogato" ) && ( CString( L"CustomProperty" ) == prop.GetClassIDName() );
	}

	CRefArray getAffogatoProperties( const X3DObject& obj ) {
		CRefArray affogatoProps, props( obj.GetProperties() );
		props.Filter( CString(), CStringArray(), L"affogato*", affogatoProps );
		return affogatoProps;
	}

	filesystem::path makeAbsRelPath( const filesystem::path& base, const filesystem::path& path, bool relativePaths = false ) {

		filesystem::path returnDir( path );

		try {
			if( relativePaths ) {
				size_t pos = path.string().find( base.string() );
				if( !pos ) { // path started base with  base -> pos = 0
					returnDir = path.string().substr( base.string().length() );
				}
			} else if( !path.is_complete() ) {
				returnDir = base / path;
			}
		} catch( filesystem::filesystem_error ) {
			message( L"Problem forming absolute/relative path from '" + stringToCString( base.string() ) + L"' and '" + stringToCString( path.string() ) + L"'.", messageWarning );
		}

		return returnDir;
	}

	/**
	 * Checks path is nonempty and creates the directory if nonexistant
	 */
	filesystem::path checkFixCreateDir( const filesystem::path& baseDir, const filesystem::path& inputDir, bool createDir, const filesystem::path& fallbackDir ) {

		filesystem::path returnDir( fallbackDir );
		filesystem::path makeDir;

		try {
			if( !inputDir.is_complete() || inputDir.empty() ) {
				makeDir = baseDir / inputDir;
			} else {
				makeDir = inputDir;
			}

			if( !makeDir.empty() ) {
				if( filesystem::exists( makeDir ) ) {
					returnDir = inputDir;
				} else if( createDir ) {
					if( createFullPath( makeDir ) ) {
						returnDir = inputDir;
					}
				} else {
					message( L"Path '" + stringToCString( makeDir.string() ) + L"' does not exist; falling back to '" + stringToCString( fallbackDir.string() ) + L"'", messageWarning );
				}
			}
		} catch( filesystem::filesystem_error ) {
			message( L"Problem checking/fixing/creating directory '" + stringToCString( inputDir.string() ) + L"'.", messageWarning );
		}

		return returnDir;
	}

	/**
	 * Createas a full path. Aka: iteratoes over the path and consecutively creates directories if nonexistant
	 */
	bool createFullPath( const filesystem::path& createPath ) {
		filesystem::path create;
		bool returnValue;
		try {
			for( filesystem::path::iterator it = createPath.begin(); it != createPath.end(); it++ ) {
				create /= *it;
				if( !filesystem::exists( create ) )
					filesystem::create_directory( create );
			}
			returnValue = true;
		}
		catch( filesystem::filesystem_error ) {
			message( stringToCString( "Creating directory '" + createPath.string() + "' failed" ), messageError );
			returnValue = false;
		}
		return returnValue;
	}

	/**
	 * Adds a trailing '/' to a path if the path is nonempty and lacks it
	 */
	string fixTrailingSlash( const string& path ) {
		string returnDir = path;
		if( returnDir.length() ) {
			if( '/' != returnDir[ returnDir.length() - 1 ] ) {
				returnDir += '/';
			}
		}
		return returnDir;
	}

	/**
	 * Removes whitespace from a searchpath
	 */
	string cleanUpSearchPath( const string& path ) {
		typedef tokenizer< char_separator< char > > tokenizer;

		char_separator< char > sep( ":" );
		tokenizer tokens( path, sep );

		string result;

		for( tokenizer::iterator it = tokens.begin(); it != tokens.end(); it++ )	{
			result += fixTrailingSlash( trim_copy( *it ) ) + ':';
		}
		if( !result.empty() )
			result = result.substr( 0, result.length() - 1 );

		//result = replace_all_copy( result, string( " " ), string( "\\ " ) );
		//result = replace_all_copy( result, string( "\\\\" ), string( "\\" ) );

		return result;
	}


	vector< float > getBoundingBox( const Primitive &prim, double atTime ) {
		vector< float > bound( 6 );
		double centerX, centerY, centerZ, extendX, extendY, extendZ;
		CTransformation tmp;
		prim.GetGeometry( atTime ).GetBoundingBox( centerX, centerY, centerZ, extendX, extendY, extendZ, tmp );
		bound[ 0 ] = ( float )( centerX - 0.5f * extendX );
		bound[ 1 ] = ( float )( centerX + 0.5f * extendX );
		bound[ 2 ] = ( float )( centerY - 0.5f * extendY );
		bound[ 3 ] = ( float )( centerY + 0.5f * extendY );
		bound[ 4 ] = ( float )( centerZ - 0.5f * extendZ );
		bound[ 5 ] = ( float )( centerZ + 0.5f * extendZ );
		return bound;
	}


	string getEnvironment( const string& envVar ) {
		string ret;
		char* tmp( getenv( envVar.c_str() ) );
		if( tmp )
			ret = tmp;
		return ret;
	}


	Property updateGlobals( const Property& prop ) {
		map< string, CValue > paramsMap;
		Property newProp;

		CParameterRefArray params( prop.GetParameters() );
		SceneItem parent( prop.GetParent() );
		CString propName( prop.GetName() );
		string name( CStringToString( propName ) );
		if( AFFOGATOVERSION != CStringToString( prop.GetParameterValue( L"___AffogatoVersion" ) ) ) {
			Application app;
			app.LogMessage( L"Affogato: Updating globals '" + prop.GetFullName() + L"'", siWarningMsg );
			for( unsigned j = 0; j < ( unsigned )params.GetCount(); j++ ) {
				Parameter param( params[ j ] );
				string paramName( CStringToString( param.GetScriptName() ) );
				if( "___AffogatoVersion" != paramName )
					paramsMap[ paramName ] = param.GetValue();
			}
			CValue retval;
			CValueArray args( 1 );
			args[ 0 ] = stringToCString( name );
			app.ExecuteCommand( L"DeleteObj", args, retval );

			newProp = parent.AddProperty( L"AffogatoGlobals", false, propName );
			CParameterRefArray params( newProp.GetParameters() );
			for( unsigned j = 0; j < ( unsigned )params.GetCount(); j++ ) {
				Parameter param( params[ j ] );
				string name( CStringToString( param.GetScriptName() ) );
				for( map< string, CValue >::const_iterator it = paramsMap.begin(); it != paramsMap.end(); it++ ) {
					if( name == it->first ) {
						CValue tmp( it->second ); // Make sure we match data type, or else this will change the parameter type (e.g. bool->int)
						tmp.ChangeType( param.GetValue().m_t );
						param.PutValue( tmp );
					}
				}
			}

#ifdef RSP // For backwards-compatibility with old pipeline tools
			if( paramsMap.end() == paramsMap.find( "Frames" ) ) {
				switch( ( long )newProp.GetParameterValue( L"FrameOutput" ) ) {
					case 1:   // Start to End
					case 3: { // Sequence
						char s[ 32 ];
						sprintf( s, "%d-%d@%d", long( paramsMap[ "StartFrame" ] ), long( paramsMap[ "EndFrame" ] ), long( paramsMap[ "FrameStep" ] ) );
						// newProp.PutParameterValue( L"Frames", stringToCString( ( format( "%d-%d@%d" ) % long( paramsMap[ "StartFrame" ] ) % long( paramsMap[ "EndFrame" ] ) % long( paramsMap[ "FrameStep" ] ) ).str() ) );
						newProp.PutParameterValue( L"Frames", stringToCString( s ) );
						newProp.PutParameterValue( L"FrameOutput", 3l );
					}
					case 0: {
						newProp.PutParameterValue( L"FrameOutput", 0l );
						break;
					}
					case 2: {
						char s[ 32 ];
						sprintf( s, "%d@%d", long( paramsMap[ "StartFrame" ] ), long( paramsMap[ "FrameStep" ] ) );
						// newProp.PutParameterValue( L"Frames", stringToCString( ( format( "%d@%d" ) % long( paramsMap[ "StartFrame" ] ) % long( paramsMap[ "FrameStep" ] ) ).str() ) );
						newProp.PutParameterValue( L"Frames", stringToCString( s ) );
						newProp.PutParameterValue( L"FrameOutput", 3l );
					}
				}
			}
#endif
		} else {
			newProp = prop;
		}

		return newProp;
	}
}

