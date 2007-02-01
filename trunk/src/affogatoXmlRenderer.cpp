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
#include <sstream>
#include <iostream>
#include <string>

// XSI headers
#ifdef __XSI_PLUGIN
	#include <xsi_application.h>
#else
	#include <stdio.h>
#endif

// Affogato headers
#include "affogatoIndentHelper.hpp"
#include "affogatoXmlRenderer.hpp"
#ifdef __XSI_PLUGIN
#include "affogatoHelpers.hpp"
#endif

#ifndef __XSI_PLUGIN
#define debugMessage(a)
#endif


namespace ueberMan {

	using namespace std;
	using namespace affogato;


	// Public methods ---------------------------------------------------------


	ueberManXmlRenderer::ueberManXmlRenderer() {
		debugMessage( L"UeberManXml: Creating instance" );
		// Number of current Ri call's motion sample
		sampleCount = -1;

		// Total number of samples in current motion block
		numSamples = 1;

		currentContext = 0;

		inWorldBlock = false;
	}

	/**-
	 * Starts a new scene.
	 * It returns a context handle that can be used to switch to direct calls
	 * to that scene specifically later.
	 * For each context, a state instance is maintained. This ensures that the
	 * whole state can be restored, regardless when the context gets switched
	 */
	context ueberManXmlRenderer::beginScene( const string& destination, bool useBinary, bool useCompression ) {
		debugMessage( L"UeberManXml: BeginScene" );
		if( "dynamicload" != destination ) { // this check allows to use the API for DSOs too, where no RiBegin() is ever called

			if( !currentContext ) {
				string file = destination + ".xml";

				debugMessage( L"UeberManXml: BeginScene1" );
				outStream.open( file.c_str(), ofstream::out );
				dso = false;

				outStream << "<?xml version=\"1.0\"?>" << endl;

				size_t pos = destination.rfind( "/" );
				string strippedDestination;
				if( string::npos != pos )
					strippedDestination = destination.substr( pos + 1 );
				else
					strippedDestination = destination;

				debugMessage( L"UeberManXml: BeginScene2" );
				outStream << indent++;
				outStream << "<scene version=\"1.0\" title=\"" << strippedDestination << "\">" << endl;

				debugMessage( L"UeberManXml: Creating Graphics State" );
			}
		} else {
			dso = true;
		}

		// Number of current Ri call's motion sample
		sampleCount = -1;

		// Total number of samples in current motion block
		numSamples = 1;

		++currentContext;
		graphicsState[ currentContext ] = boost::shared_ptr< state >( new state );
		graphicsState[ currentContext ]->push( boost::shared_ptr< xmlLook >( new xmlLook ) );

		debugMessage( L"UeberManXml: Done BeginScene" );
		return currentContext;
	}

	void ueberManXmlRenderer::switchScene( context ctx ) {
		currentContext = ctx;
	}

	context	ueberManXmlRenderer::currentScene() {
		return currentContext;
	}

	void ueberManXmlRenderer::endScene( context ctx ) {
		debugMessage( L"UeberManXml: EndScene" );

		graphicsState.erase( ctx );
		if( !graphicsState.empty() ) {
			currentState = graphicsState.rbegin()->second.get();
		} else {
			outStream << --indent << "</scene>" << endl;
			outStream.close();
		}
	}

	const ueberManXmlRenderer& ueberManXmlRenderer::accessRenderer() {
		// Singleton instance of the renderer
		static ueberManXmlRenderer theRenderer;
		return theRenderer;
	}

	void ueberManXmlRenderer::input( const string& filename ) {
		debugMessage( L"UeberManXml: Input" );
		tokenValueCache.clear();
	}

	void ueberManXmlRenderer::input( const string& filename, const float bound[ 6 ] ) {
		debugMessage( L"UeberManXml: Input [bounded]" );
		tokenValueCache.clear();
	}

	void ueberManXmlRenderer::camera( cameraHandle &cameraId ) {
		debugMessage( L"UeberManXml: Camera" );
		tokenValueCache.clear();
	}

	void ueberManXmlRenderer::output( const string& name, const string& format, const string& dataname, const cameraHandle &cameraname ) {
		debugMessage( L"UeberManXml: Output" );
		tokenValueCache.clear();
	}

	void ueberManXmlRenderer::world() {
		debugMessage( L"UeberManXml: World" );

		inWorldBlock = true;
	}

	void ueberManXmlRenderer::render( const cameraHandle &cameraname ) {
	}

	void ueberManXmlRenderer::motion( int ntimes, const float *times ) {
		debugMessage( L"UeberManXml: Motion" );

		sampleCount = ntimes;
		numSamples = ntimes;
		memcpy( motionSamples, times, ntimes * sizeof( float ) );
	}

	void ueberManXmlRenderer::parameter( const vector< tokenValue > &tokenValueArray ) {
		for( unsigned i = 0; i < tokenValueArray.size(); i++ )
			parameter( tokenValueArray[ i ] );
	}

	void ueberManXmlRenderer::parameter( const tokenValue &aTokenValue ) {
		debugMessage( L"UeberManXml: Parameter [token-value]" );

		tokenValueCache.push_back( boost::shared_ptr< tokenValue >( new tokenValue( aTokenValue ) ) );
	}

	void ueberManXmlRenderer::parameter( const string& typedname, float value ) {
		debugMessage( L"UeberManXml: Parameter [float]" );

		tokenValueCache.push_back( boost::shared_ptr< tokenValue >( new tokenValue( value, typedname ) ) );
	}

	void ueberManXmlRenderer::parameter( const string& typedname, int value ) {
		debugMessage( L"UeberManXml: Parameter [int]" );

		tokenValueCache.push_back( boost::shared_ptr< tokenValue >( new tokenValue( value, typedname ) ) );
	}

	void ueberManXmlRenderer::parameter( const string& typedname, const string& value ) {
		debugMessage( L"UeberManRi: Parameter [string]" );

		tokenValueCache.push_back( boost::shared_ptr< tokenValue >( new tokenValue( value, typedname ) ) );
	}

	void ueberManXmlRenderer::parameter( const string& typedname, bool value ) {
		debugMessage( L"UeberManXml: Parameter [bool]" );

		tokenValueCache.push_back( boost::shared_ptr< tokenValue >( new tokenValue( value, typedname ) ) );
	}

	void ueberManXmlRenderer::attribute( const tokenValue &aTokenValue ) {
		checkStartMotion();
		debugMessage( L"UeberManXml: Attribute [token-value]" );

		currentState->top()->attributeMap[ aTokenValue.name() ] = boost::shared_ptr< tokenValue >( new tokenValue( aTokenValue ) );

		checkEndMotion();
	}

	void ueberManXmlRenderer::attribute( const string& typedname, float value ) {
		checkStartMotion();

		debugMessage( L"UeberManXml: Attribute [float]" );
		currentState->top()->attributeMap[ typedname ] = boost::shared_ptr< tokenValue >( new tokenValue( value, typedname ) );

		//outStream << indent << "<attribute name=\"" << typedname << "\" type=\"float\">" << value << "</attribute>" << endl;

		checkEndMotion();
	}

	void ueberManXmlRenderer::attribute( const string& typedname, int value ) {
		checkStartMotion();

		debugMessage( L"UeberManXml: Attribute [int]" );
		currentState->top()->attributeMap[ typedname ] = boost::shared_ptr< tokenValue >( new tokenValue( value, typedname ) );

		//outStream << indent << "<attribute name=\"" << typedname << "\" type=\"int\">" << value << "</attribute>" << endl;

		checkEndMotion();
	}

	void ueberManXmlRenderer::attribute( const string& typedname, const string& value ) {
		checkStartMotion();

		currentState->top()->attributeMap[ typedname ] = boost::shared_ptr< tokenValue >( new tokenValue( value, typedname ) );

		//	outStream << indent << "<attribute name=\"" << typedname << "\" type=\"string\">" << value << "</attribute>" << endl;

		checkEndMotion();
	}

	void ueberManXmlRenderer::attribute( const string& typedname, bool value ) {
		checkStartMotion();

		debugMessage( L"UeberManXml: Attribute [bool]" );
		currentState->top()->attributeMap[ typedname ] = boost::shared_ptr< tokenValue >( new tokenValue( value, typedname ) );

		//outStream << indent << "<attribute name=\"" << typedname << "\" type=\"int\">" << value << "</attribute>" << endl;

		checkEndMotion();
	}

	bool ueberManXmlRenderer::getAttribute( const string& typedname, float &value ) {

		return false;
	}

	bool ueberManXmlRenderer::getAttribute( const string& typedname, int &value ) {

		return false;
	}

	bool ueberManXmlRenderer::getAttribute( const string& typedname, string& value ) {

		return false;
	}


	void ueberManXmlRenderer::pushAttributes() {
		debugMessage( L"UeberManXml: PushAttributes" );

		currentState->push( boost::shared_ptr< xmlLook >( new xmlLook( *currentState->top() ) ) );
	}

	void ueberManXmlRenderer::popAttributes() {
		debugMessage( L"UeberManXml: PopAttributes" );

		currentState->pop();
	}

	void ueberManXmlRenderer::pushSpace() {
		debugMessage( L"UeberManXml: PushTransform" );

	}

	void ueberManXmlRenderer::popSpace() {
		debugMessage( L"UeberManXml: PopTransform" );

	}

	void ueberManXmlRenderer::space( const float *matrix ) {
		checkStartMotion();

		debugMessage( L"UeberManXml: Space [matrix]" );


		checkEndMotion();
	}

	void ueberManXmlRenderer::space( const spaceHandle &spacename ) {
		checkStartMotion();


		checkEndMotion();
	}

	void ueberManXmlRenderer::nameSpace( spaceHandle &spacename ) {
		checkStartMotion();

		checkEndMotion();
	}

	void ueberManXmlRenderer::appendSpace( const float *matrix ) {
		checkStartMotion();

		checkEndMotion();
	}

	void ueberManXmlRenderer::translate( float x, float y, float z ) {
		checkStartMotion();

		checkEndMotion();
	}

	void ueberManXmlRenderer::rotate( float angle, float x, float y, float z ) {
		debugMessage( L"UeberManXml: Rotate" );

		checkEndMotion();
	}

	void ueberManXmlRenderer::scale( float x, float y, float z ) {
		debugMessage( L"UeberManXml: Scale" );

		checkEndMotion();
	}

	void ueberManXmlRenderer::beginLook( lookHandle& id ) {

		debugMessage( L"UeberManXml: BeginLook" );
		outStream << indent++ << "<look id=\"" << id << "\">" << endl;
//		dumpAttributeTokenValues();

	}

	void ueberManXmlRenderer::endLook() {

		debugMessage( L"UeberManXml: EndLook" );
		outStream << --indent << "</look>" << endl;

	}

	void ueberManXmlRenderer::look( const lookHandle& id ) {
		outStream << "<lookInstance>" << id << "</lookInstance>" << endl;
	}

	void ueberManXmlRenderer::appendLook( const lookHandle& id ) {
		outStream << "<appendedLookInstance>" << id << "</appendedLookInstance>" << endl;
	}

	void ueberManXmlRenderer::shader( const string& shadertype, const string& shadername, shaderHandle& shaderid ) {
		debugMessage( L"UeberManXml: Shader" );

		xmlShader::xmlShaderType t;
		if( "surface" == shadertype ) {
			t = xmlShader::surface;
		} else
		if( "displacement" == shadertype ) {
			t = xmlShader::displacement;
		} else
		if( "volume" == shadertype ) {
			t = xmlShader::volume;
		}

		currentState->top()->shaderTypeMap[ shadertype ] = boost::shared_ptr< xmlShader >( new xmlShader( t, shaderid, shadername, tokenValueCache ) );
		tokenValueCache.clear();

		/*outStream << indent++ << "<shader";
		outStream << " id=\"" << shaderid;
		outStream << "\" name=\"" << shadername;
		outStream << "\" class=\"" << shadertype;
		outStream << "\">" << endl;
		dumpShaderTokenValues();

		outStream << --indent << "</shader>" << endl;*/
	}

	void ueberManXmlRenderer::light( const string& shadername, lightHandle& lightid ) {
		debugMessage( L"UeberManXml: Light" );

		currentState->top()->lightMap[ shadername ] = boost::shared_ptr< xmlShader >( new xmlShader( xmlShader::lightsource, lightid, shadername, tokenValueCache ) );
		tokenValueCache.clear();

		/*		outStream << indent++ << "<light";
		outStream << " id=\"" << lightid;
		outStream << "\" name=\"" << shadername;
		outStream << "\">" << endl;
		dumpShaderTokenValues();
		tokenValueCache.clear();
		outStream << --indent << "</light>" << endl;*/
	}

	//void ueberManXmlRenderer::connectShaders( const string& srcId, const string& srcName, const string& destId, const string& destName ) {
	//
	//	map[ destId ]
	//}



	void ueberManXmlRenderer::switchLight( const lightHandle& lightid, const bool on ) {
		debugMessage( L"UeberManXml: LightSwitch" );

		checkStartMotion();

		outStream << indent << "<lightswitch id=\"" << lightid << "\">" << ( on ? "on" : "off" ) << "</lightswitch>" << endl;

		checkEndMotion();

	}

	void ueberManXmlRenderer::curves( const string &interp, const int ncurves, const int nvertspercurve, const bool closed, primitiveHandle &identifier ) {

		if( checkStartMotion() ) {

			debugMessage( L"UeberManXml: Curves" );
			//string newId = writeAttributeSet( primitiveid );
			doPrimitive();
		}

		checkEndMotion();
	}
/*
	void flushAttributeCache() {

		outStream << "<attributelist name=\"" << identifier << "\">" << endl;
		for( vector< tokenValue >::iterator it = attributeTokenValueCache.begin(); it < attributeTokenValueCache.end(); it++ ) {
			outStream << "<attribute ";
			outStream << getTokenAsString( aTokenValue );
			outStream << ">" << endl;
			outStream << "\t" << aTokenValue.dataAsString() << endl;
			outStream << "</attribute>" << endl;
		}
		outStream << "</attributelist>" << endl;
	}*/

	void ueberManXmlRenderer::curves( const string& interp, const int ncurves, const int *nvertspercurve, bool closed, primitiveHandle &identifier ) {

		if( checkStartMotion() ) {

			debugMessage( L"UeberManXml: Curves" );
			doPrimitive();
		}

		checkEndMotion();
	}

	void ueberManXmlRenderer::patch( const string& interp, const int nu, const int nv, primitiveHandle &identifier ) {

		if( checkStartMotion() ) {

			debugMessage( L"UeberManXml: Patch" );
			doPrimitive();
		}

		checkEndMotion();
	}

	void ueberManXmlRenderer::patch( const int nu, const int uorder, const float *uknot, const float umin, const float umax, const int nv, const int vorder, const float *vknot, const float vmin, const float vmax, primitiveHandle &identifier ) {
		if( checkStartMotion() ) {

			debugMessage( L"UeberManXml: NuPatch" );
			doPrimitive();
		}

		checkEndMotion();
	}

	void ueberManXmlRenderer::mesh( const string& interp, const int nfaces, const int *nverts, const int *verts, const bool interpolateBoundary, primitiveHandle &identifier ) {
		if( checkStartMotion() ) {

			debugMessage( L"UeberManXml: Mesh" );
			//string newId = writeLook( primitiveid );


			outStream << indent++ << "<primitive id=\"" << identifier << "\">" << endl;
			//outStream << indent << "<lookInstance>" << newId << "</lookInstance>" << endl;
			outStream << --indent << "</primitive>" << endl;

			//doPrimitive();
		}

		checkEndMotion();
	}

	void ueberManXmlRenderer::sphere( const float radius, const float zmin, const float zmax, const float thetamax, primitiveHandle &identifier ) {
		if( checkStartMotion() ) {

			debugMessage( L"UeberManXml: Sphere" );

			doPrimitive();
		}

		checkEndMotion();
	}

	// Define static class members --------------------------------------------

	//vector< RtContextHandle >ueberManXmlRenderer::contextArray( 10 );
	// The current context to index into our stateMachine array
	context ueberManXmlRenderer::currentContext;

	// State machine stuff
	//vector< ueberManXmlRenderer::state* >ueberManXmlRenderer::stateMachine( 10 );
	//ueberManXmlRenderer::state *ueberManXmlRenderer::currentState;

	bool ueberManXmlRenderer::dso;


	string ueberManXmlRenderer::getTokenAsString( const tokenValue &aTokenValue ) {

		stringstream attrib;
		attrib << "name=\"" << aTokenValue.name() << "\"";
		attrib << " type=\"" << aTokenValue.typeAsString() << "\"";

		size_t size = aTokenValue.size();
		if( ( 1 < size ) && ( tokenValue::typeString != aTokenValue.type() ) )
			attrib << " size=\"" << size << "\"";

		return attrib.str();
	}

	string ueberManXmlRenderer::getTokenAsClassifiedString( const tokenValue &aTokenValue ) {

		std::stringstream attrib;
		attrib << "name=\"" << aTokenValue.name() << "\"";

		attrib << " class=\"";

		switch( aTokenValue.storage() ) {
			case tokenValue::storageConstant:
				attrib << "constant ";
				break;
			case tokenValue::storagePerPiece:
				attrib << "uniform ";
				break;
			case tokenValue::storageLinear:      // varying
				attrib << "varying ";
				break;
			case tokenValue::storageVertex:
				attrib << "vertex ";
				break;
			case tokenValue::storageFaceVarying: // Needs to be translated into linear for Gelato
				attrib << "facevarying ";
				break;
			case tokenValue::storageFaceVertex:  // Currently unsupported in Gelato
				attrib << "facevertex ";
				break;
		};
		attrib << "\"";

		attrib << " type=\"" << aTokenValue.typeAsString() << "\"";

		size_t size = aTokenValue.size();
		if( ( 1 < size ) && ( tokenValue::typeString != aTokenValue.type() ) )
			attrib << " size=\"" << size << "\"";

		return attrib.str();
	}


	/** Checks if we need to open a MotionBegin block.
	 *
	 *  The reason we do this is that for curves & patches, eventually the
	 *  basis needs to be changed. If that is the case, we need to emit
	 *  an RiBasis call which can't go inside the motion block for obvious
	 *  reasons.
	 */
	bool ueberManXmlRenderer::checkStartMotion() {
		if( sampleCount == numSamples ) {
			debugMessage( L"UeberManXml: MotionBegin" );

			// make sure we don't emit another MotionBegin call if the API's
			// user forgets to close the current one
			++numSamples;
			return true;
		}
		return false;
	}

	void ueberManXmlRenderer::checkEndMotion() {
		--sampleCount;
		// Close a motion block when the counter reaches zero
		if( !sampleCount ) {
			debugMessage( L"UeberManXml: MotionEnd" );
		}
		// We're outside of a motion block
		else if( 0 > sampleCount ) {
			sampleCount = -1;

		}
	}



	void ueberManXmlRenderer::dumpAttributeTokenValues() {

		map< string, boost::shared_ptr< tokenValue > > &tmpMap = currentState->top()->attributeMap;

		/*for( map< string, boost::shared_ptr< tokenValue > >::iterator it = tmpMap.begin(); it != tmpMap.end(); it++ ) {
			/*outStream << indent << "<attribute " << getTokenAsString( *( it->second ) ) << ">";

			if( ( 1 < it->second->size() ) && ( tokenValue::typeString != it->second->type() ) )
				outStream << endl << indent + 1 << it->second->dataAsString() << endl << indent;
			else
				outStream << it->second->dataAsString();

			outStream << "</attribute>" << endl;
		}*/
	}

	/*void writeThingStart( const string& thing, const string& id, const string& name ) {
		outStream << indent++ << "<" << thing;
		outStream << indent << " id=\"" << id;
		outStream << indent << "\" name=\"" << name
		outStream << indent << "\">" << endl;
	}

	void writeThingEnd( const string& thing, const string& attributeId ) {
		outStream << indent << "<attributeSetInstance>" << attributeId << "</attributeSetInstance>";
		outStream << --indent << "</" << thing << ">" << endl;
	}*/

	void ueberManXmlRenderer::doPrimitive() {
		dumpAttributeTokenValues();
		//outStream << indent++ << "<primitive>" << endl;
		//outStream << indent << identifierStack.top() << endl;
		//outStream << --indent << "</primitive>" << endl;
		tokenValueCache.clear();
	}

	ueberManXmlRenderer::xmlShader::xmlShader() {
		type = surface;
		name = "default";
		id = "noid";
	}

	ueberManXmlRenderer::xmlShader::xmlShader( xmlShaderType t, const string& aId, const string& aName, const vector< boost::shared_ptr< tokenValue > > &aTokenValueArray )
	:	type( t ),
		id( aId ),
		name( aName ),
		tokenValueArray( aTokenValueArray ) {
	}

	ueberManXmlRenderer::xmlShader::xmlShader( const xmlShader &cpy ) {
		type = cpy.type;
		name = cpy.name;
		id = cpy.id;
		tokenValueArray = cpy.tokenValueArray;
	}

	void ueberManXmlRenderer::xmlShader::write( ueberManXmlRenderer& x ) {

		x.outStream << x.indent++ << "<shader";

		x.outStream << " id=\"" << id;
		x.outStream << "\" name=\"" << name;
		x.outStream << "\" class=\"";

		switch( type ) {
			case surface:
				x.outStream << "surface";
				break;
			case displacement:
				x.outStream << "displacement";
				break;
			case volume:
				x.outStream << "volume";
				break;
			case lightsource:
				x.outStream << "light";
				break;
		}

		x.outStream << "\">" << endl;
		for( vector< boost::shared_ptr< tokenValue > >::iterator it = tokenValueArray.begin(); it < tokenValueArray.end(); it++ ) {
			x.outStream << x.indent << "<parameter " << x.getTokenAsString( *( *it ) ) << ">";

			if( ( 1 < ( *it )->size() ) && ( tokenValue::typeString != ( *it )->type() ) )
				x.outStream << endl << x.indent + 1 << ( *it )->dataAsString() << endl << x.indent;
			else
				x.outStream << ( *it )->dataAsString();

			x.outStream << "</parameter>" << endl;
		}
		x.outStream << --x.indent << "</shader>" << endl;
	}

}


