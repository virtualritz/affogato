#ifndef affogatoShader_H
#define affogatoShader_H
/** Shader data management class.
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
#include <string>

// Boost headers
#include <boost/filesystem/path.hpp>

// XSI headers
#include <xsi_ref.h>
#include <xsi_status.h>

// Affogato headers
#include "affogatoTokenValue.hpp"

#define __AFFOGATO_SHADER_ID L"__AffogatoShaderID"
#define __AFFOGATO_SHADER_PARAMS L"__AffogatoShaderParams"


namespace affogato {

	using namespace XSI;
	using namespace std;

	class shader {
		public:
			typedef enum shaderType {
				shaderUndefined = -1,
				shaderSurface,
				shaderLight,
				shaderDisplacement,
				shaderVolume,
				shaderTransformation,
				shaderDeformation,
				shaderImager
			};

			shader();
			shader( const shader &aShader );
			shader( const Parameter &aShader );
			shader( const Property &aShader );

			shader( shaderType theType, const string &theName );
			~shader();

			void set( const Parameter &aShader );
			void set( const Property &aShader );
			void addParameter( const tokenValue &aTokenValue );
			bool isValid() const;
			shaderType getType() const;
			void write( const string &lightHandle = "" );
			static bool isShader( const Property &aShader );
			static shaderType getType( const Property &aShader );
		private:
			string					name;
			shaderType				type;
			//string				lightHandle;
			float					displacementSphere;
			string					displacementSpace;
			vector< tokenValue::tokenValuePtr > tokenValuePtrArray;
	};

}

using namespace XSI;

#ifdef unix
extern "C"
#endif
CStatus AffogatoShader_Define( const CRef &in_Ctx );
#ifdef unix
extern "C"
#endif
CStatus AffogatoShader_DefineLayout( const CRef &in_Ctx );


#endif
