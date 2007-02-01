#ifndef riRenderer_H
#define riRenderer_H
/** RenderMan Renderer interface class.
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
#include <vector>
#include <string>

// Boost headers
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

// RenderMan headers
#include <ri.h>

// Affogato headers
#include "affogatoRenderer.hpp"
#include "affogatoTokenValue.hpp"


namespace ueberMan {

	using namespace std;
	using namespace affogato;

	class ueberManRiRenderer : public ueberMan {
		public:
								ueberManRiRenderer();
							   ~ueberManRiRenderer();
			context	beginScene( const string& destination, bool useBinary = false, bool useCompression = false );
			void	switchScene( context ctx );
			void	endScene( context ctx );

			static	const	ueberManRiRenderer& accessRenderer();

			void	input( const string& filename );
			void	input( const string& filename, const float *bound );

			void	world();
			void	render( const cameraHandle &cameraname );

			void	camera( cameraHandle& cameraid );
			void	output( const string& name, const string& format,
							const string& dataname, const cameraHandle& camerid );

			void	motion( const vector< float >& times );

			void	parameter( const std::vector< tokenValue >& tokenValueArray );
			void	parameter( const tokenValue &aTokenValue );
			void	parameter( const string& typedname, const string& value );
			void	parameter( const string& typedname, const float value );
			void	parameter( const string& typedname, const int value );
			void	parameter( const string& typedname, const bool value );

			void	attribute( const tokenValue &aTokenValue );
			void	attribute( const string& typedname, const string& value );
			void	attribute( const string& typedname, const float value );
			void	attribute( const string& typedname, const int value );
			void	attribute( const string& typedname, const bool value );

			bool	getAttribute( const string& typedname, float &value );
			bool	getAttribute( const string& typedname, int &value );
			bool	getAttribute( const string& typedname, string& value );

			void	pushAttributes();
			void	popAttributes();

			void	option( const tokenValue &aTokenValue );
			void	option( const string& typedname, const string& value );
			void	option( const string& typedname, const float value );
			void	option( const string& typedname, const int value );
			void	option( const string& typedname, const bool value );

			void	pushSpace();
			void	popSpace();

			void	space( const vector< float >& matrix );
			void	space( const spaceHandle& spacename );
			void	nameSpace( spaceHandle& spacename );
			void	appendSpace( const vector< float >& matrix );

			void	translate( const float x, const float y, const float z );
			void	rotate( const float angle, const float x, const float y, const float z );
			void	scale( const float x, const float y, const float z );

			void	shader( const string& shadertype, const string& shadername, shaderHandle& );
			void	light( const string& shadername, lightHandle& lightid );
			void	switchLight( const lightHandle &lightid, const bool on );

			void	beginLook( lookHandle& lookid );
			void	endLook();
			void	look( const lookHandle& id );
			void	appendLook( const lookHandle& id );

			void	points( const string& type, const int numPoints, primitiveHandle& );

			void	curves( const string& interp, const int ncurves, const int numVertsPerCurve, const bool, primitiveHandle& );
			void	curves( const string& interp, const int ncurves, const vector< int >& numVertsPerCurve, const bool, primitiveHandle& );
			void	curves( const int, const vector< int >&, const vector< int >&, const vector< float >&, const vector< float >&, const vector< float >&, primitiveHandle& identifier );

			void	patch( const string& interp, const int nu, const int nv, primitiveHandle&identifier );
			void	patch(	const int nu, const int uorder, const float *uknot,	const float umin, const float umax,
							const int nv, const int vorder, const float *vknot,	const float vmin, const float vmax, primitiveHandle& );

			void 	mesh( const string& interp, const int nfaces, const int *nverts, const int *verts, const bool interpolateBoundary, primitiveHandle& );

			void	sphere(	const float, const float, const float, const float, primitiveHandle& );

			void	blobby( const int numLeafs, const vector< int >& code, const vector< float >& floatData, const vector< string >& stringData, primitiveHandle& identifier );

			void	makeMap( const string& type );

		private:

			bool dso;
			//boost::shared_array< RtToken > tokens;
			//boost::shared_array< RtPointer >values;
			//vector< tokenValue::parameterType > valueTypes;

			struct  state {
				state();
				state( const state &cpy );
				~state();

				string		getTokenAsString( const tokenValue& aTokenValue );
				string		getTokenAsClassifiedString( const tokenValue& aTokenValue );
				unsigned 	fillPrimitiveTokenValueArrays( boost::shared_array< RtToken >&, boost::shared_array< RtPointer >&, vector< tokenValue::parameterType >& );
				unsigned	fillShaderTokenValueArrays( boost::shared_array< RtToken >&, boost::shared_array< RtPointer >&, vector< tokenValue::parameterType >& );
				void		resetValueArray( boost::shared_array< RtPointer >& values, const vector< tokenValue::parameterType >& valueTypes );
				void		checkStartMotion();
				void		checkEndMotion();

				vector< tokenValue::tokenValuePtr > tokenValueCache;
				short sampleCount;
				unsigned short numSamples;
				bool inWorldBlock;
				bool secondaryDisplay;
				vector< float > motionSamples;
				unsigned numParams;
				RtContextHandle renderContext;
			};
			map< context, boost::shared_ptr< state > > stateMachine;
			state* currentState;
			context currentContext;
			context contextCounter;
	};
}

#endif

