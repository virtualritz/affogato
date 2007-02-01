#ifndef renderer_H
#define renderer_H
/** Renderer interface class.
 *
 *  This class is designed to call an arbitray number of registered
 *  renderers. The idea was that once can render certain types of
 *  geometry or parts of the scene with a different renderer each.
 *  This would require a mask that selects the renderer per call/per
 *  gprim type, This feature was never implemented so far. :)
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
#include <map>
#include <set>
#include <string>
#include <vector>

// Affogato headers
#include "affogatoTokenValue.hpp"


namespace ueberMan {

	using namespace std;
	using namespace affogato;

	typedef long context;
	static const context contextUndefined = -1;

	typedef string cameraHandle;
	typedef string spaceHandle;
	typedef string lookHandle;
	typedef string shaderHandle;
	typedef string lightHandle;
	typedef string objectHandle;
	typedef string primitiveHandle;

	typedef vector< float > matrix;

	extern spaceHandle cameraSpace, worldSpace, screenSpace, ndcSpace;

	/**
	 * Abstract base class to derive renderers from
	 *
	 */
	class ueberMan {

		public:

				ueberMan() {}
			   ~ueberMan() {}

		virtual context	beginScene( const string& destination, bool useBinary = false, bool useCompression = false ) { return 0; }
		virtual void	switchScene( context ctx ) {}
		virtual context	currentScene() { return 0; }
		virtual void	endScene( context ctx = contextUndefined ) {}

		virtual void	input( const string& filename ) {}
		virtual void	input( const string& filename, const float *bound ) {}

		virtual void	camera( cameraHandle& name ) {}
		virtual void	output( const string& name, const string& format,
								const string& dataname, const cameraHandle& cameraid ) {}

		virtual void	world() {}
		virtual void	render( const cameraHandle& cameraid ) {}

		//virtual void motion( int ntimes, float time0, ... ) {}

		virtual void	motion( const vector< float >& times ) {}

		virtual void	parameter( const vector< tokenValue > &tokenValueArray ) {}
		virtual void	parameter( const tokenValue &aTokenValue ) {}
		virtual void	parameter( const string& typedname, const string& value ) {}
		virtual void	parameter( const string& typedname, const float value ) {}
		virtual void	parameter( const string& typedname, const int value ) {}
		virtual void	parameter( const string& typedname, const bool value ) {}

		virtual void	variable( const vector< tokenValue > &tokenValueArray ) {}
		virtual void	variable( const tokenValue &aTokenValue ) {}
		virtual void	variable( const string& typedname, const string& value ) {}
		virtual void	variable( const string& typedname, const float value ) {}
		virtual void	variable( const string& typedname, const int value ) {}
		virtual void	variable( const string& typedname, const bool value ) {}

		virtual void	attribute( const tokenValue &aTokenValue ) {}
		virtual void	attribute( const string& typedname, const string& value ) {}
		virtual void	attribute( const string& typedname, const float value ) {}
		virtual void	attribute( const string& typedname, const int value ) {}
		virtual void	attribute( const string& typedname, const bool value ) {}

		virtual bool	getAttribute( const string& typedname, float &value ) { return true; }
		virtual bool	getAttribute( const string& typedname, int &value ) { return true; }
		virtual bool	getAttribute( const string& typedname, string& value ) { return true; }

		virtual void	pushAttributes() {}
		virtual void	popAttributes() {}

		virtual void	option( const tokenValue &aTokenValue ) {}
		virtual void	option( const string& typedname, const string& value ) {}
		virtual	void	option( const string& typedname, const float value ) {}
		virtual void	option( const string& typedname, const int value ) {}
		virtual void	option( const string& typedname, const bool value ) {}

		/*
		virtual void saveAttributes( const char *name, const char *attrs=NULL ) {}
		virtual void loadAttributes( const char *name, const char *attrs=NULL ) {}
		*/
		// Transformations
		virtual void	pushSpace() {}
		virtual void	popSpace() {}

		virtual void	space( const vector< float >& matrix ) {}
		virtual void	space( const spaceHandle &spacename ) {}
		virtual void	nameSpace( spaceHandle& spacename ) {}
		virtual void	appendSpace( const vector< float >& matrix ) {}

		virtual void	translate( const float x, const float y, const float z ) {}
		virtual void	rotate( const float angle, const float x, const float y, const float z ) {}
		virtual void	scale( const float x, const float y, const float z ) {}

		// Shaders

		// Begin a shader tree. Inside the tree, shaders can be connected using their IDs
		// The tree itself has an id too, which allows for connecting nested tree structures
		// and instancing shader trees using loadShaderTree()
		// All shader IDs of any shaders instanced before the call to shaderTreeBegin() become
		// invalid
		virtual	void	shaderTreeBegin( shaderHandle& treeid ) {}
		virtual	void	shaderTreeEnd() {}
		// Create an instance of a previously defined shader tree
		// If the tree ID is undefined, the graphics state will not be altered
		virtual	void	shaderTree( const string& treeid ) {}
		// Connect too shaders inside the tree through their IDs
		// This creates a new 'node' named nodeid
		virtual	void	connectShaders( const shaderHandle& srcId, const string& srcName, const shaderHandle& destId, const string& destName, shaderHandle& nodeid ) {}

		// Instance shader name of given type
		// This takes an optional shader ID to identify the shaderprimitiveHandle
		// The ID is returned or, if omitted, a unique (within the current scene or shaderTree) random ID is generated and returned
		virtual void	shader( const string& shadertype, const string& shadername, shaderHandle &shaderid ) {}
		virtual void	light( const string& shadername, lightHandle& lightid ) {}
		virtual	void	switchLight( const lightHandle& lightid, const bool on = true ) {}

		// Looks

		// Start defining a look
		// Unlike nameLook(), which saves the current graphics state under a look ID
		// this actually only saves attributes and shaders instanced after the lookBegin()
		// Read: it resets the graphics state completely until after the lookEnd(), which restores it.
		// Once a look it is defined, it can't be overwritten. Read: it is not possible to ever use the
		// same look ID again in the current context to name or begin a look. If the look ID given
		// is taken already, both beginLook() and nameLook() throw an out_of_range execption.
		// To actually apply a look defined like this, look() has to be called after the lookEnd()
		// and before the resp. primitive.
		virtual	void	beginLook( lookHandle& lookid ) {}
		virtual	void	endLook() {}
		// Save the attribute state and shaders under a look ID
		virtual	void	nameLook( lookHandle& lookid ) {}
		// Load the attribute state and shaders saved under the given look ID
		// If the look ID is undefined, the graphics state will not be altered
		virtual	void	look( const lookHandle& lookid ) {}
		// Append the look to the current graphics state
		virtual	void	appendLook( const lookHandle& lookid ) {}

		// Objects
		// Start defining an object instance
		// An instace is any object
		virtual void	beginObject( objectHandle& instanceid ) {}
		virtual void	endObject() {}
		virtual void	loadObject( const objectHandle& instanceid ) {}

		// Geometry
		// 0-D prims - point clouds
		virtual void	points( const string& type, const int numPoints, primitiveHandle& identifier ) {}

		// 1-D prims - lines, curves, hair
		virtual void	curves( const string& interp, const int numCurves, const int numVertsPerCurve, const bool closed, primitiveHandle& identifier ) {}
		virtual void	curves( const string& interp, const int numCurves, const vector< int >& numVertsPerCurve, const bool closed, primitiveHandle& identifier ) {}
		virtual void	curves( const int numCurves, const vector< int >& numVertsPerCurve, const vector< int >& order, const vector< float >& knot, const vector< float >& min, const vector< float >& max, primitiveHandle& identifier ) {}
		/*
		virtual void curves (int numCurves, int nvertspercurve, int order,
							 const float *knot, float vmin, float vmax) {}*/

		// 2-D prims - rectangular patches (NURBS, bicubics, bilinears), and
		// indexed face meshes (polys, polyhedra, subdivs)
		virtual void	patch( const string& interp, const int nu, const int nv, primitiveHandle& identifier ) {}
		virtual void	patch( const int nu, const int uorder, const float *uknot,
							const float umin, const float umax,
							const int nv, const int vorder, const float *vknot,
							const float vmin, const float vmax, primitiveHandle& identifier ) {}
		/*virtual void trimCurve (int nloops, const int *numCurves, const int *n,
								const int *order, const float *knot,
								const float *min, const float *max,
								const float *uvw) {}*/

		virtual void	mesh( const string& interp, const int nfaces, const int *nverts, const int *verts, const bool interpolateBoundary, primitiveHandle& identifier ) {}

		virtual void	sphere( const float radius, const float zmin, const float zmax, const float thetamax, primitiveHandle& identifier ) {}
		virtual void	sphere( const float radius, primitiveHandle& identifier ) { sphere( radius, -radius, radius, 360, identifier ); }

		virtual void	blobby( const int numLeafs, const vector< int >& code, const vector< float >& floatData, const vector< string >& stringData, primitiveHandle& identifier ) {}

		virtual void	makeMap( const string& type ) {}
	};


	class ueberManInterface : ueberMan {

		public:
					ueberManInterface();
				   ~ueberManInterface() {};

			/** Main renderInterface access point.
			 *  Returns a pointer to the Singleton instance of the
			 *  renderInterface to issue API calls through.
			 *
			 *  Example:
			 *
			 *  // plug-in userdata:
			 *  gelatoRenderer delight;
			 *  ueberManInterface init;
			 *  init.registerRenderer( ( renderer* )delight.accessRendere() );
			 *
			 *  // somewhere (get interface access):
			 *  ueberManInterface r;
			 *  // issue an API call to blahblah() which will call
			 *  // delight.blahblah():
			 *  r.blahblah();
			 *
			 */

			/**
			 *  Adds a renderer to the uerberManInterface.
			 *  Any API issued to the renderInterface call will be forwarded
			 *  to the given renderer
			 */
			void 	registerRenderer( const ueberMan& theRenderer );
			void	unregisterRenderer( const ueberMan& theRenderer );

			context beginScene( const string& destination, bool useBinary = false, bool useCompression = false );
			void	switchScene( context ctx );
			context	currentScene();
			void	endScene( context ctx = contextUndefined );

			void	render( const cameraHandle& cameraid );

			void	input( const string& filename );
			void	input( const string& filename, const float *bound );

			void	camera( cameraHandle& cameraid );
			void	output( const string& name, const string& format,
							const string& dataname, const cameraHandle &cameraid );

			/**
             *  Resets the transformation to indenty.
			 *  This establishes world space and tells the renderer that the
			 *  actual scene description will follow.
			 */
			void	world();
			void	motion( const vector< float >& times );

			/**
			 *  Add token value arrays.
			 */
			void	parameter( const vector< tokenValue > tokenValueArray );
			void	parameter( const tokenValue &aTokenValue );
			void	parameter( const string& typedname, const string& value );
			void	parameter( const string& typedname, const float value );
			void    parameter( const string& typedname, const int value );
			void    parameter( const string& typedname, const bool value );

			void	variable( const vector< tokenValue > tokenValueArray );
			void	variable( const tokenValue &aTokenValue );
			void	variable( const string& typedname, const string& value );
			void	variable( const string& typedname, const float value );
			void    variable( const string& typedname, const int value );
			void    variable( const string& typedname, const bool value );

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

			void	beginLook( lookHandle& lookid );
			void	endLook();
			void	nameLook( lookHandle& lookid );
			void	look( const lookHandle& lookid );
			void	appendLook( const lookHandle& lookid );
			static	set< lookHandle > getLooks();

			void	shader( const string& shadertype, const string& shadername, shaderHandle& shaderid );
			void	light( const string& shadername, lightHandle& lightid );

			void	switchLight( const lightHandle& lightid, const bool on = true );

			void	points( const string& type, const int numPoints, primitiveHandle& identifier );

			void	curves( const string& interp, const int numCurves, const int numVertsPerCurve, const bool closed, primitiveHandle& identifier );
			void	curves( const string& interp, const int numCurves, const vector< int >& numVertsPerCurve, const bool closed, primitiveHandle& identifier );
			void	curves( const int numCurves, const vector< int >& numVertsPerCurve, const vector< int >& order, const vector< float >& knot, const vector< float >& min, const vector< float >& max, primitiveHandle& identifier );

			void	patch( const string& interp, const int nu, const int nv, primitiveHandle& identifier );
			void	patch(	const int nu, const int uorder, const float *uknot,	const float umin, const float umax,
							const int nv, const int vorder, const float *vknot,	const float vmin, const float vmax, primitiveHandle& identifier );

			void 	mesh( const string& interp, const int nfaces, const int *nverts, const int *verts, const bool interpolateBoundary, primitiveHandle& identifier );

			void	sphere(	const float radius, const float zmin, const float zmax, const float thetamax, primitiveHandle& identifier );
			void 	sphere( const float radius, primitiveHandle& identifier );

			void	blobby( const int numLeafs, const vector< int >& code, const vector< float >& floatData, const vector< string >& stringData, primitiveHandle& identifier );

			void	makeMap( const string& type );

		private:

			static vector< ueberMan* > rendererList;

			/*struct renderContext {
				context renderingContext;
				set< lookHandle > lookSet;
			}*/

			static map< context, boost::shared_ptr< vector< context > > >contextArrayMap;
			static ueberManInterface theUeberManInterface;
			static context contextCounter;
			static context currentContext;

			static set< lookHandle > lookSet;

	};

};

#endif
