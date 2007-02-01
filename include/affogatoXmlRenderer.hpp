#ifndef xmlRenderer_H
#define xmlRenderer_H
/** XML renderer class.
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
#include <fstream>
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <vector>

// Boost headers
#include <boost/shared_ptr.hpp>

// Affogato headers
#include "affogatoIndentHelper.hpp"
#include "affogatoRenderer.hpp"
#include "affogatoTokenValue.hpp"

#define MAXMOTIONSAMPLES 16

namespace ueberMan {

	using namespace std;
	using namespace affogato;

	class ueberManXmlRenderer : public ueberMan {
		public:
								ueberManXmlRenderer();
							   ~ueberManXmlRenderer() {};
			context	beginScene( const string &destination, bool useBinary = false, bool useCompression = false );
			void	switchScene( context ctx );
			context currentScene();
			void	endScene( context ctx );
			static	const ueberManXmlRenderer& accessRenderer();

			void	input( const string &filename );
			void	input( const string &filename, const float *bound );

			void	world();
			void	render( const cameraHandle &cameraname );

			void	camera( cameraHandle& cameraid );
			void	output( const string &name, const string &format,
							const string &dataname, const cameraHandle& camerid );

			void	motion( int ntimes, const float *times );

			void	parameter( const std::vector< tokenValue > &tokenValueArray );
			void	parameter( const tokenValue &aTokenValue );
			void	parameter( const string &typedname, const string &value );
			void	parameter( const string &typedname, const float value );
			void	parameter( const string &typedname, const int value );
			void	parameter( const string &typedname, const bool value );

			void	attribute( const tokenValue &aTokenValue );
			void	attribute( const string &typedname, const string &value );
			void	attribute( const string &typedname, const float value );
			void	attribute( const string &typedname, const int value );
			void	attribute( const string &typedname, const bool value );

			bool	getAttribute( const string &typedname, float &value );
			bool	getAttribute( const string &typedname, int &value );
			bool	getAttribute( const string &typedname, string &value );

			void	pushAttributes();
			void	popAttributes();

			//void	option( const tokenValue &aTokenValue );
			//void	option( const string &typedname, const string &value );
			//void	option( const string &typedname, float value );
			//void	option( const string &typedname, int value );
			//void	option( const string &typedname, bool value );

			void	pushSpace();
			void	popSpace();

			void	space( const float *matrix );
			void	space( const spaceHandle& spacename );
			void	nameSpace( spaceHandle& spacename );
			void	appendSpace( const float *matrix );

			void	translate( const float x, const float y, const float z );
			void	rotate( const float angle, const float x, const float y, const float z );
			void	scale( const float x, const float y, const float z );

			void	shader( const string &shadertype, const string &shadername, shaderHandle& shaderid );
			void	light( const string &sShadername, lightHandle& lightid );
			void	switchLight( const lightHandle &lightid, const bool on );

			void	beginLook( lookHandle& lookid );
			void	endLook();
			void	look( const lookHandle& id );
			void	appendLook( const lookHandle& id );

			void	curves( const string& interp, const int ncurves, const int nvertspercurve, const bool closed, primitiveHandle &identifier );
			void	curves( const string& interp, const int ncurves, const int *nvertspercurve, const bool closed, primitiveHandle &identifier );

			void	patch( const string& interp, const int nu, const int nv, primitiveHandle &identifier );
			void	patch( const int nu, const int uorder, const float *uknot, const float umin, const float umax,
						   const int nv, const int vorder, const float *vknot, const float vmin, const float vmax, primitiveHandle &identifier );

			void 	mesh( const string& interp, const int nfaces, const int *nverts, const int *verts, const bool interpolateBoundary, primitiveHandle &identifier );

			void	sphere( const float radius, const float zmin, const float zmax, const float thetamax, primitiveHandle &identifier );

		//protected:
			static	ueberManXmlRenderer theRenderer;
			//static  context currentContext;
			static  context currentContext;
			static	bool dso;
			indentHelper indent;
			vector< boost::shared_ptr< tokenValue > > tokenValueCache;

			ofstream outStream;

			short sampleCount;
			unsigned short numSamples;
			float motionSamples[ MAXMOTIONSAMPLES ];
			bool inWorldBlock;

			string	getTokenAsString( const tokenValue &aTokenValue );
			string	getTokenAsClassifiedString( const tokenValue &aTokenValue );
			void	dumpShaderTokenValues();
			void	dumpAttributeTokenValues();

			void	doPrimitive();
			bool	checkStartMotion();
			void	checkEndMotion();

			struct xmlShader {
				friend class ueberManXmlRenderer;
				typedef enum xmlShaderType {
					surface = 0,
					displacement = 1,
					volume = 2,
					lightsource = 3
				} xmlShaderType;

				xmlShader();
				xmlShader(	xmlShaderType t,
							const string &aId,
							const string &aName,
							const vector< boost::shared_ptr< tokenValue > > &aTokenValueArray );
				xmlShader( const xmlShader& cpy );
				void write( ueberManXmlRenderer& x );

				xmlShaderType type;
				string id;
				string name;
				vector< boost::shared_ptr< tokenValue > > tokenValueArray;
			};

			// a Look -- any combination of attributes and xmlShaders
			struct xmlLook {
				// each attribute only once
				map< string, boost::shared_ptr< tokenValue > > attributeMap;
				// each xmlShader type only once -- we use the type as the key
				map< string, boost::shared_ptr< xmlShader > > shaderTypeMap;
				// any number of lights but each light only once -- we use the name as the key
				map< string, boost::shared_ptr< xmlShader > > lightMap;
				// named looks we use
				map< string, xmlLook >lookMap;
				xmlLook() {}
				xmlLook( 	const stack< map< string, boost::shared_ptr< tokenValue > > > aAttributeMap,
						map< string, boost::shared_ptr< xmlShader > > aShaderTypeMap,
						map< string, boost::shared_ptr< xmlShader > > aLightMap	);
				xmlLook( const xmlLook& cpyLook ) {
					attributeMap = cpyLook.attributeMap;
					shaderTypeMap = cpyLook.shaderTypeMap;
					lightMap = cpyLook.lightMap;
				}
			};

			// To verify if a look the user wants to instance actually is known
			set< string >lookSet;

			// Our graphics state -- essentially a look
			typedef stack< boost::shared_ptr< xmlLook > > state;
			map< context, boost::shared_ptr< state > > graphicsState;
			state* currentState;

	};
}

#endif

