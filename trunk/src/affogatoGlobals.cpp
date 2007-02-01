/** The Affogato globals class keeps all global data that needs to be
 *  accessible from anywahere in the plug-in.
 *
 *  This is implemented through public members. Would be better to
 *  have e.g. an overloaded general data type and create a map of
 *  strings with this. Then provide methods to acess this map in a
 *  controlled way. Aka: needs total refactoring.
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
#include <math.h>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <stdlib.h>
#endif

// Boost headers
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>

// XSI headers
#include <xsi_application.h>
#include <xsi_model.h>
#include <xsi_parameter.h>
#include <xsi_time.h>
#include <xsi_value.h>

// Affogato headers
#include "affogato.hpp"
#include "affogatoGlobals.hpp"
#include "affogatoHelpers.hpp"

// other headers
#include "xmlParser.h"


namespace affogato {

	using namespace XSI;
	using namespace std;
	using namespace boost;

	bool globals::getBoolAttribute( XMLNode& xNode, const string& s, bool& value ) {
		const char* valueCharPtr = xNode.getAttribute( s.c_str() );
		if( valueCharPtr ) {
			string x( valueCharPtr );
			to_lower( x );
			value = !( "off" == x );
			return true;
		} else
			return false;
	}

	bool globals::getIntAttribute( XMLNode& xNode, const string& s, int& value ) {
		const char* valueCharPtr = xNode.getAttribute( s.c_str() );
		if( valueCharPtr ) {
			value = atol( valueCharPtr );
			return true;
		} else
			return false;
	}

	bool globals::getFloatAttribute( XMLNode& xNode, const string& s, float& value ) {
		const char* valueCharPtr = xNode.getAttribute( s.c_str() );
		if( valueCharPtr ) {
			value = ( float )atof( valueCharPtr );
			return true;
		} else
			return false;
	}

	bool globals::getStringAttribute( XMLNode& xNode, const string& s, string& value ) {
		const char* valueCharPtr = xNode.getAttribute( s.c_str() );
		if( valueCharPtr ) {
			value = valueCharPtr;
			return true;
		} else
			return false;
	}

	globals::globals( const string &xmlFile ) {
		set( xmlFile );
	}

	globals::globals( const Property &affogatoGlobals ) {
		set( affogatoGlobals );
	}

	void globals::scan( const string &xmlFile ) {

		debugMessage( stringToCString( "Parsing '" + xmlFile + "'" ) );

		XMLNode xMainNode;
		// Check if that file exists -- the XML parser class is not very good at handling non exising files (it crashes) :)
		if( filesystem::exists( xmlFile ) )
			xMainNode = XMLNode::openFileHelper( const_cast< char* >( xmlFile.c_str() ), "PMML" );
		else
			throw( runtime_error( "File: '" + xmlFile + "' does not exist" ) );

		debugMessage( L"Start Scanning");

		string tempString;
		int tempInt;
		globals& g = const_cast< globals& >( access() );

		debugMessage( L"Parsing <xi:include> tag" );

		XMLNode xNode = xMainNode.getChildNode( "xi:include" );
		if( !xNode.isEmpty() ) {

			string include;

			if( getStringAttribute( xNode, "href", tempString ) )
				scan( tempString ); // Recursion!
		}

		debugMessage( L"Parsing <frames> tag" );

		// <frames> tag
		xNode = xMainNode.getChildNode( "frames" );

		if( getIntAttribute( xNode, "startFrame", tempInt ) ) {
			g.time.startFrame					= ( float )tempInt;
			g.time.frameOutput 					= time::frameStartToEnd;
		}

		if( getIntAttribute( xNode, "endFrame", tempInt ) ) {
			g.time.endFrame						= ( float )tempInt;
			g.time.frameOutput 					= time::frameStartToEnd;
		}

		if( getIntAttribute( xNode, "frameStep", tempInt ) ) {
			g.time.frameStep					= ( float )tempInt;
			g.time.frameOutput 					= time::frameStartToEnd;
		}

		debugMessage( L"Parsing <file> tag" );
		// <file> tag
		xNode = xMainNode.getChildNode( "file" );

		if( getIntAttribute( xNode, "resolutionX", tempInt ) )
			g.resolution.x = tempInt;

		if( getIntAttribute( xNode, "resolutionY", tempInt ) )
			g.resolution.y = tempInt;

		getFloatAttribute( xNode, "pixelAspect", g.resolution.pixelAspect );

		getFloatAttribute( xNode, "resolutionmultiplier", g.resolution.multiplier );

		if( getStringAttribute( xNode, "frames", tempString ) ) {
			g.time.sequence						= tempString;
			g.time.frameOutput 					= time::frameSequence;
		}

		debugMessage( L"Parsing <job> tag" );
		// <job> tag
		xNode = xMainNode.getChildNode( "job" );

		if( getStringAttribute( xNode, "type", tempString ) ) {
			to_lower( tempString );
			if( "	" == tempString )
				g.jobGlobal.jobScript.type = jobGlobal::jobScript::jobScriptOff;
			else if( "xml" == tempString )
				g.jobGlobal.jobScript.type = jobGlobal::jobScript::jobScriptXML;
			else if( "jobengine" == tempString )
				g.jobGlobal.jobScript.type = jobGlobal::jobScript::jobScriptJobEngineXML;
			else if( "alfred" == tempString )
				g.jobGlobal.jobScript.type = jobGlobal::jobScript::jobScriptAlfred;
		}

		if( getStringAttribute( xNode, "frames", tempString ) ) {
			g.time.sequence						= tempString;
			g.time.frameOutput 					= time::frameSequence;
		}

		if( getStringAttribute( xNode, "dir", tempString ) )
			g.directories.temp = tempString;

		getStringAttribute( xNode, "blocknumber", g.name.blockName );

		if( getStringAttribute( xNode, "launch", tempString ) ) {
			to_lower( tempString );
			if( "off" == tempString ) {
				g.jobGlobal.launch = jobGlobal::launchOff;
				g.jobGlobal.launchSub = false;
			}
			else if( ( "renderer" == tempString ) || ( "on" == tempString ) ) {
				g.jobGlobal.launch = jobGlobal::launchRenderer;
				g.jobGlobal.launchSub = true;
			}
			else if( ( "job" == tempString ) || ( "interpreter" == tempString ) ) {
				g.jobGlobal.launch = jobGlobal::launchJobInterpreter;
				g.jobGlobal.launchSub = true;
			}
		}

		getBoolAttribute( xNode, "launchsub", g.jobGlobal.launchSub );

		getStringAttribute( xNode, "interpreter", g.jobGlobal.jobScript.interpreter	);

		if( getIntAttribute( xNode, "chunksize", tempInt ) )
			g.jobGlobal.jobScript.chunkSize = tempInt;

		// <renderman> tag
		debugMessage( L"Parsing <renderman> tag" );
		XMLNode xRManNode = xMainNode.getChildNode( "renderman" );

		//   <renderer> tag
		xNode = xRManNode.getChildNode( "renderer" );

		getStringAttribute( xNode, "version", g.renderer.version );

		getStringAttribute( xNode, "command", g.renderer.command );

		//   <reyes> tag
		xNode = xRManNode.getChildNode( "reyes" );

		getIntAttribute( xNode, "bucketsizex", ( int& )g.reyes.bucketSize.x );
		getIntAttribute( xNode, "bucketsizey", ( int& )g.reyes.bucketSize.y );

		if( getIntAttribute( xNode, "bucketsize", tempInt ) )
			g.reyes.bucketSize.x = g.reyes.bucketSize.y = tempInt;

		getIntAttribute( xNode, "gridsize", ( int& )g.reyes.gridSize );

		getStringAttribute( xNode, "bucketorder", g.reyes.bucketorder );

		if( getIntAttribute( xNode, "texturememory", ( int& )g.reyes.textureMemory ) )
			g.reyes.textureMemory *= 1024;

		getFloatAttribute( xNode, "opacitythreshold", g.reyes.opacityThreshold );

		getFloatAttribute( xNode, "motionfactor", g.reyes.motionFactor );
		getFloatAttribute( xNode, "focusfactor", g.reyes.focusFactor );

		getBoolAttribute( xNode, "extrememotiondepthoffield", g.reyes.extremeMotionDepthOfField );

		getIntAttribute( xNode, "eyesplits", ( int& )g.reyes.eyeSplits );

		getFloatAttribute( xNode, "shadingrate", g.shading.rate );

		getBoolAttribute( xNode, "smoothshading", g.shading.smooth );

		getIntAttribute( xNode, "curvewidthshading", ( int& )g.shading.hair );

		//   <sampling> tag
		xNode = xRManNode.getChildNode( "sampling" );

		getIntAttribute( xNode, "samplesx", ( int& )g.sampling.x );
		getIntAttribute( xNode, "samplesy", ( int& )g.sampling.y );

		if( getIntAttribute( xNode, "samples", tempInt ) )
			g.sampling.x = g.sampling.y = tempInt;

		getBoolAttribute( xNode, "jitter", g.reyes.jitter );

		getBoolAttribute( xNode, "samplemotion", g.reyes.sampleMotion );

		//   <path> tag
		debugMessage( L"Parsing <path> tag" );
		xNode = xRManNode.getChildNode( "path" );

		getStringAttribute( xNode, "basename", g.name.baseName );

		if( getStringAttribute( xNode, "map", tempString ) )
			g.directories.map = tempString;

		tempString.clear();
		if( !getStringAttribute( xNode, "data", tempString ) ) {
			getStringAttribute( xNode, "rib", tempString );
		}

		if( !tempString.empty() ) {
			g.directories.map					=
			g.directories.data					=
			g.directories.object				=
			g.directories.attribute				=
			g.directories.base					= tempString;
		}

		if( !getStringAttribute( xNode, "base", tempString ) ) {
			if( getStringAttribute( xNode, "basepath", tempString ) )
				g.directories.base = tempString;
		} else {
			g.directories.base = tempString;
		}

		if( getStringAttribute( xNode, "hub", tempString ) )
			g.directories.hub = tempString;

		if( getStringAttribute( xNode, "image", tempString ) )
			g.directories.image = tempString;

		if( getStringAttribute( xNode, "job", tempString ) )
			g.directories.temp = tempString;

		if( getStringAttribute( xNode, "cache", tempString ) )
			g.directories.cache = tempString;

		getBoolAttribute( xNode, "createmissingdirectories", g.directories.createMissing );

		debugMessage( L"Parsing <cache> tag" );
		//   <cache> tag
		xNode = xRManNode.getChildNode( "cache" );

		getBoolAttribute( xNode, "writedata", g.directories.caching.dataWrite );
		getBoolAttribute( xNode, "sourcedata", g.directories.caching.dataSource );
		getBoolAttribute( xNode, "copydata", g.directories.caching.dataCopy );
		getBoolAttribute( xNode, "writemap", g.directories.caching.mapWrite );
		getBoolAttribute( xNode, "sourcemap", g.directories.caching.mapSource );
		getBoolAttribute( xNode, "copymap", g.directories.caching.mapCopy );
		getBoolAttribute( xNode, "writeimage", g.directories.caching.imageWrite );
		getBoolAttribute( xNode, "copyimage", g.directories.caching.imageCopy );

		int cacheSize;
		if( getIntAttribute( xNode, "size", cacheSize ) )
			g.directories.caching.size = 1000 * 1000 * 1000 * ( unsigned long )cacheSize;

		sanitizePaths();

		debugMessage( L"Parsing <searchpath> tag" );
		//   <searchpath> tag
		xNode = xRManNode.getChildNode( "searchpath" );

		getStringAttribute( xNode, "shader", g.searchPath.shader );

		getStringAttribute( xNode, "texture", g.searchPath.texture );

		getStringAttribute( xNode, "procedural", g.searchPath.procedural );

		getStringAttribute( xNode, "archive", g.searchPath.archive );

		debugMessage( L"Parsing <granularity> tag" );
		//   <granularity> tag
		xNode = xRManNode.getChildNode( "granularity" );

		if( getStringAttribute( xNode, "type", tempString ) ) {
			to_lower( tempString );
			if( "subframe" == tempString )
				g.data.granularity	= data::granularitySubFrame;
			else
			if( "sections" == tempString )
				g.data.granularity	= data::granularitySections;
			else
			if( "objects" == tempString )
				g.data.granularity	= data::granularityObjects;
			else
			if( "attributes" == tempString )
				g.data.granularity	= data::granularityAttributes;
		}

		getBoolAttribute( xNode, "frame", g.data.frame );

		getBoolAttribute( xNode, "shadows", g.data.shadow.shadow );

		getBoolAttribute( xNode, "options", g.data.sections.options );

		getBoolAttribute( xNode, "camera", g.data.sections.camera );

		getBoolAttribute( xNode, "spaces", g.data.sections.spaces );

		getBoolAttribute( xNode, "lights", g.data.sections.lights );

		getBoolAttribute( xNode, "looks",	g.data.sections.looks );

		getBoolAttribute( xNode, "objects", g.data.sections.geometry );

		getBoolAttribute( xNode, "attributes", g.data.sections.attributes );

		getBoolAttribute( xNode, "shaderparameters", g.data.sections.shaderParameters );

		// <rays> tag
		xNode = xRManNode.getChildNode( "rays" );

		bool raysEnable;
		if( getBoolAttribute( xNode, "enable", raysEnable ) ) {
			g.rays.enable = raysEnable;
		}

		debugMessage( L"Parsing <verbosity> tag" );
		// <verbosity> tag
		xNode = xRManNode.getChildNode( "verbosity" );

		if( getIntAttribute( xNode, "level", tempInt ) ) {
			g.feedback.verbosity = static_cast< feedback::verbosityType >( tempInt );
			//feedback::verbosityNone;
		//	if( tempInt )
		//		g.feedback.verbosity = static_cast< feedback::verbosityType >( tempInt );
		}
		//message( L"Parsing <passes> tag", messageError );

		debugMessage( L"Parsing <passes> tag" );
		// <passes> tag
		xNode = xMainNode.getChildNode( "passes" );
		int node = 0;

		debugMessage( L"Parsing <pass> tag" );
		XMLNode xPassNode = xNode.getChildNode( "pass", node++ );

		while( !xPassNode.isEmpty() ) {

			filesystem::path fileName;
			try {
				if( !getStringAttribute( xPassNode, "output", tempString ) ) {
					throw( "bang" );
				} else {
					fileName = tempString; // Throws is path is ill-formed
				}
			} catch( filesystem::filesystem_error a ) {
				message( L"Ill-formed or missing output location found under <pass> tag; skipping current pass. (\nError was: '" + charToCString( a.what() ) + L"'.", messageWarning );
				xPassNode = xNode.getChildNode( "pass", node++ );
				continue;
			}

			if( !fileName.is_complete() )
				fileName = g.directories.image / fileName;

			string name( "untitled" );
			getStringAttribute( xPassNode, "name", name );

			string format( "exr" );
			if( getStringAttribute( xPassNode, "format", tempString ) ) {
				to_lower( tempString );
				if( "tif" == tempString )
					format = "tiff";
				else if( "cin" == tempString )
					format = "cineon";
				else if( "rad" == tempString )
					format = "radiance";
				else
					format = tempString;
			}

			tokenValue::parameterType type = tokenValue::typeFloat;
			if( getStringAttribute( xPassNode, "type", tempString ) ) {
				to_lower( tempString );
				if( "float" == tempString )
					type = tokenValue::typeFloat;
				else if( "color" == tempString )
					type = tokenValue::typeColor;
				else if( "point" == tempString )
					type = tokenValue::typePoint;
				else if( "vector" == tempString )
					type = tokenValue::typeVector;
				else if( "normal" == tempString )
					type = tokenValue::typeNormal;
				else if( "matrix" == tempString )
					type = tokenValue::typeMatrix;
				else
					type = tokenValue::typeUndefined;
			}

			string filter( "catmull-rom" );
			getStringAttribute( xPassNode, "filter", filter );

			float filterSize( 4 );
			getFloatAttribute( xPassNode, "filtersize", filterSize );

			bool computeAlpha( false );
			getBoolAttribute( xPassNode, "computealpha", computeAlpha );

			bool autoCrop;
			if( computeAlpha ) {
				autoCrop = false;
			} else {
				autoCrop = true;
				getBoolAttribute( xPassNode, "autocrop", autoCrop ); // DataWindow != ScreenWindow makes Shake 4 crash if the EXR has more than 3 channels
			}

			bool associateAlpha( true );
			getBoolAttribute( xPassNode, "associatealpha", associateAlpha );

			bool exclusive( true );
			getBoolAttribute( xPassNode, "exclusive", exclusive );

			bool matte( true );
			getBoolAttribute( xPassNode, "matte", matte );

			pass::lineOrderType lineOrder = pass::decreasing;
			if( getStringAttribute( xPassNode, "lineorder", tempString ) ) {
				if( "increasing" == tempString )
					lineOrder = pass::increasing;
			}

			pass::quantizeType quantize;
			if( getStringAttribute( xPassNode, "quantize", tempString ) ) {
				to_lower( tempString );
				if( "8" == tempString ) {
					quantize = pass::quantize8;
				} else if( "16" == tempString ) {
					quantize = pass::quantize16;
				} else if( ( "float" == tempString ) || ( "0" == tempString ) ) {
					quantize = pass::quantizeFloat;
				} else if( "16wp1k" == tempString ) {
					quantize = pass::quantize16wp1k;
				} else if( "16wp2k" == tempString ) {
					quantize = pass::quantize16wp2k;
				} else if( "16wp4k" == tempString ) {
					quantize = pass::quantize16wp4k;
				}
			}

			float dither( -1 );
			bool tempBool;
			if( getBoolAttribute( xPassNode, "dither", tempBool ) ) {
				dither = tempBool ? 0.0f : 0.5f;
			}

			float filterSizeArray[ 2 ] = { filterSize, filterSize };
			g.passPtrArray.push_back( shared_ptr< pass >( new pass(	fileName,
																			name,
																			format,
																			type,
																			filter,
																			filterSizeArray,
																			computeAlpha,
																			associateAlpha,
																			exclusive,
																			matte,
																			quantize,
																			dither,
																			autoCrop,
																			lineOrder ) ) );
			message( L"Adding pass: '" + stringToCString( fileName.native_file_string() ) + L"'", messageInfo );

			xPassNode = xNode.getChildNode( "pass", node++ );
		}
	}

	void globals::set( const string &xmlFile ) {

		globals& g( const_cast< globals& >( access() ) );

		Application app;
		// When fed from an XML file in batch mode,
		// we assume we're doing RIBgen only by default!
		if( !app.IsInteractive() ) {
			g.time.doAnimation				= true;
			g.feedback.verbosity			= feedback::verbosityWarningsAndErrors;
			// Switch off the default framebuffer preview display
			g.feedback.previewDisplay		= feedback::previewNone;
			g.jobGlobal.launch				= jobGlobal::launchOff;
			g.jobGlobal.launchSub			= false;

			g.camera.frontPlane				= false;
			g.camera.backPlane				= false;
		}

		g.passPtrArray.clear();

		scan( xmlFile );

		sanitize();
	}

	bool globals::nextTime() {
		globals& g( const_cast< globals& >( access() ) );
		if( g.time.timeIndex < g.animation.times.size() - 1 ) {
			g.animation.time = g.animation.times[ ++g.time.timeIndex ];
			//char out[ 256 ];
			//sprintf( out, "%04d", ( int )floor( g.animation.time ) );
			//g.name.currentFrame = out;
			// bloody boost format gives me linking errors when I use it in this cpp file!!! :(
			g.name.currentFrame = ( format( "%04d" ) % ( int )floor( g.animation.time ) ).str();
			return true;
		}
		return false;
	}

	float globals::getNormalizedTime() const {
		const globals& g( access() );
		if( 1 < g.animation.times.size() )
			return ( float )( g.time.timeIndex / ( g.animation.times.size() - 1.0 ) );
		return 0.0f;
	}

	void globals::resetTime() {
		globals& g( const_cast< globals& >( access() ) );
		g.time.timeIndex = 0;
		g.animation.time = g.animation.times[ g.time.timeIndex ];
	}

	void globals::aquire( const Property &affogatoGlobals ) {

		debugMessage( L"Aquiring globals from property" );

		globals& g( const_cast< globals& >( access() ) );

		string version( CStringToString( affogatoGlobals.GetParameterValue( L"___AffogatoVersion" ) ) );
		if( AFFOGATOVERSION != version ) {
			message( stringToCString( "Globals where created with a different Affogato version (" + version + ")" ), messageWarning );
		}

		// Camera
		g.camera.cameraName					= CStringToString( affogatoGlobals.GetParameterValue( L"CameraName" ) );
		g.camera.depthOfField				= ( bool )affogatoGlobals.GetParameterValue( L"DepthOfField" );
		g.camera.fStop						= ( float )affogatoGlobals.GetParameterValue( L"FStop" );
		g.camera.focalDistance				= ( float )affogatoGlobals.GetParameterValue( L"FocalDistance" );
		g.camera.hypeOverscan				= ( bool )affogatoGlobals.GetParameterValue( L"HypeOverscan" );
		g.camera.frontPlane					= ( bool )affogatoGlobals.GetParameterValue( L"FrontPlane" );
		g.camera.backPlane					= ( bool )affogatoGlobals.GetParameterValue( L"BackPlane" );
		g.camera.useAspect					= ( bool )affogatoGlobals.GetParameterValue( L"UseCameraAspect" );
		g.camera.freezeScale				= ( bool )affogatoGlobals.GetParameterValue( L"CameraFreezeScale" );
		g.camera.rotoViewStyle				= static_cast< camera::rotoViewStyleType >( ( unsigned long )affogatoGlobals.GetParameterValue( L"CameraRotoViewStyle" ) );

		// Resolution
		g.resolution.x						= ( unsigned long )affogatoGlobals.GetParameterValue( L"ResolutionX" );
		g.resolution.y						= ( unsigned long )affogatoGlobals.GetParameterValue( L"ResolutionY" );
		g.resolution.pixelAspect			= ( float )affogatoGlobals.GetParameterValue( L"PixelAspect" );
		g.resolution.multiplier				= ( float )affogatoGlobals.GetParameterValue( L"ResolutionMultiplier" );

		g.time.frameOutput					= static_cast< time::frameOutputType >( ( unsigned long )affogatoGlobals.GetParameterValue( L"FrameOutput" ) );
		g.time.startFrame					= ( float )affogatoGlobals.GetParameterValue( L"StartFrame" );
		g.time.endFrame						= ( float )affogatoGlobals.GetParameterValue( L"EndFrame" );
		g.time.frameStep					= ( float )affogatoGlobals.GetParameterValue( L"FrameStep" );
		g.time.sequence						= CStringToString( affogatoGlobals.GetParameterValue( L"Frames" ) );

		g.motionBlur.shutterEfficiency		= ( float )affogatoGlobals.GetParameterValue( L"ShutterEfficiency" );

		typedef enum shutterTimingType {
			shutterOpenOnFrame = 0,
			shutterCenterOnFrame = 1,
			shutterCenterBetweenFrames = 2,
			shutterCloseOnNextFrame = 3
		} shutterTimingType;
		shutterTimingType shutterTiming;
		shutterTiming						= static_cast< shutterTimingType >( ( unsigned long )affogatoGlobals.GetParameterValue( L"ShutterTiming", CValue::siUInt1 ) );

		float shutter						= ( float )affogatoGlobals.GetParameterValue( L"ShutterAngle" ) / 360.0;
		switch( shutterTiming ) {
			case shutterCenterOnFrame: {
				g.motionBlur.shutterOpen  = -0.5f * shutter;
				g.motionBlur.shutterClose =  0.5f * shutter;
				break;
			}
			case shutterCenterBetweenFrames: {
				g.motionBlur.shutterOpen  = 0.5f - 0.5f * shutter;
				g.motionBlur.shutterClose = 0.5f + 0.5f * shutter;
				break;
			}
			case shutterCloseOnNextFrame: {
				g.motionBlur.shutterOpen  = 1.0f - shutter;
				g.motionBlur.shutterClose = 1.0f;
				break;
			}
			case shutterOpenOnFrame: {
				g.motionBlur.shutterOpen  = 0.0f;
				g.motionBlur.shutterClose = shutter;
				break;
			}
		}

		g.motionBlur.shutterConfiguration	= static_cast< motionBlur::shutterConfigurationType >( ( unsigned long )affogatoGlobals.GetParameterValue( L"ShutterConfiguration", CValue::siUInt1 ) );

		g.motionBlur.shutterOffset			= ( float )affogatoGlobals.GetParameterValue( L"ShutterOffset" );

		g.motionBlur.transformMotionSamples = ( unsigned long )affogatoGlobals.GetParameterValue( L"TransformationMotionSegments" ) + 1;
		g.motionBlur.deformMotionSamples	= ( unsigned long )affogatoGlobals.GetParameterValue( L"DeformationMotionSegments" ) + 1;

		g.motionBlur.geometryBlur			= ( bool )affogatoGlobals.GetParameterValue( L"GeometryMotionBlur" );
		g.motionBlur.geometryParameterBlur	= ( bool )affogatoGlobals.GetParameterValue( L"GeometryParameterMotionBlur" );
		g.motionBlur.geometryVariableBlur	= ( bool )affogatoGlobals.GetParameterValue( L"GeometryVariableMotionBlur" );
		g.motionBlur.cameraBlur				= ( bool )affogatoGlobals.GetParameterValue( L"CameraMotionBlur" );
		g.motionBlur.lightBlur				= ( bool )affogatoGlobals.GetParameterValue( L"LightMotionBlur" );
		g.motionBlur.attributeBlur			= ( bool )affogatoGlobals.GetParameterValue( L"AttributeMotionBlur" );
		g.motionBlur.shaderBlur				= ( bool )affogatoGlobals.GetParameterValue( L"ShaderMotionBlur" );
		g.motionBlur.shadowMapBlur			= ( bool )affogatoGlobals.GetParameterValue( L"ShadowMapMotionBlur" );
		g.motionBlur.subFrame				= ( bool )affogatoGlobals.GetParameterValue( L"SubFrameMotionBlur" );

		g.name.baseName						= CStringToString( affogatoGlobals.GetParameterValue( L"BaseName" ) );
		g.name.blockName					= CStringToString( affogatoGlobals.GetParameterValue( L"JobBlockName" ) );

		g.directories.createMissing			= ( bool )affogatoGlobals.GetParameterValue( L"CreateDirs" );
		g.directories.relativePaths			= ( bool )affogatoGlobals.GetParameterValue( L"RelativePaths" );

		try {
			g.directories.base				= filesystem::path( CStringToString( affogatoGlobals.GetParameterValue( L"BaseDir" ) ) );
		} catch( filesystem::filesystem_error ) {
			message( L"The base path in the globals is ill-formed and will be ignored. ", messageWarning );
		}

		try {
			g.directories.cache				= filesystem::path( CStringToString( affogatoGlobals.GetParameterValue( L"CacheDir" ) ) );
		} catch( filesystem::filesystem_error ) {
			message( L"The cache path in the globals is ill-formed and will be ignored. ", messageWarning );
		}

		g.directories.caching.dataWrite		= ( bool )affogatoGlobals.GetParameterValue( L"CacheWriteData" );
		g.directories.caching.dataSource	= ( bool )affogatoGlobals.GetParameterValue( L"CacheSourceData" );
		g.directories.caching.dataCopy		= ( bool )affogatoGlobals.GetParameterValue( L"CacheCopyData" );
		g.directories.caching.mapWrite		= ( bool )affogatoGlobals.GetParameterValue( L"CacheWriteMap" );
		g.directories.caching.mapSource		= ( bool )affogatoGlobals.GetParameterValue( L"CacheSourceMap" );
		g.directories.caching.mapCopy		= ( bool )affogatoGlobals.GetParameterValue( L"CacheCopyMap" );
		g.directories.caching.imageWrite	= ( bool )affogatoGlobals.GetParameterValue( L"CacheWriteImage" );
		g.directories.caching.imageCopy		= ( bool )affogatoGlobals.GetParameterValue( L"CacheCopyImage" );
		g.directories.caching.size			= 1000 * 1000 * 1000 * ( unsigned long )affogatoGlobals.GetParameterValue( L"CacheSize" );


		try {
			//g.directories.imageDestination =
			g.directories.image				= filesystem::path( CStringToString( affogatoGlobals.GetParameterValue( L"ImageDir" ) ) );
		} catch( filesystem::filesystem_error ) {
			message( L"The image path in the globals is ill-formed and will be ignored. ", messageWarning );
		}

		try {
			//g.directories.mapDestination	=
			//g.directories.mapSource		=
			g.directories.map				= filesystem::path( CStringToString( affogatoGlobals.GetParameterValue( L"MapDir" ) ) );
		} catch( filesystem::filesystem_error ) {
			message( L"The map/texture path in the globals is ill-formed and will be ignored. ", messageWarning );
		}

		try {
			//g.directories.dataSource		=
			//g.directories.dataDestination	=
			g.directories.data				= filesystem::path( CStringToString( affogatoGlobals.GetParameterValue( L"DataDir" ) ) );
		} catch( filesystem::filesystem_error ) {
			message( L"The data path in the globals is ill-formed and will be ignored. ", messageWarning );
		}

		try {
			//g.directories.objectSource	=
			//g.directories.objectDestination =
			g.directories.object			= filesystem::path( CStringToString( affogatoGlobals.GetParameterValue( L"ObjectDataDir" ) ) );
		} catch( filesystem::filesystem_error ) {
			message( L"The object path in the globals is ill-formed and will be ignored. ", messageWarning );
		}
		try {
			//g.directories.attributeSource	=
			//g.directories.attributeDestination =
			g.directories.attribute			= filesystem::path( CStringToString( affogatoGlobals.GetParameterValue( L"AttributeDataDir" ) ) );
		} catch( filesystem::filesystem_error ) {
			message( L"The atribute path in the globals is ill-formed and will be ignored. ", messageWarning );
		}

		try {
			g.directories.temp				= filesystem::path( CStringToString( affogatoGlobals.GetParameterValue( L"TempDir" ) ) );
		} catch( filesystem::filesystem_error ) {
			message( L"The temp path in the globals is ill-formed and will be ignored. ", messageWarning );
		}

		try {
			g.directories.hub				= filesystem::path( CStringToString( affogatoGlobals.GetParameterValue( L"HubDir" ) ) );
		} catch( filesystem::filesystem_error ) {
			message( L"The hub path in the globals is ill-formed and will be ignored. ", messageWarning );
		}

		g.searchPath.shader					= CStringToString( affogatoGlobals.GetParameterValue( L"ShaderPath" ) );
		g.searchPath.texture				= CStringToString( affogatoGlobals.GetParameterValue( L"TexturePath" ) );
		g.searchPath.archive				= CStringToString( affogatoGlobals.GetParameterValue( L"ArchivePath" ) );
		g.searchPath.procedural				= CStringToString( affogatoGlobals.GetParameterValue( L"ProceduralPath" ) );

		g.searchPath.shaderPaths			= ( bool )affogatoGlobals.GetParameterValue( L"ShaderPaths" );

		g.sampling.x						= ( unsigned short )affogatoGlobals.GetParameterValue( L"PixelSamplesX" );
		g.sampling.y						= ( unsigned short )affogatoGlobals.GetParameterValue( L"PixelSamplesY" );
		g.shading.rate						= ( float )affogatoGlobals.GetParameterValue( L"ShadingRate" );
		g.shading.smooth					= ( bool )affogatoGlobals.GetParameterValue( L"SmoothShading" );
		g.shading.hair						= ( unsigned short )affogatoGlobals.GetParameterValue( L"CurveWidthShading" );
		g.shading.shadowRate				= ( float )affogatoGlobals.GetParameterValue( L"ShadowShadingRate" );

		g.filtering.x						= ( float )affogatoGlobals.GetParameterValue( L"PixelFilterX" );
		g.filtering.y						= ( float )affogatoGlobals.GetParameterValue( L"PixelFilterY" );
		g.filtering.filter					= CStringToString( affogatoGlobals.GetParameterValue( L"PixelFilter" ) );

		g.image.displayDriver				= CStringToString( affogatoGlobals.GetParameterValue( L"DisplayDriver" ) );
		g.image.displayQuantization			= static_cast< pass::quantizeType >( ( unsigned long )affogatoGlobals.GetParameterValue( L"DisplayQuantization" ) );
		g.image.associateAlpha				= ( bool )affogatoGlobals.GetParameterValue( L"AssociateAlpha" );
		g.image.gain						= ( float )affogatoGlobals.GetParameterValue( L"Gain" );
		g.image.gamma						= ( float )affogatoGlobals.GetParameterValue( L"Gamma" );

		g.reyes.jitter						= ( bool )affogatoGlobals.GetParameterValue( L"SampleJitter" );
		g.reyes.sampleMotion				= ( bool )affogatoGlobals.GetParameterValue( L"SampleMotion" );
		g.reyes.bucketSize.x				= ( unsigned long )affogatoGlobals.GetParameterValue( L"BucketX" );
		g.reyes.bucketSize.y				= ( unsigned long )affogatoGlobals.GetParameterValue( L"BucketY" );
		g.reyes.gridSize					= ( unsigned long )affogatoGlobals.GetParameterValue( L"GridSize" );
		g.reyes.bucketorder					= CStringToString( affogatoGlobals.GetParameterValue( L"BucketOrder" ) );
		g.reyes.textureMemory				= 1024 * ( unsigned long )affogatoGlobals.GetParameterValue( L"TextureMemory" );
		g.reyes.opacityThreshold			= ( float )affogatoGlobals.GetParameterValue( L"OpacityThreshold" );
		g.reyes.motionFactor				= ( float )affogatoGlobals.GetParameterValue( L"MotionFactor" );
		g.reyes.focusFactor					= ( float )affogatoGlobals.GetParameterValue( L"FocusFactor" );
		g.reyes.extremeMotionDepthOfField	= ( bool )affogatoGlobals.GetParameterValue( L"ExtremeMotionDepthOfField" );
		g.reyes.eyeSplits					= ( unsigned long )affogatoGlobals.GetParameterValue( L"EyeSplits" );

		g.rays.enable						= ( bool )affogatoGlobals.GetParameterValue( L"RayTracing" );
		g.rays.subSurface.rate				= ( float )affogatoGlobals.GetParameterValue( L"SubSurfaceShadingRate" );
		g.rays.trace.depth					= ( unsigned long )affogatoGlobals.GetParameterValue( L"RayDepth" );
		g.rays.trace.bias					= ( float )affogatoGlobals.GetParameterValue( L"TraceBias" );
		g.rays.trace.motion					= ( bool )affogatoGlobals.GetParameterValue( L"TraceMotion" );
		g.rays.trace.displacements			= ( bool )affogatoGlobals.GetParameterValue( L"TraceDisplacements" );
		g.rays.irradiance.samples			= ( unsigned long )affogatoGlobals.GetParameterValue( L"IrradianceSamples" );
		g.rays.irradiance.shadingRate		= ( float )affogatoGlobals.GetParameterValue( L"IrradianceShadingRate" );

		g.data.directToRenderer				= ( bool )affogatoGlobals.GetParameterValue( L"DirectToRenderer" );

		g.data.granularity					= static_cast< data::granularityType >( ( unsigned long )affogatoGlobals.GetParameterValue( L"DataGranularity" ) );
		g.data.frame						= ( bool )affogatoGlobals.GetParameterValue( L"FrameData" );
		g.data.attributeDataType			= static_cast< data::attributeDataTypeType >( ( unsigned long )affogatoGlobals.GetParameterValue( L"AttributeDataType" ) );
		g.data.subSectionParentTransforms	= ( bool )affogatoGlobals.GetParameterValue( L"TransformsInParentData" );
		g.data.relativeTransforms			= ( bool )affogatoGlobals.GetParameterValue( L"RelativeTransforms" );
		g.data.binary						= ( bool )affogatoGlobals.GetParameterValue( L"WriteBinaryData" );
		g.data.compress						= ( bool )affogatoGlobals.GetParameterValue( L"CompressData" );
		g.data.delay						= ( bool )affogatoGlobals.GetParameterValue( L"DelayData" );
		g.data.doHub						= ( bool )affogatoGlobals.GetParameterValue( L"HubSupport" );
		g.data.sections.options				= ( bool )affogatoGlobals.GetParameterValue( L"OptionsData" );
		g.data.sections.camera				= ( bool )affogatoGlobals.GetParameterValue( L"CameraData" );
		g.data.sections.world				= true; //( bool )affogatoGlobals.GetParameterValue( L"WorldData" );
		g.data.sections.spaces				= ( bool )affogatoGlobals.GetParameterValue( L"SpacesData" );
		g.data.sections.lights				= ( bool )affogatoGlobals.GetParameterValue( L"LightsData" );
		g.data.sections.looks				= ( bool )affogatoGlobals.GetParameterValue( L"LooksData" );
		g.data.sections.geometry			= ( bool )affogatoGlobals.GetParameterValue( L"GeometryData" );
		g.data.sections.attributes			= ( bool )affogatoGlobals.GetParameterValue( L"AttributeData" );
		g.data.sections.shaderParameters	= ( bool )affogatoGlobals.GetParameterValue( L"ShaderParameterData" );
		g.data.sections.shaderNumericParameters	= ( bool )affogatoGlobals.GetParameterValue( L"ShaderNumericParameterData" );
		g.data.hierarchical					= ( bool )affogatoGlobals.GetParameterValue( L"HierarchicalDataScanning" );
		g.data.attributeScanningOrder		= static_cast< data::scanningOrderType >( ( unsigned long )affogatoGlobals.GetParameterValue( L"AttributeDataScanningOrder" ) );

		g.data.shadow.shadow				= ( bool )affogatoGlobals.GetParameterValue( L"ShadowData" );

		g.feedback.previewDisplay			= static_cast< feedback::previewDisplayType >( ( unsigned long )affogatoGlobals.GetParameterValue( L"PreviewDisplay" ) );
		g.feedback.verbosity				= static_cast< feedback::verbosityType >( ( unsigned long )affogatoGlobals.GetParameterValue( L"VerbosityLevel" ) );
		g.feedback.stopWatch				= ( bool )affogatoGlobals.GetParameterValue( L"Stopwatch" );

		g.defaultShader.surface				= CStringToString( affogatoGlobals.GetParameterValue( L"DefaultSurfaceShader" ) );
		g.defaultShader.displacement		= CStringToString( affogatoGlobals.GetParameterValue( L"DefaultDisplacementShader" ) );
		g.defaultShader.volume				= CStringToString( affogatoGlobals.GetParameterValue( L"DefaultVolumeShader" ) );
		g.defaultShader.overrideAll			= ( bool )affogatoGlobals.GetParameterValue( L"OverrideAllShaders" );

		g.baking.bake						= ( bool )affogatoGlobals.GetParameterValue( L"BakeMode" );

		g.geometry.normalizeNurbKnotVector	= ( bool )affogatoGlobals.GetParameterValue( L"NormalizeNurbKnotVector" );
		g.geometry.nonRationalNurbSurface	= ( bool )affogatoGlobals.GetParameterValue( L"NonRationalNurbSurface" );
		g.geometry.nonRationalNurbCurve		= ( bool )affogatoGlobals.GetParameterValue( L"NonRationalNurbCurve" );
		g.geometry.defaultNurbCurveWidth	= ( float )affogatoGlobals.GetParameterValue( L"NurbCurveWidth" );

		g.jobGlobal.launch					= static_cast< jobGlobal::launchType >( ( unsigned long )affogatoGlobals.GetParameterValue( L"LaunchType" ) );
		g.jobGlobal.launchSub				= ( bool )affogatoGlobals.GetParameterValue( L"LaunchSubJobs" );
		g.jobGlobal.jobScript.type			= static_cast< jobGlobal::jobScript::jobScriptType >( ( unsigned long )affogatoGlobals.GetParameterValue( L"JobScriptFormat" ) );
		g.jobGlobal.jobScript.interpreter	= CStringToString( affogatoGlobals.GetParameterValue( L"JobScriptInterpreter" ) );
		g.jobGlobal.jobScript.chunkSize		= ( unsigned long )affogatoGlobals.GetParameterValue( L"JobScriptChunkSize" );
		g.jobGlobal.preJobCommand			= CStringToString( affogatoGlobals.GetParameterValue( L"PreJobCommand" ) );
		g.jobGlobal.preFrameCommand			= CStringToString( affogatoGlobals.GetParameterValue( L"PreFrameCommand" ) );
		g.jobGlobal.postFrameCommand		= CStringToString( affogatoGlobals.GetParameterValue( L"PostFrameCommand" ) );
		g.jobGlobal.postJobCommand			= CStringToString( affogatoGlobals.GetParameterValue( L"PostJobCommand" ) );
		g.jobGlobal.numCPUs					= ( unsigned short )affogatoGlobals.GetParameterValue( L"CPUs" );
		g.jobGlobal.hosts					= CStringToString( affogatoGlobals.GetParameterValue( L"RemoteRenderHosts" ) );
		g.jobGlobal.useRemoteSSH			= ( bool )affogatoGlobals.GetParameterValue( L"UseSSHForRemote" );
		g.jobGlobal.cache					= static_cast< jobGlobal::cacheType >( ( unsigned long )affogatoGlobals.GetParameterValue( L"RenderCache" ) );
		g.jobGlobal.cacheDir				= CStringToString( affogatoGlobals.GetParameterValue( L"RenderCacheDir" ) );
		g.jobGlobal.cacheSize				= 1024 * ( unsigned long )affogatoGlobals.GetParameterValue( L"RenderCacheSize" );

		g.renderer.command					= CStringToString( affogatoGlobals.GetParameterValue( L"RenderCommand" ) );

	}


	void globals::set( const Property &affogatoGlobals ) {
		globals& g( const_cast< globals& >( access() ) );

		debugMessage( L"Setting globals from property" );

		aquire( affogatoGlobals );

		sanitize();

		g.blobbyGroupsMap.clear();
		float filterSizeArray[ 2 ] = { g.filtering.x, g.filtering.y };
		g.passPtrArray.push_back( shared_ptr< pass >( new pass(
				( g.directories.image / g.name.baseName ).string(), // FileName
				string( "rgba" ), // Name
				g.image.displayDriver, // Format
				tokenValue::typeUndefined, // Type
				g.filtering.filter, // Filter
				filterSizeArray, // Filter Size Array
				true, // Compute alpha
				g.image.associateAlpha, // Associate Alpha
				false, // Exclusive
				true, // Matte
				g.image.displayQuantization, // Quantize
				g.image.displayQuantization ? 0.5f : 0.0f, // Dither
				//g.image.gain // Gain
				//g.image.gamma // Gamma
				true, // auto crop
				pass::increasing // line order
				) ) );
	}


	void globals::sanitize() {

		debugMessage( L"Sanitizing globals" );

		globals& g( const_cast< globals& >( access() ) );

		Application app;

		// Affogato
		string affogatoHomePath( getEnvironment( "AFFOGATOHOME" ) );
		try {
			if( !affogatoHomePath.empty() ) {
				g.directories.affogatoHome = filesystem::path( affogatoHomePath );
			} else {
				throw( "bang" );
			}
		} catch ( ... ) {
			message( L"AFFOGATOHOME environment variable is empty/undefined or conatins an ill-formed path.", messageWarning );
		}

		g.animation.times.clear();
		switch( g.time.frameOutput ) {
			case time::frameCurrent: {
				CTime theTime;
				g.animation.times.push_back( ( float )theTime.GetTime() );
				break;
			}
			case time::frameStart: {
				g.animation.times.push_back( g.time.startFrame );
				break;
			}
			case time::frameStartToEnd: {
				//char frameStr[ 40 ];
				//sprintf( frameStr, "%f-%f@%f", g.time.startFrame, g.time.endFrame, g.time.frameStep );
				//g.animation.times = getSequence( string( frameStr ) );
				g.animation.times = getSequence( ( format( "%f-%f@%f" ) % g.time.startFrame % g.time.endFrame % g.time.frameStep ).str() );
				break;
			}
			case time::frameSequence: {
				g.animation.times = getSequence( g.time.sequence );
				break;
			}
		}

		g.time.timeIndex = 0;
		g.animation.time = g.animation.times[ g.time.timeIndex ];
		//g.resetTime();
		//char out[ 256 ];
		//sprintf( out, "%04d", ( int )floor( g.animation.time ) );
		//g.name.currentFrame = out;
		g.name.currentFrame = ( format( "%04d" ) % ( int )floor( g.animation.time ) ).str();

		if( g.animation.times.size() < g.jobGlobal.jobScript.chunkSize )
			g.jobGlobal.jobScript.chunkSize = 0;

		sanitizePaths();

		// Fix searchpaths & add known folders -- just in case
		g.searchPath.shader 		= cleanUpSearchPath( parseString( g.searchPath.shader ) );
		debugMessage( L"Shader path: " + stringToCString( g.searchPath.shader ) );
		//g.searchPath.shader 		= parseString( g.searchPath.shader );
		g.searchPath.shader			= "@:" + g.searchPath.shader + ":" + ( !g.directories.affogatoHome.empty() ? ( g.directories.affogatoHome / "shaders" ).string() : "" ) + ":.";
		debugMessage( L"Shader path: " + stringToCString( g.searchPath.shader ) );

		g.searchPath.texture 		= cleanUpSearchPath( parseString( g.searchPath.texture ) );
		debugMessage( L"Texture path: " + stringToCString( g.searchPath.texture ) );
		//g.searchPath.texture 		= parseString( g.searchPath.texture );
		g.searchPath.texture		= "@:" + g.searchPath.texture + ":.";
		debugMessage( L"Texture path: " + stringToCString( g.searchPath.texture ) );

		g.searchPath.archive 		= cleanUpSearchPath( parseString( g.searchPath.archive ) );
		debugMessage( L"Archive path: " + stringToCString( g.searchPath.archive ) );
		//g.searchPath.archive 		= parseString( g.searchPath.archive );
		g.searchPath.archive		= "@:" + g.searchPath.archive + ":.";
		debugMessage( L"Archive path: " + stringToCString( g.searchPath.archive ) );

		g.searchPath.procedural 	= cleanUpSearchPath( parseString( g.searchPath.procedural ) );
		debugMessage( L"Procedural path: " + stringToCString( g.searchPath.procedural ) );
		//g.searchPath.procedural 	= parseString( g.searchPath.procedural );
		g.searchPath.procedural		= "@:" + g.searchPath.procedural + ":.";
		debugMessage( L"Procedural path: " + stringToCString( g.searchPath.procedural ) );

		if( g.searchPath.texture.rfind( ":" ) != g.searchPath.texture.length() - 1 )
			g.searchPath.texture += ":";

		if( g.directories.caching.mapSource )
			g.searchPath.texture += g.directories.cache.string();

		if( g.searchPath.archive.rfind( ":" ) != g.searchPath.archive.length() - 1 )
			g.searchPath.archive += ":";

		if( g.directories.caching.dataSource )
			g.searchPath.archive += g.directories.cache.string();
		else
			g.searchPath.archive += g.directories.data.string() + ":" + g.directories.object.string() + ":" + g.directories.attribute.string();

		if( 0 == g.shading.rate )
			g.shading.rate = 1;

		if( 0 == g.shading.shadowRate )
			g.shading.shadowRate = 1;

		if( 0 == g.resolution.multiplier )
			g.resolution.multiplier = 1;

		if( g.renderer.command.empty() )
			g.renderer.command = "renderdl";


		debugMessage( L"Done sanitizing all globals." );
	}


	void globals::sanitizePaths() {
		globals& g( const_cast< globals& >( access() ) );

		if( g.name.baseName.empty() )
			g.name.baseName = "untitled";

		if( !g.name.blockName.empty() )
			if( '.' != g.name.blockName[ 0 ] )
				g.name.blockName = "." + g.name.blockName;

		string env( getEnvironment( "TMP" ) );
		if( env.empty() ) {
			env = getEnvironment( "TEMP" );
			if( env.empty() ) {
				env = getEnvironment( "TEMPDIR" );
				if( env.empty() ) {
#ifndef _WIN32
					env = "/tmp";
#else
					env = "%SystemRoot%/Temp/";
#endif
				}
			}
		}
		try {
			g.system.tempDir = replace_all_copy( env, "//", "/" );
		} catch( ... ) {
			message( L"The system's temp path is ill-formed and will be ignored.", messageWarning );
		}

		if( g.data.directToRenderer ) {
			g.data.granularity = data::granularityFrame;
			g.jobGlobal.launch = jobGlobal::launchOff;
		}

		if( g.directories.base.empty() )
			g.directories.base = ".";
		checkFixCreateDir( g.directories.base, g.directories.base, g.directories.createMissing, g.system.tempDir );

		if( g.directories.cache.empty() )
			g.directories.caching.dataWrite		=
			g.directories.caching.dataSource	=
			g.directories.caching.mapWrite		=
			g.directories.caching.mapSource		=
			g.directories.caching.imageWrite	= false;
		else
			checkFixCreateDir( g.directories.cache, g.directories.cache, g.directories.createMissing, g.system.tempDir );

		if( !g.directories.caching.dataWrite )
			g.directories.caching.dataCopy = false;

		if( !g.directories.caching.mapWrite )
			g.directories.caching.mapCopy = false;

		if( !g.directories.caching.imageWrite )
			g.directories.caching.imageCopy = false;

		// If we're using relative paths, make sure we're in the right folder
		if( g.directories.relativePaths && !g.directories.base.empty() )
#ifdef _WIN32
			_chdir( g.directories.base.string().c_str() );
#else
			chdir( g.directories.base.string().c_str() );
#endif

		//message( L"image", messageError );
		g.directories.image					= makeAbsRelPath( g.directories.base, g.directories.image, g.directories.relativePaths );
		g.directories.image					= checkFixCreateDir( g.directories.base, g.directories.image, g.directories.createMissing, g.system.tempDir );

		//message( L"map", messageError );
		g.directories.map					= makeAbsRelPath( g.directories.base, g.directories.map, g.directories.relativePaths );
		g.directories.map					= checkFixCreateDir( g.directories.base, g.directories.map, g.directories.createMissing, g.system.tempDir  );

		//message( L"data", messageError );
		g.directories.data					= makeAbsRelPath( g.directories.base, g.directories.data, g.directories.relativePaths );
		g.directories.data					= checkFixCreateDir( g.directories.base, g.directories.data, g.directories.createMissing, g.system.tempDir );

		//message( L"object", messageError );
		g.directories.object				= makeAbsRelPath( g.directories.base, g.directories.object, g.directories.relativePaths );
		g.directories.object				= checkFixCreateDir( g.directories.base, g.directories.object, g.directories.createMissing, g.system.tempDir );

		//message( L"attribute", messageError );
		g.directories.attribute				= makeAbsRelPath( g.directories.base, g.directories.attribute, g.directories.relativePaths );
		g.directories.attribute				= checkFixCreateDir( g.directories.base, g.directories.attribute, g.directories.createMissing, g.system.tempDir );

		//message( L"temp", messageError );
		g.directories.temp					= makeAbsRelPath( g.directories.base, g.directories.temp, g.directories.relativePaths );
		g.directories.temp					= checkFixCreateDir( g.directories.base, g.directories.temp, g.directories.createMissing, g.system.tempDir );

		g.directories.hub					= makeAbsRelPath( g.directories.base, g.directories.hub, g.directories.relativePaths );
		g.directories.hub 					= checkFixCreateDir( g.directories.base, g.directories.hub, g.directories.createMissing, g.system.tempDir );
	}

	globals& globals::access() {
		// Define singleton instance
		static globals theGlobals;
		return theGlobals;
	}

	void globals::writePasses() const {

		debugMessage( L"Writing passes" );
		for( vector< shared_ptr< pass > >::const_iterator it = passPtrArray.begin(); it < passPtrArray.end(); it++ ) {
			( *it )->write();
		}
	}

	vector< filesystem::path > globals::getPassNames() const {
		vector< filesystem::path > passNames;
		for( vector< shared_ptr< pass > >::const_iterator it = passPtrArray.begin(); it < passPtrArray.end(); it++ ) {
			passNames.push_back( ( *it )->getFileName() );
		}
		return passNames;
	}

	void globals::writePassesXML( const string &destination ) const {
		ofstream outXML( destination.c_str() );
		outXML << getPassesXML();
	}

	string globals::getPassesXML() const {
		stringstream out;
		for( vector< shared_ptr< pass > >::const_iterator it = passPtrArray.begin(); it < passPtrArray.end(); it++ )
			out << ( *it )->getXML();
		return out.str();
	}
}

