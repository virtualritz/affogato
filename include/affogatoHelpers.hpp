#ifndef affogatoHelpers_H
#define affogatoHelpers_H
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *  @par
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *	Boston, MA 02110-1301 USA or point your web browser to
 *	http://www.gnu.org/licenses/lgpl.txt.
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
#include <sstream>
#include <string>
#include <vector>

// Boost headers
#include <boost/filesystem/path.hpp>

// XSI headers
#ifdef __XSI_PLUGIN
	#include <xsi_matrix4.h>
	#include <xsi_primitive.h>
	#include <xsi_string.h>
	#include <xsi_x3dobject.h>
#endif

// Affogato headers
#include "affogatoTokenValue.hpp"

namespace affogato {

	using namespace boost;
	using namespace std;
#ifdef __XSI_PLUGIN
	using namespace XSI;
	typedef enum  messageType {
		messageInfo,
		messageWarning,
		messageError,
		messageDebug,
		messageForce
	} messageType;

	bool isVisible( const X3DObject& obj );
	vector< float >	getSequence( const string& seq );
	vector< float > getMotionSamples( const unsigned short motionsamples );
	vector< float > remapMotionSamples( const vector< float >& motionsamples );
	bool CStringToChar( const CString& theString, char *dest );
	string CStringToString( const CString& theString );
	CString charToCString( const char *theString );
	CString stringToCString( const string& theString );
	template< typename T > inline string toString( const T& t ) {
		stringstream ss;
		ss << t;
		return ss.str();
	}
	const vector< float >& CMatrix4ToFloat( const MATH::CMatrix4& in );

	void debugMessage( const CString& msg );
	void message( const CString& msg, messageType level );


	string getParameterTypeAsString( tokenValue::parameterType type );
	string getAffogatoName( const string& name );
	bool isAffogatoProperty( const Property& prop );
	CRefArray getAffogatoProperties( const X3DObject& obj );
#endif

	string parseString( const string& inputString, int frameNumber = -9999999 );
	string checkEnvironmentForFile( string envName, string fileName );

	filesystem::path makeAbsRelPath( const filesystem::path& base, const filesystem::path& path, bool relativePaths );
	filesystem::path checkFixCreateDir( const filesystem::path& baseDir, const filesystem::path& inputDir, bool createDir, const filesystem::path& fallbackDir );

	filesystem::path getCacheFilePath( const filesystem::path& toCache, bool cache = true );

	string sanitizeWindowsMultiPath( const string& paths );

	bool tidyUpCache();

	string fixTrailingSlash( const string& path );
	bool createFullPath( const filesystem::path& createPath );
	string cleanUpSearchPath( const string& path );

	vector< float > getBoundingBox( const XSI::Primitive& prim, double atTime );

	string getEnvironment( const string& envVar );

	Property updateGlobals( const Property& prop );

	class arrayDeleter // needed to free a shared_ptr to an array
	{
		public:
			template< typename T >
			void operator()( T* t )	{
				delete[] t;
			}
	};
}

#endif
