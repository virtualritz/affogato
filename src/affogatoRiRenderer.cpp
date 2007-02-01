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
#include <string>

// Boost headers
#include <boost/algorithm/string/erase.hpp>

// XSI headers
#ifdef __XSI_PLUGIN
	#include <xsi_application.h>
#else
	#include <stdio.h>
#endif

// RenderMan headers
#include <ri.h>
#include <rx.h>

// Affogato headers
#include "affogatoRiRenderer.hpp"
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

	ueberManRiRenderer::ueberManRiRenderer()
	:	currentContext( 0 ),
		contextCounter( 0 ),
		dso( false )
	{
		debugMessage( L"Constructing RiRenderer" );
	}

	ueberManRiRenderer::~ueberManRiRenderer() {
		debugMessage( L"Destructing RiRenderer" );
	}
	/**
	 * Starts a new scene.
	 * It returns a context handle that can be used to switch to direct calls
	 * to that scene specifically later.
	 * For each context, a state instance is maintained. This ensures that the
	 * whole state can be restored, regardless when the context gets switched
	 */
	context ueberManRiRenderer::beginScene( const string& destination, bool useBinary, bool useCompression ) {
		if( !dso ) { // if DSO is treu we already started a new scene
			if( "dynamicload" != destination ) { // this check allows to use the API for DSOs too, where no RiBegin() is ever called
				if( "direct" == destination ) {
					RtInt numThreads = 2;
					RiOption( "render", "nthreads", &numThreads, RI_NULL );
					debugMessage( L"UeberManRi: BeginScene [direct]" );
					RiBegin( NULL );
					RiOption( "render", "nthreads", &numThreads, RI_NULL );
				} else {
					debugMessage( L"UeberManRi: BeginScene" );
					debugMessage( L"UeberManRi: BeginScene0 " + stringToCString( destination ) );
					RiBegin( const_cast< RtToken >( ( destination + ".rib" ).c_str() ) );
					debugMessage( L"UeberManRi: BeginScene0.0" );
					if( useBinary ) {
						RtString format = "binary";
						RiOption( "rib", "format", &format, RI_NULL );
					}
					debugMessage( L"UeberManRi: BeginScene0.1" );
					if( useCompression ) {
						RtString compress = "gzip";
						RiOption( "rib", "compression", &compress, RI_NULL );
					}
					debugMessage( L"UeberManRi: BeginScene0.2" );
				}
				dso = false;
			} else {
				dso = true;
			}
		}
		++contextCounter;
		currentContext = contextCounter;
		debugMessage( L"UeberManRi: BeginScene1" );
		stateMachine[ currentContext ] = boost::shared_ptr< state >( new state );
		debugMessage( L"UeberManRi: Using Context " + CValue( currentContext ).GetAsText() );
		currentState = stateMachine[ currentContext ].get();
		debugMessage( L"UeberManRi: Got RMan context " + CValue( currentState->renderContext ).GetAsText() );
		return currentContext;
	}

	void ueberManRiRenderer::switchScene( context ctx ) {
		debugMessage( L"UeberManRi: SwitchScene [" + CValue( ctx ).GetAsText()+ L"]" );
		if( !dso ) {
			debugMessage( L"UeberManRi: Got RMan context " + CValue( stateMachine[ ctx ]->renderContext ).GetAsText() );
			RiContext( stateMachine[ ctx ]->renderContext );
		}
		currentContext = ctx;
		currentState = stateMachine[ currentContext ].get();
	}

	void ueberManRiRenderer::endScene( context ctx ) {

		//map< context, boost::shared_ptr< state > >::iterator where = stateMachine.find( ctx );
		//if( stateMachine.begin() != where )
		//	currentContext = ( --where )->first; // get the element before the current
		//else
		//	currentContext = stateMachine.begin()->first;

		if( !dso ) {
			if(	currentState->inWorldBlock ) {
				currentState->inWorldBlock = false;
				debugMessage( L"UeberManRi: WorldEnd" );
				RiWorldEnd();
			}

			debugMessage( L"UeberManRi: Ending RMan context " + CValue( stateMachine[ ctx ]->renderContext ).GetAsText() );
			RiContext( stateMachine[ ctx ]->renderContext );
			RiEnd();
		}
		stateMachine.erase( ctx ); // free the memory but keep the array size
		if( !stateMachine.empty() )
			currentState = stateMachine.rbegin()->second.get();

		debugMessage( L"UeberManRi: Number of contexts left: " + CValue( ( long )stateMachine.size() ).GetAsText() );
	}

	/*const ueberManRiRenderer* ueberManRiRenderer::accessRendererStatic() {
		debugMessage( L"UeberManRi: accessRendererStatic" );
		static ueberManRiRenderer theRenderer;
		return &theRenderer;
	}*/

	const ueberManRiRenderer& ueberManRiRenderer::accessRenderer() {
		debugMessage( L"UeberManRi: accessRenderer" );
		static ueberManRiRenderer theRenderer;
		theRenderer.currentContext = 0;
		theRenderer.contextCounter = 0;
		theRenderer.dso = false;
		return theRenderer;
	}

	void ueberManRiRenderer::input( const string& filename ) {
		debugMessage( L"UeberManRi: Input" );

		string::size_type pos = filename.find( " " );
		if( pos == string::npos ) {
			string arg0 = filename + ".rib";
			RiReadArchive( const_cast< RtString >( arg0.c_str() ), NULL, RI_NULL );

		} else {
			RtString args[ 2 ];
			string arg0 = filename.substr( 0, pos );
			args[ 0 ] = const_cast< RtString >( arg0.c_str() );
			string arg1 = filename.substr( pos + 1 );
			args[ 1 ] = const_cast< RtString >( arg1.c_str() );
			float bound[] = { -1e38f, 1e38f, -1e38f, 1e38f, -1e38f, 1e38f };
			RiProcedural( ( RtPointer )args, const_cast< RtFloat* >( bound ), RiProcDynamicLoad, NULL );

		}
	}

	void ueberManRiRenderer::input( const string& filename, const float bound[ 6 ] ) {
		debugMessage( L"UeberManRi: Input [bounded]" );

		string::size_type pos = filename.find( " " );
		if( pos == string::npos ) {
			RtString args[ 1 ];
			string arg0 = filename + ".rib";
			args[ 0 ] = const_cast< RtString >( arg0.c_str() );
			RiProcedural( ( RtPointer )args, const_cast< RtFloat* >( bound ), RiProcDelayedReadArchive, NULL );
		} else {
			RtString args[ 2 ];
			string arg0 = filename.substr( 0, pos );
			args[ 0 ] = const_cast< RtString >( arg0.c_str() );
			string arg1 = filename.substr( pos + 1 );
			args[ 1 ] = const_cast< RtString >( arg1.c_str() );
			RiProcedural( ( RtPointer )args, const_cast< RtFloat* >( bound ), RiProcDynamicLoad, NULL );
		}
	}

	void ueberManRiRenderer::camera( cameraHandle& cameraid ) {
		debugMessage( L"UeberManRi: Camera" );

		RtString projection = "perspective";

		RtFloat fov = 90.0f;

		RtFloat screen[ 4 ] = { 0, 0, 0, 0 };

		RtFloat near = 0.1f;
		RtFloat far  = 1e6f;

		RtInt resolution[ 2 ] = { 640, 480 };
		RtFloat pixelaspect = 1.0f;

		RtFloat crop[ 4 ] = { 0.0f, 1.0f, 0.0f, 1.0f };

		RtFloat shutter[ 2 ] = { 0.0f, 0.0f };
		RtFloat shutterOffset = 0.0f;
		RtFloat shutterEfficiency[ 2 ] = { 1.0f, 1.0f };

		RtFloat fstop = RI_INFINITY;
		RtFloat focallength = 0.0f;
		RtFloat focaldistance = 0.0f;

		RtFloat pixelsamples[ 2 ] = { 3.0, 3.0 };

		RtString bucketorder = NULL;
		RtInt bucketsize[ 2 ] = { 16, 16 };
		RtInt gridsize = 256;
		RtInt texturememory = 8192;

		RtString hider = NULL;

		RtInt sampleMotion = 0;

		RtInt hiderjitter = 1;

		RtInt extremeMotionDof = 0;

#ifdef DELIGHT
		RtInt eyeSplits = 6;
#else
		RtInt eyeSplits = 10;
#endif

		RtString hiderdepthfilter = NULL;

		debugMessage( L"UeberManRi: Interpreting tokens" );

		for( vector< tokenValue::tokenValuePtr >::iterator it = currentState->tokenValueCache.begin(); it < currentState->tokenValueCache.end(); it++ ) {
			string name( ( *it )->name() );
			boost::erase_all( name, " " );
			if( "projection" == name ) {
				projection = const_cast< RtString >( ( char* )( *it )->data() );
			} else if( "fov" == name ) {
				fov = *( ( RtFloat* )( *it )->data() );
			} else if( "screen" == name ) {
				RtFloat *tmp = ( RtFloat* )( *it )->data();
				screen[ 0 ] = tmp[ 0 ];
				screen[ 1 ] = tmp[ 1 ];
				screen[ 2 ] = tmp[ 2 ];
				screen[ 3 ] = tmp[ 3 ];
			} else if( "near" == name ) {
				near = *( ( RtFloat* )( *it )->data() );
			} else if( "far" == name ) {
				far = *( ( RtFloat* )( *it )->data() );
			} else if( "resolution" == name ) {
				RtInt *tmp = ( RtInt* )( *it )->data();
				resolution[ 0 ] = tmp[ 0 ];
				resolution[ 1 ] = tmp[ 1 ];
			} else if( "pixelaspect" == name ) {
				pixelaspect = *( ( RtFloat* )( *it )->data() );
			} else if( "crop" == name ) {
				RtFloat *tmp = ( RtFloat* )( *it )->data();
				crop[ 0 ] = tmp[ 0 ];
				crop[ 1 ] = tmp[ 1 ];
				crop[ 2 ] = tmp[ 2 ];
				crop[ 3 ] = tmp[ 3 ];
			} else if( "shutter" == name ) {
				RtFloat *tmp = ( RtFloat* )( *it )->data();
				shutter[ 0 ] = tmp[ 0 ];
				shutter[ 1 ] = tmp[ 1 ];
			} else if( "shutteroffset" == name ) {
				shutterOffset = *( ( RtFloat* )( *it )->data() );
			} else if( "shutterefficiency" == name ) {
				RtFloat *tmp = ( RtFloat* )( *it )->data();
				shutterEfficiency[ 0 ] = tmp[ 0 ];
				shutterEfficiency[ 1 ] = tmp[ 1 ];
			} else if( "fstop" == name ) {
				fstop = *( ( RtFloat* )( *it )->data() );
			} else if( "focallength" == name ) {
				focallength = *( ( RtFloat* )( *it )->data() );
			} else if( "focaldistance" == name ) {
				focaldistance = *( ( RtFloat* )( *it )->data() );
			} else if( "pixelsamples" == name ) {
				RtFloat *tmp = ( RtFloat* )( *it )->data();
				pixelsamples[ 0 ] = tmp[ 0 ];
				pixelsamples[ 1 ] = tmp[ 1 ];
			} else if( "bucketorder" == name ) {
				bucketorder = const_cast< RtString >( ( char* )( *it )->data() );
			} else if( name.find( "bucketsize" ) != string::npos ) {
				RtInt *tmp = ( RtInt* )( *it )->data();
				bucketsize[ 0 ] = tmp[ 0 ];
				bucketsize[ 1 ] = tmp[ 1 ];
			} else if( "gridsize" == name ) {
				gridsize = *( ( RtInt* )( *it )->data() );
			} else if( "texturememory" == name ) {
				texturememory = *( ( RtInt* )( *it )->data() );
			} else if( "hider" == name ) {
				hider = const_cast< RtString >( ( char* )( *it )->data() );
			} else if( "samplemotion" == name ) {
				sampleMotion = *( ( RtInt* )( *it )->data() );
			} else if( "jitter" == name ) {
				hiderjitter = *( ( RtInt* )( *it )->data() );
			} else if( "depthfilter" == name ) {
				hiderdepthfilter = const_cast< RtString >( ( char* )( *it )->data() );
			} else if( "extrememotiondof" == name ) {
				extremeMotionDof = *( ( RtInt* )( *it )->data() );
			} else if( "eyesplits" == name ) {
				eyeSplits = *( ( RtInt* )( *it )->data() );
			}
		}

		debugMessage( L"UeberManRi: Writing tokens 0" );

		if( projection ) {
			if( fov != 90 ) {
				RiProjection( projection, "fov", &fov, RI_NULL );
			} else {
				RiProjection( projection, RI_NULL );
			}
		}

		debugMessage( L"UeberManRi: Writing tokens 0.1" );

		//if( ( pixelsamples[ 0 ] != 3 ) || ( pixelsamples[ 1 ] != 3 ) )
		RiPixelSamples( pixelsamples[ 0 ], pixelsamples[ 1 ] );

		if( screen[ 0 ] || screen[ 1 ] || screen[ 2 ] || screen[ 3 ] )
			RiScreenWindow( screen[ 0 ], screen[ 1 ], screen[ 2 ], screen[ 3 ] );

		if( ( 0.1 != near ) || ( 1e6 != far ) ) {
			RiClipping( near, far );
		}

		if( ( 640 != resolution[ 0 ] ) || ( 480 != resolution[ 1 ] ) || ( 1 != pixelaspect ) ) {
			RiFormat( resolution[ 0 ], resolution[ 1 ], pixelaspect );
		}

		if( ( 0 != crop[ 0 ] ) || ( 1 != crop[ 1 ] ) || ( 0 != crop[ 2 ] ) || ( 1 != crop[ 3 ] ) ) {
			RiCropWindow( crop[ 0 ], crop[ 1 ], crop[ 2 ], crop[ 3 ] );
		}

		if( ( 0 != shutter[ 1 ] ) || ( 0 != shutter[ 0 ] ) ) {
			RiShutter( shutter[ 0 ], shutter[ 1 ]  );
		}

		if( 0 != shutterOffset ) {
			RiOption( "shutter", "offset", &shutterOffset, RI_NULL );
		}

		debugMessage( L"UeberManRi: Writing tokens 0.2" );

		if( ( 1 != shutterEfficiency[ 1 ] ) || ( 1 != shutterEfficiency[ 0 ] ) ) {
			RiOption( "shutter", "efficiency", shutterEfficiency, RI_NULL );
		}

		debugMessage( L"UeberManRi: Writing tokens 1" );

		if( RI_INFINITY != fstop ) {
			RiDepthOfField( fstop, focallength, focaldistance );
		}

		if( NULL != bucketorder ) {
			RiOption( "render", "bucketorder", &bucketorder, RI_NULL );
		}

		if( ( 16 != bucketsize[ 0 ] ) || ( 16 != bucketsize[ 1 ] ) ) {
			RiOption( "limits", "bucketsize", bucketsize, RI_NULL );
		}

		if( 256 != gridsize ) {
			RiOption( "limits", "gridsize", &gridsize, RI_NULL );
		}

		if( 8192 != texturememory ) {
			RiOption( "limits", "texturememory", &texturememory, RI_NULL );
		}

#ifdef DELIGHT
		if( 6 != eyeSplits )
#else
		if( 10 != eyeSplits )
#endif
		{
			RiOption( "limits", "eyesplits", &eyeSplits, RI_NULL );
		}

		if( NULL != hider ) {
			if( NULL != hiderdepthfilter ) {
				RiHider( hider, "jitter", &hiderjitter, "samplemotion", &sampleMotion, "extrememotiondof", &extremeMotionDof, "depthfilter", &hiderdepthfilter, RI_NULL );
			} else {
				RiHider( hider, "jitter", &hiderjitter, "samplemotion", &sampleMotion, "extrememotiondof", &extremeMotionDof, RI_NULL );
			}
		} else
			if( NULL != hiderdepthfilter ) {
				RiHider( RI_HIDDEN, "jitter", &hiderjitter, "samplemotion", &sampleMotion, "extrememotiondof", &extremeMotionDof, "depthfilter", &hiderdepthfilter, RI_NULL );
			} else {
				RiHider( RI_HIDDEN, "jitter", &hiderjitter, "samplemotion", &sampleMotion, "extrememotiondof", &extremeMotionDof, RI_NULL );
		}

		currentState->tokenValueCache.clear();

		debugMessage( L"UeberManRi: Writing tokens done" );

	}

	void ueberManRiRenderer::output( const string& name, const string& format, const string& dataname, const cameraHandle& cameraid ) {
		debugMessage( L"UeberManRi: Output" );

		RtFloat exposure[ 2 ] = { 1, 1 };
		RtFloat filterwidth[ 2 ] = { 2, 2 };
		RtInt quantize[ 3 ] = { 0, 0, 0 }; // one, min, max
		RtFloat dither = 0;
		string filter( "box" );

		// convert gaim + gamma to RMan "exposure"
		// scan:
		for( vector< tokenValue::tokenValuePtr >::iterator it = currentState->tokenValueCache.begin(); it < currentState->tokenValueCache.end(); it++ ) {
			string name = ( *it )->name();
			if( name.find( "gain" ) != string::npos ) {
				exposure[ 0 ] = *( ( float* )( *it )->data() );
				continue;
			}
			if( name.find( "gamma" ) != string::npos ) {
				exposure[ 1 ] = *( ( float* )( *it )->data() );
				continue;
			}
			if( !currentState->secondaryDisplay ) {
				if( name.find( "filterwidth[2]" ) != string::npos ) {
					float* tmp = ( float* )( *it )->data();
					filterwidth[ 0 ] = tmp[ 0 ];
					filterwidth[ 1 ] = tmp[ 1 ];
					continue;
				} else
				if( name.find( "filter" ) != string::npos ) {
					filter = ( char* )( *it )->data();
					continue;
				} else
				if( name.find( "quantize[4]" ) != string::npos ) {
					float* tmp = ( float* )( *it )->data();
					quantize[ 0 ] = ( int )tmp[ 1 ];
					quantize[ 1 ] = ( int )tmp[ 2 ];
					quantize[ 2 ] = ( int )tmp[ 3 ];
					continue;
				} else
				if( name.find( "dither" ) != string::npos ) {
					dither = *( ( float* )( *it )->data() );
					continue;
				}
			}
		}

		if( ( 1 != exposure[ 0 ] ) || ( 1 != exposure[ 1 ] ) )
			currentState->tokenValueCache.push_back( tokenValue::tokenValuePtr( new tokenValue( exposure, 2, "exposure", tokenValue::storageUniform, tokenValue::typeFloat ) ) );

		string displayName;
		if( currentState->secondaryDisplay ) {
			displayName = '+' + name;
		} else {
			displayName = name;

			if( "box" == filter )
				RiPixelFilter( RiBoxFilter, filterwidth[ 0 ], filterwidth[ 1 ] );
			else
			if( "triangle" == filter )
				RiPixelFilter( RiTriangleFilter, filterwidth[ 0 ], filterwidth[ 1 ] );
			else
			if( "catmull-rom" == filter )
				RiPixelFilter( RiCatmullRomFilter, filterwidth[ 0 ], filterwidth[ 1 ] );
			else
			if( "sinc" == filter )
				RiPixelFilter( RiSincFilter, filterwidth[ 0 ], filterwidth[ 1 ] );
			else
			if( "gaussian" == filter )
				RiPixelFilter( RiGaussianFilter, filterwidth[ 0 ], filterwidth[ 1 ] );
			else
			if( "mitchell" == filter )
				RiPixelFilter( RiMitchellFilter, filterwidth[ 0 ], filterwidth[ 1 ] );
			else
			if( "bessel" == filter )
				RiPixelFilter( RiBesselFilter, filterwidth[ 0 ], filterwidth[ 1 ] );
			else
			if( "blackmann-harris" == filter )
				RiPixelFilter( RiBlackmanHarrisFilter, filterwidth[ 0 ], filterwidth[ 1 ] );

			RiQuantize( const_cast< char* >( dataname.c_str() ), quantize[ 0 ], quantize[ 1 ], quantize[ 2 ], dither );
		}

		boost::shared_array< RtToken > tokens;
		boost::shared_array< RtPointer > values;
		vector< tokenValue::parameterType > valueTypes( currentState->tokenValueCache.size() );
		unsigned numParams = currentState->fillShaderTokenValueArrays( tokens, values, valueTypes );

		RiDisplayV(	const_cast< char* >( displayName.c_str() ),	const_cast< char* >( format.c_str() ), const_cast< char* >( dataname.c_str() ),	numParams, tokens.get(), values.get() );

		currentState->resetValueArray( values, valueTypes );

		currentState->secondaryDisplay = true;

		currentState->tokenValueCache.clear();
	}

	void ueberManRiRenderer::world() {
		debugMessage( L"UeberManRi: World" );

		currentState->inWorldBlock = true;
		RiWorldBegin();
		RtString operation( "save" );
		RtString subset( "shading,geometrymodification,geometrydefinition" );
		RiResource( "__zero", "attributes", "opration", &operation, "subset", &subset, RI_NULL );
	}

	void ueberManRiRenderer::render( const string& cameraname ) {
		debugMessage( L"UeberManRi: Render" );
		stateMachine.clear();
	}

	void ueberManRiRenderer::motion( const vector< float >& times ) {
		debugMessage( L"UeberManRi: Motion" );

		currentState->sampleCount	=
		currentState->numSamples	= times.size();
		currentState->motionSamples = times;
	}

	void ueberManRiRenderer::parameter( const vector< tokenValue >& tokenValueArray ) {
		for( vector< tokenValue >::const_iterator it( tokenValueArray.begin() ); it != tokenValueArray.end(); it++ )
			currentState->tokenValueCache.push_back( tokenValue::tokenValuePtr( new tokenValue( *it  ) ) );
	}

	void ueberManRiRenderer::parameter( const tokenValue& aTokenValue ) {
		debugMessage( L"UeberManRi: Parameter [token-value]" );

		currentState->tokenValueCache.push_back( tokenValue::tokenValuePtr( new tokenValue( aTokenValue ) ) );
	}

	void ueberManRiRenderer::parameter( const string& typedname, const string& value ) {
		debugMessage( L"UeberManRi: Parameter [string]" );

		string::size_type pos = typedname.find( " " );
		if( string::npos != pos ) {
			string type = typedname.substr( 0, pos );
			string name = typedname.substr( pos + 1 );
			if( "input" == type ) // this should be formatted as a RIB file name
				currentState->tokenValueCache.push_back( tokenValue::tokenValuePtr( new tokenValue( tokenValue( value + ".rib", name ) ) ) );
		} else
			currentState->tokenValueCache.push_back( tokenValue::tokenValuePtr( new tokenValue( tokenValue( value, typedname ) ) ) );
	}

	void ueberManRiRenderer::parameter( const string& typedname, const float value ) {
		debugMessage( L"UeberManRi: Parameter [float]" );

		currentState->tokenValueCache.push_back( tokenValue::tokenValuePtr( new tokenValue( tokenValue( value, typedname ) ) ) );
	}

	void ueberManRiRenderer::parameter( const string& typedname, const int value ) {
		debugMessage( L"UeberManRi: Parameter [int]" );

		currentState->tokenValueCache.push_back( tokenValue::tokenValuePtr( new tokenValue( tokenValue( value, typedname ) ) ) );
	}

	void ueberManRiRenderer::parameter( const string& typedname, const bool value ) {
		debugMessage( L"UeberManRi: Parameter [bool]" );

		parameter( typedname, ( int )value );
	}

	void ueberManRiRenderer::attribute( const tokenValue& aTokenValue ) {
		currentState->checkStartMotion();
		debugMessage( L"UeberManRi: Attribute [token-value]" );

		string typedname = aTokenValue.name();
		string type, name;

		string::size_type pos = typedname.find( ":" );
		if( string::npos == pos ) {
			name = typedname;
			type = "user";
		}
		else {
			type = typedname.substr( 0,  pos );
			name = typedname.substr( pos + 1 );
		}

		RtPointer value;
		if( tokenValue::typeString == aTokenValue.type() ) {
			static char *tmp;
			value = &tmp;
			*( ( char** ) value ) = ( char* ) const_cast< void* >( aTokenValue.data() );
		} else {
			value = const_cast< void* >( aTokenValue.data() );
		}

		if( "shading" == type ) {
			if( "rate" == name ) {
				RiShadingRate( *( float* )value );
			} else
			if( "quality" == name ) {
				RiShadingRate( 1.0f / ( *( float* )value * *( float* )value ) );
			} else
			if( "color" == name ) {
				RiColor( ( float* )value );
			} else
			if( "opacity" == name ) {
				RiOpacity( ( float* )value );
			} else
			if( "motionfactor" == name ) {
				RiGeometricApproximation( "motionfactor", *( float* )value );
			} else
			if( "focusfactor" == name ) {
				RiGeometricApproximation( "focusfactor", *( float* )value );
			} else
			if( "backfacing" == name ) {
				if( *( bool* )value ) {
					RiSides( 2 );
				} else {
					RiSides( 1 );
				}
			} else
			if( "matte" == name ) {
				RiMatte( *( int* )value );
			} else
			if( "interpolation" == name ) {
				RiShadingInterpolation( ( char* )value );
			} else {
				name = aTokenValue.typeAsString() + " " + name;
				// Do we need to handle the string case???
				RiAttribute( const_cast< char* >( type.c_str() ), name.c_str(), value, RI_NULL );
			}
		} else
		if( "dicing" == type ) {
			type = "dice";
			name = aTokenValue.typeAsString() + " " + name;
			// Do we need to handle the string case???
			RiAttribute( const_cast< char* >( type.c_str() ), name.c_str(), value, RI_NULL );
		}
		else {
			name = aTokenValue.typeAsString() + " " + name;
			// Do we need to handle the string case???
			RiAttribute( const_cast< char* >( type.c_str() ), name.c_str(), value, RI_NULL );
		}
		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::attribute( const string& typedname, const string& value ) {
		currentState->checkStartMotion();

		debugMessage( L"UeberManRi: Attribute [string]" );

		string::size_type pos = typedname.find( ":" );
		RtString str = const_cast< char* >( value.c_str() );
		if( pos == string::npos ) {
			string name( typedname );
			if( string::npos == typedname.find( "string" ) )
				name = "string " + name;
			RiAttribute( "user", name.c_str(), &str, RI_NULL );
		} else {
			string type( typedname.substr( 0,  pos ) );
			string name( typedname.substr( pos + 1 ) );
			if( ( "interpolation" == name ) && ( "shading" == type ) ) {
				RiShadingInterpolation( str );
			}
			RiAttribute( const_cast< char* >( type.c_str() ), name.c_str(), &str, RI_NULL );
		}
		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::attribute( const string& typedname, const float value ) {
		currentState->checkStartMotion();

		debugMessage( L"UeberManRi: Attribute [float]" );

		string::size_type pos = typedname.find( ":" );
		if( string::npos == pos ) {
			string name( typedname );
			if( string::npos == typedname.find( "float" ) )
				name = "float " + name;
			RiAttribute( "user", name.c_str(), &value, RI_NULL );
		}
		else {
			string type( typedname.substr( 0,  pos ) );
			string name( typedname.substr( pos + 1 ) );
			/*if( string::npos == typedname.find( "float" ) )
				name = "float " + name;*/
			if( "shading" == type ) {
				if( "rate" == name ) {
					RiShadingRate( value );
				} else
				if( "quality" == name ) {
					RiShadingRate( 1.0f / value );
				} else
				if( "color" == name ) {
					float color[ 3 ] = { value, value, value };
					RiColor( color );
				} else
				if( "opacity" == name ) {
					float opacity[ 3 ] = { value, value, value };
					RiOpacity( opacity );
				} else
				if( "motionfactor" == name ) {
					RiGeometricApproximation( "motionfactor", value );
				} else
				if( "focusfactor" == name ) {
					RiGeometricApproximation( "focusfactor", value );
				} else {
					RiAttribute( const_cast< char* >( type.c_str() ), name.c_str(), &value, RI_NULL );
				}
			} else
			if( "dicing" == type ) {
				type = "dice";
				// Do we need to handle the string case???
				RiAttribute( const_cast< char* >( type.c_str() ), name.c_str(), &value, RI_NULL );
			} else {
				RiAttribute( const_cast< char* >( type.c_str() ), name.c_str(), &value, RI_NULL );
			}
		}
		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::attribute( const string& typedname, const bool value ) {
		attribute( typedname, ( int )value );
	}

	void ueberManRiRenderer::attribute( const string& typedname, const int value ) {
		currentState->checkStartMotion();

		debugMessage( L"UeberManRi: Attribute [int]" );

		string::size_type pos = typedname.find( ":" );
		if( string::npos == pos ) {
			string name( typedname );
			if( string::npos == typedname.find( "int" ) )
				name = "int " + name;
			RiAttribute( "user", name.c_str(), &value, RI_NULL);
		}
		else {
			string type( typedname.substr( 0,  pos ) );
			string name( typedname.substr( pos + 1 ) );
			/*if( string::npos == typedname.find( "int" ) )
				name = "int " + name;*/
			if( "shading" == type ) {
				if( "matte" == name ) {
					RiMatte( value );
				} else
				if( "backfacing" == name ) {
					if( value ) {
						RiSides( 2 );
					} else {
						RiSides( 1 );
					}
				} else {
					RiAttribute( const_cast< RtToken >( type.c_str() ), name.c_str(), &value, RI_NULL );
				}
			} else
			if( "dicing" == type ) {
				type = "dice";
				// Do we need to handle the string case???
				RiAttribute( const_cast< RtToken >( type.c_str() ), name.c_str(), &value, RI_NULL );
			} else {
				RiAttribute( const_cast< RtToken >( type.c_str() ), name.c_str(), &value, RI_NULL );
			}
		}
		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::option( const tokenValue &aTokenValue ) {
		currentState->checkStartMotion();
		debugMessage( L"UeberManRi: Option [token-value]" );

		tokenValue *tokenValuePtr = const_cast< tokenValue* >( &aTokenValue );

		string typedname = tokenValuePtr->name();
		string type, name;

		string::size_type pos = typedname.find( ":" );
		if( string::npos == pos ) {
			name = typedname;
			type = "user";
		}
		else {
			type = typedname.substr( 0,  pos );
			name = typedname.substr( pos + 1 );
		}

		RtPointer value;
		if( tokenValue::typeString == tokenValuePtr->type() ) {
			static char *tmp;
			value = &tmp;
			*( ( char** ) value ) = ( char* ) const_cast< void* >( tokenValuePtr->data() );
		} else {
			value = const_cast< void* >( tokenValuePtr->data() );
		}

		name = tokenValuePtr->typeAsString() + " " + name;
		// Do we need to handle the string case???
		RiOption( const_cast< char* >( type.c_str() ), name.c_str(), value, RI_NULL );

		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::option( const string& typedname, const string& value ) {
		currentState->checkStartMotion();

		debugMessage( L"UeberManRi: Option [string]" );

		string::size_type pos = typedname.find( ":" );
		RtString str = const_cast< RtString >( value.c_str() );
		if( pos == string::npos ) {
			string name( typedname );
			if( string::npos == typedname.find( "string" ) )
				name = "string " + name;
			RiOption( "user", name.c_str(), &str, RI_NULL );
		} else {
			string type = typedname.substr( 0,  pos );
			string name = typedname.substr( pos + 1 );
			RiOption( const_cast< char* >( type.c_str() ), name.c_str(), &str, RI_NULL );
		}
		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::option( const string& typedname, const float value ) {
		currentState->checkStartMotion();

		debugMessage( L"UeberManRi: Option [float]" );

		string::size_type pos = typedname.find( ":" );
		if( string::npos == pos ) {
			string name( typedname );
			if( string::npos == typedname.find( "float" ) )
				name = "float " + name;
			RiOption( "user", name.c_str(), &value, RI_NULL );
		}
		else {
			string type = typedname.substr( 0,  pos );
			string name = typedname.substr( pos + 1 );
			RiOption( const_cast< char* >( type.c_str() ), name.c_str(), &value, RI_NULL );
		}
		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::option( const string& typedname, const bool value ) {
		option( typedname, ( int )value );
	}

	void ueberManRiRenderer::option( const string& typedname, const int value ) {
		currentState->checkStartMotion();

		debugMessage( L"UeberManRi: Option [int]" );

		string::size_type pos = typedname.find( ":" );
		if( string::npos == pos ) {
			string name( typedname );
			if( string::npos == typedname.find( "int" ) )
				name = "int " + name;
			RiOption( "user", name.c_str(), &value, RI_NULL);
		}
		else {
			string type = typedname.substr( 0,  pos );
			string name = typedname.substr( pos + 1 );
			RiOption( const_cast< char* >( type.c_str() ), name.c_str(), &value, RI_NULL );
		}
		currentState->checkEndMotion();
	}

	bool ueberManRiRenderer::getAttribute( const string& typedname, float& value ) {

		RtPointer result = NULL;
		RxInfoType_t resultType;
		RtInt resultCount;
		RtInt resultLen = sizeof( RtFloat );

		RxAttribute( const_cast< char* >( typedname.c_str() ), result, 0, &resultType, &resultCount );

		bool success = false;
		if( ( RxInfoFloat == resultType ) && ( resultLen == resultCount ) ) {
			result = new RtFloat;
			if( !RxAttribute( const_cast< char* >( typedname.c_str() ), result, resultLen, &resultType, &resultCount ) ) {
				value = ( float )*( ( RtFloat* )result );
				success = true;
			}
			delete ( RtFloat* )result;
		}

		return success;
	}

	bool ueberManRiRenderer::getAttribute( const string& typedname, int& value ) {

		RtPointer result = NULL;
		RxInfoType_t resultType;
		RtInt resultCount;
		RtInt resultLen = sizeof( RtInt );

		RxAttribute( const_cast< char* >( typedname.c_str() ), result, 0, &resultType, &resultCount );

		bool success = false;
		if( ( RxInfoInteger == resultType ) && ( resultLen == resultCount ) ) {
			result = new RtInt;
			if( !RxAttribute( const_cast< char* >( typedname.c_str() ), result, resultLen, &resultType, &resultCount ) ) {
				value = ( int )*( ( RtInt* )result );
				success = true;
			}
			delete ( RtInt* )result;
		}

		return success;
	}

	bool ueberManRiRenderer::getAttribute( const string& typedname, string& value ) {

		RtPointer result = NULL;
		RxInfoType_t resultType;
		RtInt resultCount;
		RtInt resultLen;

		RxAttribute( const_cast< char* >( typedname.c_str() ), result, 0, &resultType, &resultCount );

		bool success = false;
		if( RxInfoStringV == resultType ) {
			result = new RtString*;
			resultLen = resultCount;
			*( ( RtString* )result ) = new char[ resultLen ];

			if( !RxAttribute( const_cast< char* >( typedname.c_str() ), result, resultLen, &resultType, &resultCount ) ) {
				value = *( ( RtString* )result );
				success = true;
			}
			delete[] *( ( RtString* )result );
			delete ( RtString* )result;
		}

		return success;
	}

	void ueberManRiRenderer::pushAttributes() {
		debugMessage( L"UeberManRi: PushAttributes" );

		RiAttributeBegin();

	}

	void ueberManRiRenderer::popAttributes() {
		debugMessage( L"UeberManRi: PopAttributes" );

		RiAttributeEnd();

	}

	void ueberManRiRenderer::pushSpace() {
		debugMessage( L"UeberManRi: PushSpace" );

		RiTransformBegin();

	}

	void ueberManRiRenderer::popSpace() {
		debugMessage( L"UeberManRi: PopSpace" );

		RiTransformEnd();

	}

	void ueberManRiRenderer::space( const vector< float >& matrix ) {
		currentState->checkStartMotion();

		debugMessage( L"UeberManRi: space[matrix]" );

		RiTransform( *( RtMatrix* )( void* )&matrix[ 0 ] );

		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::space( const spaceHandle& spacename ) {
		currentState->checkStartMotion();

		debugMessage( L"UeberManRi: space[name]" );
		if( "identity" == spacename )
			RiIdentity();
		else
			RiCoordSysTransform( const_cast< char* >( spacename.c_str() ) );

		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::nameSpace( spaceHandle& spacename ) {
		if( "identity" != spacename ) {
			currentState->checkStartMotion();

			debugMessage( L"UeberManRi: nameSpace" );

			RiCoordinateSystem( const_cast< char* >( spacename.c_str() ) );

			currentState->checkEndMotion();
		}
	}

	void ueberManRiRenderer::appendSpace( const vector< float >& matrix ) {
		currentState->checkStartMotion();

		debugMessage( L"UeberManRi: AppendSpace" );

		RiConcatTransform( *( RtMatrix* )( void* )&matrix[ 0 ] );

		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::translate( const float x, const float y, const float z ) {
		currentState->checkStartMotion();

		debugMessage( L"UeberManRi: Translate" );
		RiTranslate( x, y, z );

		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::rotate( const float angle, const float x, const float y, const float z ) {
		debugMessage( L"UeberManRi: Rotate" );

		currentState->checkStartMotion();
		RiRotate( angle, x, y, z );

		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::scale( const float x, const float y, const float z ) {
		debugMessage( L"UeberManRi: Scale" );

		currentState->checkStartMotion();
		RiScale( x, y, z );

		currentState->checkEndMotion();
	}


	void ueberManRiRenderer::shader( const string& shadertype, const string& shadername, shaderHandle& shaderid ) {
		debugMessage( L"UeberManRi: Shader" );

		boost::shared_array< RtToken > tokens;
		boost::shared_array< RtPointer > values;
		vector< tokenValue::parameterType > valueTypes( currentState->tokenValueCache.size() );
		unsigned numParams = currentState->fillShaderTokenValueArrays( tokens, values, valueTypes );

		if( "surface" == shadertype ) {
			RiSurfaceV( const_cast< char* >( shadername.c_str() ), numParams, tokens.get(), values.get() );
		} else
		if( "displacement" == shadertype ) {
			RiDisplacementV( const_cast< char* >( shadername.c_str() ), numParams, tokens.get(), values.get() );
		} else
		if( "volume" == shadertype ) {
			RiAtmosphereV( const_cast< char* >( shadername.c_str() ), numParams, tokens.get(), values.get() );
		}
		if( !currentState->inWorldBlock && ( "imager" == shadertype ) ) {
			RiImagerV( const_cast< char* >( shadername.c_str() ), numParams, tokens.get(), values.get() );
		}

		currentState->resetValueArray( values, valueTypes );

		currentState->tokenValueCache.clear();
	}

	void ueberManRiRenderer::light( const string& shadername, lightHandle& lightid ) {
		debugMessage( L"UeberManRi: Light" );

		currentState->tokenValueCache.push_back( tokenValue::tokenValuePtr( new tokenValue( lightid, "__handleid" ) ) );

		boost::shared_array< RtToken > tokens;
		boost::shared_array< RtPointer > values;
		vector< tokenValue::parameterType > valueTypes( currentState->tokenValueCache.size() );
		unsigned numParams = currentState->fillShaderTokenValueArrays( tokens, values, valueTypes );

		RiLightSourceV( const_cast< char* >( shadername.c_str() ), numParams, tokens.get(), values.get() );

		currentState->resetValueArray( values, valueTypes );

		currentState->tokenValueCache.clear();
	}

	void ueberManRiRenderer::switchLight( const lightHandle& lightid, const bool on ) {
		debugMessage( L"UeberManRi: LightSwitch" );

		currentState->checkStartMotion();

		RiIlluminate( ( RtLightHandle )lightid.c_str(), on );

		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::beginLook( lookHandle& id ) {
		debugMessage( L"UeberManRi: BeginLook" );
		string name = id + ".look";
		RiArchiveBegin( const_cast< char* >( name.c_str() ), RI_NULL );
	}

	void ueberManRiRenderer::endLook() {
		debugMessage( L"UeberManRi: EndLook" );
		RiArchiveEnd();
	}

	void ueberManRiRenderer::look( const lookHandle& id ) {
		debugMessage( L"UeberManRi: Look" );
		string name = id + ".look";
		// Unroll the entire graphics state to WorldBegin state, then instance the look
		RtString operation( "restore" );
		RtString subset( "shading,geometrymodification,geometrydefinition" );
		RiResource( "__zero", "attributes", "opration", &operation, "subset", &subset, RI_NULL );
		RiReadArchive( const_cast< char* >( name.c_str() ), NULL, RI_NULL );
	}

	void ueberManRiRenderer::appendLook( const lookHandle& id ) {
		string name = id + ".look";
		debugMessage( L"UeberManRi: AppendLook" );
		RiReadArchive( const_cast< char* >( name.c_str() ), NULL, RI_NULL );
	}

	void ueberManRiRenderer::points( const string& type, const int numPoints, primitiveHandle &identifier ) {

		currentState->checkStartMotion();

		if( !type.empty() ) {
			// Below abtraction allows one to be arther lazy and specify "blobby"-type particles
			if( "blobby" == type ) {

				vector< RtInt > code;
				code.reserve( numPoints * 2 + 2 + numPoints );
				float* widths( NULL );
				float* positions( NULL );
				float width( 1 );

				boost::shared_array< RtToken > tokens;
				boost::shared_array< RtPointer > values;
				vector< tokenValue::parameterType > valueTypes;
				unsigned numParams( 0 );

				tokens = boost::shared_array< RtToken >( new RtToken[ currentState->tokenValueCache.size() ] );
				values = boost::shared_array< RtPointer >( new RtPointer[ currentState->tokenValueCache.size() ] );

				for( vector< tokenValue::tokenValuePtr >::const_iterator it = currentState->tokenValueCache.begin(); it < currentState->tokenValueCache.end(); it++ ) {
					string name( ( *it )->name() );
					if( "width" == name ) {
						if( tokenValue::typeFloat == ( *it )->type() ) {
							switch( ( *it )->storage() ) {
								case tokenValue::storageConstant:
								case tokenValue::storageUniform:
									width = *( float* )( *it )->data();
									break;
								case tokenValue::storageVarying:
								case tokenValue::storageVertex:
									widths = const_cast< float* >( ( float* )( *it )->data() );
							}
						}
					} else
					if( "P" == name ) {
						if( tokenValue::typePoint == ( *it )->type() ) {
							switch( ( *it )->storage() ) {
								case tokenValue::storageVarying:
								case tokenValue::storageVertex:
									positions = const_cast< float* >( ( float* )( *it )->data() );
							}
						}
					} else {
						string tmpStr( currentState->getTokenAsClassifiedString( **it ) );
						tokens[ numParams ] = new char[ tmpStr.length() + 1 ];
						strcpy( tokens[ numParams ], tmpStr.c_str() );
						valueTypes.push_back( ( *it )->type() );
						if( tokenValue::typeString == valueTypes.back() ) {
							values[ numParams ] = new char*;
							*( (char**) values [ numParams ] ) = ( char* ) ( *it )->data();
						}
						else {
							values[ numParams ] = const_cast< void* >( ( *it )->data() );
						}
						numParams++;
					}
				}

				RtString strings[ 2 ];
				strings[ 0 ] = "";
				strings[ 1 ] = NULL;

				if( positions ) {
					int index = 0;
					vector< float > pos;
					pos.reserve( numPoints * 3 );
					for( unsigned i = 0; i < ( unsigned )numPoints; i++ ) {
						code.push_back( 1001 );
						code.push_back( index );

						float theWidth;
						if( widths )
							theWidth = widths[ i ];
						else
							theWidth = width;

						pos.push_back( theWidth );
						pos.push_back( 0 );
						pos.push_back( 0 );
						pos.push_back( 0 );

						pos.push_back( 0 );
						pos.push_back( theWidth );
						pos.push_back( 0 );
						pos.push_back( 0 );

						pos.push_back( 0 );
						pos.push_back( 0 );
						pos.push_back( theWidth );
						pos.push_back( 0 );

						pos.push_back( positions[ i * 3 ] );
						pos.push_back( positions[ i * 3 + 1 ] );
						pos.push_back( positions[ i * 3 + 2 ] );
						pos.push_back( 1 );

						index += 16;
					}
					code.push_back( 0 );
					code.push_back( numPoints );
					for( RtInt i = 0; i < numPoints; i++ )
						code.push_back( i );

					RiBlobbyV(  numPoints, code.size(), &code[ 0 ], pos.size(), const_cast< float* >( &pos[ 0 ] ),
								1, strings,
								numParams, tokens.get(), values.get() );

					currentState->resetValueArray( values, valueTypes );
				} else { // no positions -> write an empty blob
					RiBlobby( 0, 0, NULL, 0, NULL, 1, strings, RI_NULL );
				}
			} else {
				currentState->tokenValueCache.push_back( tokenValue::tokenValuePtr( new tokenValue( type, "type" ) ) );

				boost::shared_array< RtToken > tokens;
				boost::shared_array< RtPointer > values;
				vector< tokenValue::parameterType > valueTypes( currentState->tokenValueCache.size() );
				unsigned numParams = currentState->fillPrimitiveTokenValueArrays( tokens, values, valueTypes );

				debugMessage( L"UeberManRi: Points" );

				RiPointsV( numPoints, numParams, tokens.get(), values.get() );

				currentState->resetValueArray( values, valueTypes );
			}
		}

		currentState->tokenValueCache.clear();

		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::curves( const string& interp, const int ncurves, const int numVertsPerCurve, const bool closed, primitiveHandle &identifier ) {
		// RenderMan needs nverts per curve
		vector< int > nverts;
		nverts.reserve( ncurves );
		for( unsigned i = 0; i < ( unsigned )ncurves; i++ )
			nverts[ i ] = numVertsPerCurve;

		curves( interp, ncurves, nverts, closed, identifier );
	}

	void ueberManRiRenderer::curves( const string& interp, const int ncurves, const vector< int >& numVertsPerCurve, const bool closed, primitiveHandle &identifier ) {

		// If we need to set the basis, make sure we're not overwriting the current one
		RtToken type;
		if( "linear" != interp ) {
			type = RI_CUBIC;

			// Check if we're about to start a motion block
			if( ( 0 > currentState->sampleCount ) || ( currentState->sampleCount == currentState->numSamples ) ) {
				RiAttributeBegin();
				// Set basis
				if( ( "bspline" == interp ) || ( "b-spline" == interp ) ) {
					RiBasis( RiBSplineBasis, RI_BSPLINESTEP, RiBSplineBasis, RI_BSPLINESTEP );
				} else
				if( "bezier" == interp ) {
					RiBasis( RiBezierBasis, RI_BEZIERSTEP, RiBezierBasis, RI_BEZIERSTEP );
				} else
				if( "catmull-rom" == interp ) {
					RiBasis( RiCatmullRomBasis, RI_CATMULLROMSTEP, RiCatmullRomBasis, RI_CATMULLROMSTEP );
				}
			}
		} else
			type = RI_LINEAR;

		// We check for calling RiMotionBegin() here, after we (may) have set the basis
		currentState->checkStartMotion();

		boost::shared_array< RtToken > tokens;
		boost::shared_array< RtPointer > values;
		vector< tokenValue::parameterType > valueTypes( currentState->tokenValueCache.size() );
		unsigned numParams = currentState->fillPrimitiveTokenValueArrays( tokens, values, valueTypes );

		debugMessage( L"UeberManRi: Curves" );
		RiCurvesV( type, ncurves, const_cast< int* >( &numVertsPerCurve[ 0 ] ), closed ? RI_PERIODIC : RI_NONPERIODIC, numParams, tokens.get(), values.get() );

		currentState->resetValueArray( values, valueTypes );

		currentState->tokenValueCache.clear();

		currentState->checkEndMotion();

		// Close the basis attribute block if we're ending a motion block
		if( ( "linear" != interp ) && ( ( !currentState->sampleCount ) || ( 0 > currentState->sampleCount ) ) ) {
			RiAttributeEnd();
		}
	}

	void ueberManRiRenderer::curves( const int numCurves, const vector< int >& numVertsPerCurve, const vector< int >& order, const vector< float >& knot, const vector< float >& min, const vector< float >& max, primitiveHandle& identifier ) {

		currentState->checkStartMotion();

		boost::shared_array< RtToken > tokens;
		boost::shared_array< RtPointer > values;
		vector< tokenValue::parameterType > valueTypes( currentState->tokenValueCache.size() );
		unsigned numParams = currentState->fillPrimitiveTokenValueArrays( tokens, values, valueTypes );

		// If an implementation doesn't support RiNuCurves (e.g. PRMan),
		// we have to 'convert'/approximate our NuCurves with cubic curves here
		// I leave this as an exercise to you, the inclined hacker reading this
		// I'm in the lucky situation to use 3Delight as I'm writing this, so no
		// need... :)

		debugMessage( L"UeberManRi: NuCurves" );
		RiNuCurvesV( numCurves,
			const_cast< int* >( &numVertsPerCurve[ 0 ] ),
			const_cast< int* >( &order[ 0 ] ),
			const_cast< float* >( &knot[ 0 ] ),
			const_cast< float* >( &min[ 0 ] ),
			const_cast< float* >( &max[ 0 ] ),
		numParams, tokens.get(), values.get() );

		currentState->resetValueArray( values, valueTypes );

		currentState->tokenValueCache.clear();

		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::patch( const string& interp, int nu, int nv, primitiveHandle &identifier ) {

		// If we need to set the basis, make sure we're not overwriting the current one
		RtToken type;
		if( "linear" != interp ) {
			type = RI_BICUBIC;

			// Check if we're about to start a motion block
			if( ( 0 > currentState->sampleCount ) || ( currentState->sampleCount == currentState->numSamples ) ) {
				RiAttributeBegin();
				// Set basis
				if( ( "bspline" == interp ) || ( "b-spline" == interp ) ) {
					RiBasis( RiBSplineBasis, RI_BSPLINESTEP, RiBSplineBasis, RI_BSPLINESTEP );
				} else
				if( "bezier" == interp ) {
					RiBasis( RiBezierBasis, RI_BEZIERSTEP, RiBezierBasis, RI_BEZIERSTEP );
				} else
				if( "catmull-rom" == interp ) {
					RiBasis( RiCatmullRomBasis, RI_CATMULLROMSTEP, RiCatmullRomBasis, RI_CATMULLROMSTEP );
				}
			}
		} else
			type = RI_BILINEAR;

		currentState->checkStartMotion();

		boost::shared_array< RtToken > tokens;
		boost::shared_array< RtPointer > values;
		vector< tokenValue::parameterType > valueTypes( currentState->tokenValueCache.size() );
		unsigned numParams = currentState->fillPrimitiveTokenValueArrays( tokens, values, valueTypes );

		debugMessage( L"UeberManRi: Patch" );

		RiPatchMeshV( type, nu, RI_NONPERIODIC, nv, RI_NONPERIODIC, numParams, tokens.get(), values.get() );

		currentState->resetValueArray( values, valueTypes );

		currentState->tokenValueCache.clear();

		currentState->checkEndMotion();

		if( ( "linear" != interp ) && ( ( !currentState->sampleCount ) || ( 0 > currentState->sampleCount ) ) ) {
			RiAttributeEnd();
		}
	}

	void ueberManRiRenderer::patch( const int nu, const int uorder, const float *uknot, const float umin, const float umax, const int nv, const int vorder, const float *vknot, const float vmin, const float vmax, primitiveHandle &identifier ) {
		currentState->checkStartMotion();

		debugMessage( L"UeberManRi: NuPatch" );

		boost::shared_array< RtToken > tokens;
		boost::shared_array< RtPointer > values;
		vector< tokenValue::parameterType > valueTypes( currentState->tokenValueCache.size() );
		unsigned numParams = currentState->fillPrimitiveTokenValueArrays( tokens, values, valueTypes );

		RiNuPatchV( nu, uorder, const_cast< float* >( uknot ), umin, umax, nv, vorder, const_cast< float* >( vknot ), vmin, vmax, numParams, tokens.get(), values.get() );

		currentState->resetValueArray( values, valueTypes );

		currentState->tokenValueCache.clear();

		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::mesh( const string& interp, const int nfaces, const int *nverts, const int *verts, const bool interpolateBoundary, primitiveHandle &identifier ) {
		debugMessage( L"UeberManRi: Mesh" );

		currentState->checkStartMotion();

		boost::shared_array< RtToken > tokens;
		boost::shared_array< RtPointer > values;
		vector< tokenValue::parameterType > valueTypes( currentState->tokenValueCache.size() );

		if( "linear" == interp ) {
			unsigned numParams = currentState->fillPrimitiveTokenValueArrays( tokens, values, valueTypes );
			RiPointsPolygonsV( nfaces, const_cast< RtInt* >( nverts ), const_cast< RtInt* >( verts ), numParams, tokens.get(), values.get() );
		} else {
			// Hacky as hell: we take the crease/corner values from the parameter stack!
			// Gotta wash my hands, they're -- oh -- so dirty!

			vector< RtToken > tags;
			vector< RtInt > nargs;
			vector< RtInt > intargs;
			vector< RtFloat > floatargs;

			vector< tokenValue::tokenValuePtr > newParamArray;
			float currentCreaseValue( RI_INFINITY );
			float currentCornerValue( RI_INFINITY );
			for( vector< tokenValue::tokenValuePtr >::const_iterator it( currentState->tokenValueCache.begin() ); it != currentState->tokenValueCache.end(); it++ ) {
				if( !( *it )->empty() ) {
					string name( ( *it )->name() );
					if( "creasevalue" == name ) {
						currentCreaseValue = ( ( float* )( *it )->data() )[ 0 ];
					} else
					if( "crease" == name ) {
						tags.push_back( "crease" );
						RtInt size( ( *it )->size() );
						nargs.push_back( size ); // n vertex indices
						nargs.push_back( 1 ); // one crease value
						for( unsigned i( 0 ); i < size; i++ ) {
							intargs.push_back( ( ( int* )( *it )->data() )[ i ] );
						}
						floatargs.push_back( currentCreaseValue );
					} else
					if( "cornervalue" == name ) {
						currentCornerValue = ( ( float* )( *it )->data() )[ 0 ];
					} else
					if( "corner" == name ) {
						tags.push_back( "corner" );
						nargs.push_back( 1 ); // One vertex index
						nargs.push_back( 1 ); // one crease value
						intargs.push_back( ( ( int* )( *it )->data() )[ 0 ] );
						floatargs.push_back( currentCreaseValue );
					} else {
						newParamArray.push_back( *it );
					}
				}
			}

			currentState->tokenValueCache = newParamArray;
			unsigned numParams = currentState->fillPrimitiveTokenValueArrays( tokens, values, valueTypes );

			if ( interpolateBoundary ) {
				tags.push_back( "interpolateboundary" );
				nargs.push_back( 0 );
				nargs.push_back( 0 );
#ifdef _WIN32
				intargs.push_back( 0 );
				floatargs.push_back( 0 );
#endif
			}

			unsigned numTags( tags.size() );
#ifdef _WIN32
			if( !numTags ) {
				tags.push_back( "" );
				nargs.push_back( 0 );
				intargs.push_back( 0 );
				floatargs.push_back( 0 );
			}
#endif

			RiSubdivisionMeshV( const_cast< RtToken >( interp.c_str() ), nfaces, const_cast< RtInt* >( nverts ), const_cast< RtInt* >( verts ),
								numTags, &tags[ 0 ], &nargs[ 0 ], &intargs[ 0 ], &floatargs[ 0 ],
								numParams, tokens.get(), values.get() );
		}

		currentState->resetValueArray( values, valueTypes );

		currentState->tokenValueCache.clear();

		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::sphere( const float radius, const float zmin, const float zmax, const float thetamax, primitiveHandle &identifier ) {
		debugMessage( L"UeberManRi: Sphere" );

		currentState->checkStartMotion();

		boost::shared_array< RtToken > tokens;
		boost::shared_array< RtPointer > values;
		vector< tokenValue::parameterType > valueTypes( currentState->tokenValueCache.size() );
		unsigned numParams = currentState->fillPrimitiveTokenValueArrays( tokens, values, valueTypes );

		RiSphereV( radius, zmin, zmax, thetamax, numParams, tokens.get(), values.get() );

		currentState->resetValueArray( values, valueTypes );

		currentState->tokenValueCache.clear();

		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::blobby( const int numLeafs, const vector< int >& code, const vector< float >& floatData, const vector< string >& stringData, primitiveHandle& identifier ) {
		debugMessage( L"UeberManRi: Blobby" );

		currentState->checkStartMotion();

		boost::shared_array< RtToken > tokens;
		boost::shared_array< RtPointer > values;
		vector< tokenValue::parameterType > valueTypes( currentState->tokenValueCache.size() );
		unsigned numParams = currentState->fillPrimitiveTokenValueArrays( tokens, values, valueTypes );

		unsigned numStrings( stringData.size() );
		RtString* strings( new RtString [ numStrings ? numStrings + 1 : 2 ] );

		if( numStrings ) {
			unsigned i = 0;
			for( vector< string >::const_iterator it = stringData.begin(); it < stringData.end(); it++, i++ )
				strings[ i ] = const_cast< char* >( it->c_str() );
			strings[ numStrings ] = NULL;
		} else {
			strings[ 0 ] = "";
			strings[ 1 ] = NULL;
			numStrings = 1;
		}


		RiBlobbyV(  numLeafs, code.size(), const_cast< int* >( &code[ 0 ] ), floatData.size(), const_cast< float* >( &floatData[ 0 ] ),
					numStrings, strings,
					numParams, tokens.get(), values.get() );

		delete[] strings;

		currentState->resetValueArray( values, valueTypes );

		currentState->tokenValueCache.clear();

		currentState->checkEndMotion();
	}

	void ueberManRiRenderer::makeMap( const string& type ) {

		if( "environment" == type ) {
			char *px, *nx, *py, *ny, *pz, *nz, *mapname;
			RtFloat fov, swidth, twidth;
			RtFilterFunc filterFunc;

			px = nx = py = ny = pz = nz = mapname = "nomap";
			fov = 90.0f;
			filterFunc = RiGaussianFilter;
			swidth = 2.5f;
			twidth = 2.5f;

			for( vector< tokenValue::tokenValuePtr >::const_iterator it( currentState->tokenValueCache.begin() ); it != currentState->tokenValueCache.end(); it++ ) {
				if( !( *it )->empty() ) {
					string name( ( *it )->name() );

					if( tokenValue::typeString == ( *it )->type() ) {
						if( "px" == name )
							px = ( char* )( *it )->data();

						if( "nx" == name )
							nx = ( char* )( *it )->data();

						if( "py" == name )
							py = ( char* )( *it )->data();

						if( "ny" == name )
							ny = ( char* )( *it )->data();

						if( "pz" == name )
							pz = ( char* )( *it )->data();

						if( "nz" == name )
							nz = ( char* )( *it )->data();

						if( "mapname" == name )
							mapname = ( char* ) ( *it )->data();

						if( "filter" == name ) {
							string filter( ( char* ) ( *it )->data() );

							if( "box" == filter )
								filterFunc = RiBoxFilter;
							else
							if( "triangle" == filter )
								filterFunc = RiTriangleFilter;
							else
							if( "catmull-rom" == filter )
								filterFunc = RiCatmullRomFilter;
							else
							if( "sinc" == filter )
								filterFunc = RiSincFilter;
							else
							if( "gaussian" == filter )
								filterFunc = RiGaussianFilter;
							else
							if( "mitchell" == filter )
								filterFunc = RiMitchellFilter;
							else
							if( "bessel" == filter )
								filterFunc = RiBesselFilter;
							else
							if( "blackmann-harris" == filter )
								filterFunc = RiBlackmanHarrisFilter;
						}
					}

					if( tokenValue::typeFloat == ( *it )->type() ) {
						if( "filterwidth" == name ) {
							swidth = ( ( float* ) ( *it )->data() )[ 0 ];
							twidth = ( ( float* ) ( *it )->data() )[ 1 ];
						}

						if( "filterwidth" == name )
							fov = *( float* )( *it )->data();
					}
				}
			}

			RiMakeCubeFaceEnvironment( px, nx, py, ny, pz, nz, mapname, fov, filterFunc, swidth, twidth, RI_NULL );
		}
		currentState->tokenValueCache.clear();
	}

	// Define static class members --------------------------------------------

	// Singleton instance of the renderer
	//ueberManRiRenderer ueberManRiRenderer::theRenderer;


	// The current context to index into our stateMachine array
//	context ueberManRiRenderer::currentContext = 0;

	// State machine stuff
	//map< context, boost::shared_ptr< ueberManRiRenderer::state > > ueberManRiRenderer::stateMachine;
	//boost::shared_ptr< ueberManRiRenderer::state > ueberManRiRenderer::currentState;

	//bool ueberManRiRenderer::dso = false;

	// state subclass ---------------------------------------------------------

	ueberManRiRenderer::state::state()
	:
		// Number of current Ri call's motion sample
		sampleCount( -1 ),

		// Total number of samples in current motion block
		numSamples( 1 ),

		inWorldBlock( false ),
		secondaryDisplay( false )

		// Make room for 20 token values
		//tokenValueCache.resize( 20 );
	{
		debugMessage( L"UeberManRi: Constructing State" );
		renderContext = RiGetContext();
		debugMessage( L"UeberManRi: Done constructing State" );
	}


	ueberManRiRenderer::state::state( const state &cpy ) {
		debugMessage( L"UeberManRi: Copying state" );

		tokenValueCache = cpy.tokenValueCache;
		sampleCount		= cpy.sampleCount;
		numSamples		= cpy.numSamples;
		inWorldBlock	= cpy.inWorldBlock;
		renderContext	= cpy.renderContext;
		motionSamples	= cpy.motionSamples;
	}

	ueberManRiRenderer::state::~state() {
		debugMessage( L"UeberManRi: Deleting state" );
	}

	string ueberManRiRenderer::state::getTokenAsString( const tokenValue& aTokenValue ) {

#ifndef __XSI_PLUGIN
		if(	( "P" == aTokenValue.name() ) ||
			( "Pw" == aTokenValue.name() ) ) {
			return aTokenValue.name();
		}
#endif

		string tokenValueStr;

		switch( aTokenValue.type() ) {
			case tokenValue::typeFloat:
				tokenValueStr = "float ";
				break;
			case tokenValue::typeInteger:
				tokenValueStr = "int ";
				break;
			case tokenValue::typeColor:
				tokenValueStr = "color ";
				break;
			case tokenValue::typePoint:
				tokenValueStr = "point ";
				break;
			case tokenValue::typeHomogenousPoint:
				tokenValueStr = "hpoint ";
				break;
			case tokenValue::typeVector:
				tokenValueStr = "vector ";
				break;
			case tokenValue::typeNormal:
				tokenValueStr = "normal ";
				break;
			case tokenValue::typeMatrix:
				tokenValueStr = "matrix ";
				break;
			case tokenValue::typeString:
				tokenValueStr = "string ";
				break;
			case tokenValue::typeUndefined:
			default:
				tokenValueStr = "";
		}

		tokenValueStr += aTokenValue.name();

		return tokenValueStr;
	}

	string ueberManRiRenderer::state::getTokenAsClassifiedString( const tokenValue& aTokenValue ) {
// Work around the strange 3Delight bug with P/Pw not getting recognized when called from a DSO with their type specified
#ifndef __XSI_PLUGIN
		if(	( "P" == aTokenValue.name() ) ||
			( "Pw" == aTokenValue.name() ) ) {
			return aTokenValue.name();
		}
#endif

		string tokenValueStr;

		switch( aTokenValue.storage() ) {
			case tokenValue::storageConstant:
				tokenValueStr = "constant ";
				break;
			case tokenValue::storagePerPiece:
				tokenValueStr = "uniform ";
				break;
			case tokenValue::storageLinear:      // varying
				tokenValueStr = "varying ";
				break;
			case tokenValue::storageVertex:
				tokenValueStr = "vertex ";
				break;
			case tokenValue::storageFaceVarying: // Needs to be translated into linear for Gelato
				tokenValueStr = "facevarying ";
				break;
			case tokenValue::storageFaceVertex:  // Currently unsupported in Gelato
				tokenValueStr = "facevertex ";
				break;
			case tokenValue::storageUndefined:
			default:
				tokenValueStr = "";
		};

		tokenValueStr += getTokenAsString( aTokenValue );

		return tokenValueStr;
	}

	/** Set up the token value arrays for the next Ri..V() call.
	 *
	 *  Manages memory alloaction and then fills the class internal token and
	 *  value arrays to be passed to the next Ri..V() call.
	 *  Note that the cache itself can't be cleared at this point as the value
	 *  array only contains pointers to the actual data stored in the cache
	 *  (thus avoiding unneccessary copying of data).
	 */
	unsigned ueberManRiRenderer::state::fillPrimitiveTokenValueArrays( boost::shared_array< RtToken >& tokens, boost::shared_array< RtPointer >& values, vector< tokenValue::parameterType >& valueTypes ) {

		tokens = boost::shared_array< RtToken >( new RtToken[ tokenValueCache.size() ] );
		values = boost::shared_array< RtPointer >( new RtPointer[ tokenValueCache.size() ] );
		unsigned numParams = 0;
		for( vector< tokenValue::tokenValuePtr >::const_iterator it = tokenValueCache.begin(); it < tokenValueCache.end(); it++, numParams++ ) {
			string tmpStr( getTokenAsClassifiedString( **it ) );
			tokens[ numParams ] = new char[ tmpStr.length() + 1 ];
			strcpy( tokens[ numParams ], tmpStr.c_str() );
			valueTypes[ numParams ] = ( *it )->type();
			if( tokenValue::typeString == valueTypes[ numParams ] ) {
				values[ numParams ] = new char*;
				*( (char**) values [ numParams ] ) = ( char* ) ( *it )->data();
			}
			else {
				values[ numParams ] = const_cast< void* >( ( *it )->data() );
			}
		}

		return numParams;
	}

	/** Set up the token value arrays for the next RiLight/Surface/Displacement..V() call.
	 *
	 *  Manages memory alloaction and then fills the class internal token and
	 *  value arrays to be passed to the next Ri..V() call.
	 *  Note that the cache itself can't be cleared at this point as the value
	 *  array only contains pointers to the actual data stored in the cache
	 *  (thus avoiding unneccessary copying of data).
	 */
	unsigned ueberManRiRenderer::state::fillShaderTokenValueArrays( boost::shared_array< RtToken >& tokens, boost::shared_array< RtPointer >& values, vector< tokenValue::parameterType >& valueTypes ) {

		tokens = boost::shared_array< RtToken >( new RtToken[ tokenValueCache.size() ] );
		values = boost::shared_array< RtPointer >( new RtPointer[ tokenValueCache.size() ] );

		unsigned numParams = 0;
		for( vector< tokenValue::tokenValuePtr >::const_iterator it = tokenValueCache.begin(); it < tokenValueCache.end(); it++, numParams++ ) {
			string tmpStr( getTokenAsString( tokenValue( **it ) ) );
			tokens[ numParams ] = new char[ tmpStr.length() + 1 ];
			strcpy( tokens[ numParams ], tmpStr.c_str() );
			valueTypes[ numParams ] = ( *it )->type();
			if( tokenValue::typeString == valueTypes[ numParams ] ) {
				values[ numParams ] = new char*;
				*( ( char** ) values[ numParams ] ) = ( char* ) ( *it )->data();
			}
			else {
				values[ numParams ] = const_cast< RtPointer >( ( *it )->data() );
			}
		}

		return numParams;
	}

	void ueberManRiRenderer::state::resetValueArray( boost::shared_array< RtPointer >& values, const vector< tokenValue::parameterType >& valueTypes ) {
		if( values )
			for( unsigned i = 0; i < valueTypes.size(); i++ )
				if( tokenValue::typeString == valueTypes[ i ] )
					delete ( char* ) values[ i ];
	}

	/** Checks if we need to open a MotionBegin block.
	 *
	 *  The reason we do this is that for curves & patches, eventually the
	 *  basis needs to be changed. If that is the case, we need to emit
	 *  an RiBasis call which can't go inside the motion block for obvious
	 *  reasons.
	 */
	void ueberManRiRenderer::state::checkStartMotion() {
		if( sampleCount == numSamples ) {
			debugMessage( L"UeberManRi: MotionBegin" );
			RiMotionBeginV( numSamples, &motionSamples[ 0 ] );

			// make sure we don't emit another MotionBegin call if the API's
			// user forgets to close the current one
			++numSamples;
		}
	}

	void ueberManRiRenderer::state::checkEndMotion() {
		--sampleCount;
		// Close a motion block when the counter reaches zero
		if( !sampleCount ) {
			debugMessage( L"UeberManRi: MotionEnd" );
			RiMotionEnd();
		}
		// We're outside of a motion block
		else if( 0 > sampleCount ) {
			sampleCount = -1;
		}
	}
}


