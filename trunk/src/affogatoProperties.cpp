/** Affogato XSI properties and resp. GUI layouts.
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
#include <math.h>

// XSI headers
#include <xsi_application.h>
#include <xsi_argument.h>
#include <xsi_context.h>
#include <xsi_command.h>
#include <xsi_model.h>
#include <xsi_ppgeventcontext.h>
#include <xsi_ppglayout.h>
#include <xsi_sceneitem.h>
#include <xsi_selection.h>
#include <xsi_griddata.h>

// Affogato headers
#include "affogato.hpp"
#include "affogatoHelpers.hpp"


using namespace affogato;
using namespace std;

/*
AffogatoShadowCameraGlobals
PixelSamples
Resolution
ShadingRate
CurveWithShading

	prop.AddParameter(	L"Type", CValue::siUInt1, caps,
						L"Type", CValue(),
						0l, 0l, 2l, 0l, 2l, param );

	prop.AddParameter(	L"Depth", CValue::siUInt1, caps,
						L"Depth", CValue(),
						0l, 0l, 3l, 0l, 3l, param );
	//Minimum
	//Midpoint
	//Maximum
	//Average



	prop.AddParameter(	L"SearchPath", CValue::siString, caps,
						L"Search Path", CValue(),
						CValue(), param );

	prop.AddParameter(	L"GeometryMotionBlur", CValue::siBool, caps,
						L"Geometry Motion Blur", CValue(),
						false, param );

	prop.AddParameter(	L"CameraMotionBlur", CValue::siBool, caps,
						L"Camera Motion Blur", CValue(),
						false, param );

	prop.AddParameter(	L"SearchPath", CValue::siString, caps,
						L"Search Path", CValue(),
						CValue(), param );

	prop.AddParameter(	L"Resolution", CValue::siUInt2,
						caps,
						L"Resolution", CValue(),
						1024l, 1l, 65535l, 1l, 4096l, param );

	prop.AddParameter(	L"PixelSamples", CValue::siUInt1, caps,
						L"Pixel Samples", CValue(),
						3l, 1l, 255l, 1l, 64l, param );

	prop.AddParameter(	L"Samples", CValue::siUInt1, caps,
						L"Samples", CValue(),
						3l, 1l, 255l, 1l, 64l, param );
*/

XSIPLUGINCALLBACK CStatus AffogatoOpenGlobals_Execute( const CRef& inContext) {

	Application app;
	SceneItem sceneRoot( app.GetActiveSceneRoot() );

	Property affogatoGlobals;

	bool noGlobals = true;
	CRefArray props( sceneRoot.GetProperties() );
	for( long k = 0; k < props.GetCount(); k++ ) {
		Property prop( props[ k ] );
		if( CString( L"AffogatoGlobals" ) == prop.GetType() ) {
			affogatoGlobals = prop;
			noGlobals = false;
			break;
		}
	}

	if( noGlobals )
		affogatoGlobals = sceneRoot.AddProperty( L"AffogatoGlobals" );

	CValueArray args( 5 );
	CValue retval;

	args[ 0 ] = affogatoGlobals.GetFullName();
	args[ 1 ] = CValue();
	args[ 2 ] = L"Affogato Globals";
	args[ 3 ] = 1l; //siInspectMode::siModal;
	args[ 4 ] = true;

	CStatus ist = app.ExecuteCommand( L"InspectObj", args, retval );

	return ist;
}

XSIPLUGINCALLBACK CStatus AffogatoCreateHUBProperty_Init( const CRef& inContext ) {
	Context ctxt( inContext);
	Command cmd( ctxt.GetSource() );

	Application app;
	app.LogMessage( L"Initalizing '" + cmd.GetName() + L"' command" );

	ArgumentArray args = cmd.GetArguments();
	args.Add( L"Target Object", CValue() );
	args.Add( L"Inspect Property", CValue() );

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus AffogatoCreateHUBProperty_Execute( const CRef& inContext ) {

	Application app;

	Context ctx( inContext );
	CValueArray args = ctx.GetAttribute( L"Arguments" );

	Selection selection( app.GetSelection() );

		/*SceneItem object;
		object = SceneItem( CRef( args[ 1 ] ) );	*/

	SceneItem object;
	object = SceneItem( CRef( args[ 0 ] ) );

	if( !object.IsValid() ) {
		Selection selected( app.GetSelection() );

		if( !selected.GetCount() ) {
			object = app.GetActiveSceneRoot();
		} else {
			object = selected[ 0 ];
		}
	}

	Property prop( object.AddProperty( L"AffogatoHUB" ) );

	if( ( bool )args[ 1 ] ) {
		CValueArray args( 5 );
		CValue retval;

		args[ 0 ] = prop.GetFullName();
		args[ 1 ] = CValue();
		args[ 2 ] = L"Affogato HUB Property";
		args[ 3 ] = 1l; //siInspectMode::siModal;
		args[ 4 ] = true;

		CStatus status( app.ExecuteCommand( L"InspectObj", args, retval ) );
	}

	return CStatus::OK;
}

#ifdef RSP
XSIPLUGINCALLBACK CStatus AffogatoHUB_Define( const CRef& inContext ) {

	Application app;
	CustomProperty prop( Context( inContext ).GetSource() );
	Parameter param;

	int caps( siPersistable );

	prop.AddParameter(	L"Type", CValue::siUInt1, caps,
						L"Type", CValue(),
						0l, 0l, 2l, 0l, 2l, param );
	prop.AddParameter(	L"Action", CValue::siUInt1, caps,
						L"Action", CValue(),
						0l, 0l, 3l, 0l, 3l, param );

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus AffogatoHUB_PPGEvent( const CRef& inContext ) {

	Application app;
	PPGEventContext ctx( inContext );
	PPGEventContext::PPGEvent eventID( ctx.GetEventID() );

	switch( eventID ) {
		case PPGEventContext::siOnInit: {
			CustomProperty prop( ctx.GetSource() );
			PPGLayout layout( prop.GetPPGLayout() );

			layout.Clear();
			layout.PutAttribute( siUIHelpFile, L"http://affogato.sf.net/" );

			CValueArray tmpArray;

			PPGItem item( layout.AddItem( L"Affogato HUB Parameters" ) );

			tmpArray.Clear();
			tmpArray.Add( L"RSP HDF5 HUB" );
			tmpArray.Add( 1l );
			tmpArray.Add( L"RSP SQlite Hair Database" );
			tmpArray.Add( 2l );
			item = layout.AddEnumControl( L"Type", tmpArray, L"Type", L"Combo" );
			item.PutLabelMinPixels( LABEL_WIDTH );

			tmpArray.Clear();
			tmpArray.Add( L"Off (Affogato handles Data)" );
			tmpArray.Add( 0l );
			tmpArray.Add( L"Always Write" );
			tmpArray.Add( 1l );
			tmpArray.Add( L"Only Write if Missing" );
			tmpArray.Add( 2l );
			tmpArray.Add( L"Only Reference, Do Not Write" );
			tmpArray.Add( 2l );
			item = layout.AddEnumControl( L"Action", tmpArray, L"Action", L"Combo" );
			item.PutLabelMinPixels( LABEL_WIDTH );

			ctx.PutAttribute( L"Refresh", true );
			break;
		}
		case PPGEventContext::siButtonClicked: {
			break;
		}
		case PPGEventContext::siParameterChange: {
			break;
		}
	}

	return CStatus::OK;
}
#endif

XSIPLUGINCALLBACK CStatus AffogatoEnvironmentMapGenerator_Define( const CRef& inContext ) {
	Application app;
	CustomProperty prop( Context( inContext ).GetSource() );
	Parameter param;

	int caps( siPersistable );

	prop.AddParameter(	L"Resolution", CValue::siUInt2, caps,
						L"Resolution", CValue(),
						512l, 1l, 65535l, 1l, 4096l, param );

	prop.AddParameter(	L"NearClip", CValue::siFloat, caps,
						L"Near Clip", CValue(),
						0.1f, 0.0f, 1.0e38f, 0.0f, 1000.0f, param );

	prop.AddParameter(	L"FarClip", CValue::siFloat, caps,
						L"Far Clip", CValue(),
						32768.0f, 0.0f, 1.0e38f, 0.0f, 100000.0f, param );

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus AffogatoGlobals_Define( const CRef& inContext ) {

	// Here is where we add all the parameters to the
	// Custom Property.  This will be called each time
	// an new instance of the Custom Property is called.
	// It is not called when an persisted Custom Property
	// is loaded.
	Application app;
	CustomProperty prop( Context( inContext ).GetSource() );
	Parameter param;

	debugMessage( L"Defining Globals" );

	// Default capabilities for most of these parameters
	int caps( siPersistable );
	CValue dft;	// Used for arguments we don't want to set

	// Frame Tab ------------------
	// Camera Group
	prop.AddParameter(	L"___AffogatoVersion", CValue::siString, siPersistable | siReadOnly,
						L"Affogato Version", CValue(),
						stringToCString( AFFOGATOVERSION ), param );

	prop.AddParameter(	L"CameraName", CValue::siString, caps,
						L"Camera Name", CValue(),
						dft, param );

	prop.AddParameter(	L"UseCameraAspect", CValue::siBool, caps,
						L"Use Camera Aspect", CValue(),
						false, param );

	prop.AddParameter(	L"FrontPlane", CValue::siBool, caps,
						L"Front Plane", CValue(),
						false, param );

	prop.AddParameter(	L"BackPlane", CValue::siBool, caps,
						L"Back Plane", CValue(),
						false, param );

	prop.AddParameter(	L"DepthOfField", CValue::siBool, caps,
						L"Depth of Field", CValue(),
						false, param );

	prop.AddParameter(	L"FocusFactor", CValue::siFloat, siReadOnly,
						L"Focus Factor", CValue(),
						1.0, 0.0, 1024.0, 0.0, 32.0, param );

	prop.AddParameter(	L"CameraFreezeScale", CValue::siBool, caps,
						L"Freeze Camera Scale", CValue(),
						true, param );

	prop.AddParameter(	L"CameraRotoViewStyle", CValue::siUInt1, caps,
						L"Camera Rotoscope View Style", CValue(),
						0l, param );

	// Image Group
	prop.AddParameter(	L"DisplayDriver", CValue::siString, caps,
						L"Display Driver", CValue(),
						L"tiff", param );

	prop.AddParameter(	L"DisplayQuantization", CValue::siUInt1, caps,
						L"Display Quantization", CValue(),
						1l, param );

	prop.AddParameter(	L"HypeOverscan", CValue::siBool, caps,
						L"Hype Overscan", CValue(),
#ifdef RSP
						true,
#else
						false,
#endif
						param );

	prop.AddParameter(	L"ResolutionX", CValue::siUInt2,
#ifdef RSP
						siReadOnly,
#else
						caps,
#endif
						L"Resolution X", CValue(),
						1024l, 1l, 65535l, 1l, 4096l, param );

	prop.AddParameter(	L"ResolutionY", CValue::siUInt2,
#ifdef RSP
						siReadOnly,
#else
						caps,
#endif
						L"Resolution Y", CValue(),
						768l, 1l, 65535l, 1l, 4096l, param );

	prop.AddParameter(	L"PixelAspect", CValue::siFloat, caps,
						L"Pixel Aspect", CValue(),
						1.0, 0.001, 4.0, 0.0, 20.0, param );

	prop.AddParameter(	L"ResolutionMultiplier", CValue::siFloat, caps,
						L"Resolution Multiplier", CValue(),
						1.0, 0.01, 1.0, 0.01, 1.0, param );

	// Animation Group
	prop.AddParameter(	L"FrameOutput", CValue::siUInt1, caps,
						L"Frame Output", CValue(),
						0l, 0l, 3l, 0l, 3l, param ) ;

#ifdef RSP // For backwards-compatibility with old pipeline tools
	prop.AddParameter(	L"StartFrame", CValue::siInt4, caps,
						L"Start Frame", CValue(),
						1l, -65536l, 65536l, 1l, 1000l, param );

	prop.AddParameter(	L"EndFrame", CValue::siInt4, caps,
						L"End Frame", CValue(),
						100l, -65536l, 65536l, 1l, 1000l, param );

	prop.AddParameter(	L"FrameStep", CValue::siUInt2, caps,
						L"Frame Step", CValue(),
						1l, 1l, 100l, 1l, 1000l, param );
#endif

	prop.AddParameter(	L"Frames", CValue::siString, siReadOnly,
						L"Frame Sequence", CValue(),
						L"1-100@1", param );

	// Motion Blur & DOF Group
	prop.AddParameter(	L"ShutterAngle", CValue::siFloat, caps,
						L"Shutter Angle", CValue(),
						180.0, 0.0, 360.0, 0.0, 360.0, param );

	prop.AddParameter(	L"TransformationMotionSegments", CValue::siUInt1, caps,
						L"Transformation Motion Segments", CValue(),
						1l, 0l, 15l, 0l, 15l, param );

	prop.AddParameter(	L"DeformationMotionSegments", CValue::siUInt1, caps,
						L"Deformation Motion Segments", CValue(),
						1l, 0l, 15l, 0l, 15l, param );

	prop.AddParameter(	L"MotionFactor", CValue::siFloat, caps,
						L"Motion Factor", CValue(),
						2.0, 0.0, 1024.0, 0.0, 32.0, param );

	prop.AddParameter(	L"ShutterTiming", CValue::siUInt1, caps,
						L"Shutter Timing", CValue(),
						0l, 0l, 3l, 0l, 3l, param );

	prop.AddParameter(	L"ShutterConfiguration", CValue::siUInt1, caps,
						L"Shutter Configuration", CValue(),
						0l, 0l, 1l, 0l, 1l, param );

	prop.AddParameter(	L"ShutterOffset", CValue::siFloat, caps,
						L"Shutter Offset", CValue(),
						0.0, -1e38f, 1e38f, -10.0, 10.0, param );

	prop.AddParameter(	L"ShutterAngle", CValue::siFloat, caps,
						L"Shutter Angle", CValue(),
						180.0, 0.0, 360.0, 0.0, 360.0, param );

	prop.AddParameter(	L"ShutterEfficiency", CValue::siFloat, caps,
						L"Shutter Efficiency", CValue(),
						0.5, 0.5, 1.0, 0.5, 1.0, param );

	prop.AddParameter(	L"GeometryMotionBlur", CValue::siBool, caps,
						L"Geometry Motion Blur", CValue(),
						true, param );

	prop.AddParameter(	L"GeometryParameterMotionBlur", CValue::siBool, caps,
						L"Geometry Parameter Motion Blur", CValue(),
						true, param );

	prop.AddParameter(	L"GeometryVariableMotionBlur", CValue::siBool, caps,
						L"Geometry Variable Motion Blur", CValue(),
						false, param );

	prop.AddParameter(	L"CameraMotionBlur", CValue::siBool, caps,
						L"Camera Motion Blur", CValue(),
						true, param );

	prop.AddParameter(	L"LightMotionBlur", CValue::siBool, caps,
						L"Light Motion Blur", CValue(),
						false, param );

	prop.AddParameter(	L"AttributeMotionBlur", CValue::siBool, caps,
						L"Attribute Motion Blur", CValue(),
						false, param );

	prop.AddParameter(	L"ShaderMotionBlur", CValue::siBool, caps,
						L"Shader Motion Blur", CValue(),
						false, param );

	prop.AddParameter(	L"ShadowMapMotionBlur", CValue::siBool, caps,
						L"Shadow Map Motion Blur", CValue(),
						false, param );

	prop.AddParameter(	L"SubFrameMotionBlur", CValue::siBool, caps,
						L"Sub Frame Motion Blur", CValue(),
						true, param );

/*
	// Display Tab -------------------
	GridData displayGrid( Parameter( prop.AddGridParameter( L"Displays" ) ).GetValue() );
    displayGrid.PutColumnCount( 5 ) ;
    displayGrid.PutRowCount( 1 ) ;
	displayGrid.PutColumnLabel( 0, L"Name" );
    displayGrid.PutColumnLabel( 1, L"Type" );
	CValueArray tmpArray;
	tmpArray.Add( L"Open EXR" );
	tmpArray.Add( L"exr" );
	tmpArray.Add( L"TIFF" );
	tmpArray.Add( L"tiff" );
	tmpArray.Add( L"Cineon" );
	tmpArray.Add( L"cineon" );
	tmpArray.Add( L"Radiance" );
	tmpArray.Add( L"radiance" );
	tmpArray.Add( L"EPS" );
	tmpArray.Add( L"eps" );
	tmpArray.Add( L"Z File" );
	tmpArray.Add( L"zfile" );
	displayGrid.PutColumnComboItems( 1, tmpArray );
    displayGrid.PutColumnLabel( 2, L"Channels" );
	displayGrid.PutColumnLabel( 3, L"Compute Alpha" );
	displayGrid.PutColumnType ( 3, siColumnBool );
	displayGrid.PutColumnLabel( 4, L"Premultiply" );
	displayGrid.PutColumnType ( 4, siColumnBool );

	// Channels Tab
	GridData channelGrid( Parameter( prop.AddGridParameter( L"Channels" ) ).GetValue() );
    channelGrid.PutColumnCount( 7 ) ;
    channelGrid.PutRowCount( 1 ) ;
	channelGrid.PutColumnLabel( 0, L"Name" ) ;
    channelGrid.PutColumnLabel( 1, L"Filter" ) ;
    channelGrid.PutColumnLabel( 2, L"Filter Width" );
	channelGrid.PutColumnLabel( 3, L"Filter Height" );
	channelGrid.PutColumnLabel( 4, L"Quantize" );
	tmpArray.Clear();
	tmpArray.Add( L"Float" );
	tmpArray.Add( 0l );
	tmpArray.Add( L"8 Bit" );
	tmpArray.Add( 1l );
	tmpArray.Add( L"16 Bit" );
	tmpArray.Add( 2l );
	tmpArray.Add( L"16 Bit, White Point @ 1k" );
	tmpArray.Add( 3l );
	tmpArray.Add( L"16 Bit, White Point @ 2k" );
	tmpArray.Add( 4l );
	tmpArray.Add( L"16 Bit, White Point @ 4k" );
	tmpArray.Add( 5l );
	channelGrid.PutColumnComboItems( 5, tmpArray );
	channelGrid.PutColumnLabel( 5, L"Exlusive" );
	channelGrid.PutColumnType ( 5, siColumnBool );
	channelGrid.PutColumnLabel( 6, L"Matted" );
	channelGrid.PutColumnType ( 6, siColumnBool );
*/


	// File Tab -------------------
	// RIB Name Group
	prop.AddParameter(	L"BaseName", CValue::siString, caps,
						L"Base Name", CValue(),
						L"untitled", param );

	// Directories Group
	prop.AddParameter(	L"BaseDir", CValue::siString, caps,
						L"Base Directory", CValue(),
						CValue(), param );

	prop.AddParameter(	L"RelativePaths", CValue::siBool, caps,
						L"Relative Paths", CValue(),
						false, param );

	prop.AddParameter(	L"ImageDir", CValue::siString, caps,
						L"Images Directory", CValue(),
						dft, param );

	prop.AddParameter(	L"MapDir", CValue::siString, caps,
						L"Maps Directory", CValue(),
						dft, param );

	prop.AddParameter(	L"DataDir", CValue::siString, caps,
						L"Data Directory", CValue(),
						dft, param );

	prop.AddParameter(	L"HubDir", CValue::siString, caps,
						L"Hub Directory", CValue(),
						dft, param );

	prop.AddParameter(	L"ObjectDataDir", CValue::siString, caps,
						L"Object Directory", CValue(),
						CValue(), param );

	prop.AddParameter(	L"AttributeDataDir", CValue::siString, caps,
						L"Attributes Directory", CValue(),
						CValue(), param );

	prop.AddParameter(	L"TempDir", CValue::siString, caps,
						L"Temporary Files Directory", CValue(),
						dft, param );

	prop.AddParameter(	L"CreateDirs", CValue::siBool, caps,
						L"Create Missing Directories", CValue(),
						false, param );

	// Search Paths Group
	prop.AddParameter(	L"ShaderPath", CValue::siString, caps,
						L"Shader Searchpath", CValue(),
						dft, param );

	prop.AddParameter(	L"TexturePath", CValue::siString, caps,
						L"Texture Searchpath", CValue(),
						dft, param );

	prop.AddParameter(	L"ArchivePath", CValue::siString, caps,
						L"Archive Searchpath", CValue(),
						dft, param );

	prop.AddParameter(	L"ProceduralPath", CValue::siString, caps,
						L"Procedural Searchpath", CValue(),
						dft, param );

	prop.AddParameter(	L"ShaderPaths", CValue::siBool, caps,
						L"Output Shader Paths", CValue(),
						true, param );

	// Image Tab ------------------
	// Shading Group
	prop.AddParameter(	L"ShadingRate", CValue::siFloat, caps,
						L"Shading Rate", CValue(),
						1.0, 0.001, 500.0, 0.01, 100.0, param );

	prop.AddParameter(	L"ShadingRateMultiplier", CValue::siFloat, caps,
						L"Shading Rate Multiplier", CValue(),
						1.0, 0.001, 500.0, 0.01, 100.0, param );

	prop.AddParameter(	L"ShadowShadingRate", CValue::siFloat, caps,
						L"Shadow Shading Rate", CValue(),
						3.0, 0.001, 500.0, 0.01, 100.0, param );

	prop.AddParameter(	L"SmoothShading", CValue::siBool, caps,
						L"Smooth Shading", CValue(),
						true, param );

	prop.AddParameter(	L"CurveWidthShading", CValue::siUInt1, caps,
						L"Curve Width Shading", CValue(),
						0l, 0l, 10l, 0l, 10l, param );

	// Sampling & Filtering Group
	prop.AddParameter(	L"PixelSamplesX", CValue::siUInt1, caps,
						L"Pixel Samples X", CValue(),
						3l, 1l, 255l, 1l, 64l, param );

	prop.AddParameter(	L"PixelSamplesY", CValue::siUInt1, caps,
						L"Pixel Samples Y", CValue(),
						3l, 1l, 255l, 1l, 64l, param );

	prop.AddParameter(	L"PixelFilter", CValue::siString, caps,
						L"Pixel Filter", CValue(),
						L"bessel", param );

	prop.AddParameter(	L"PixelFilterX", CValue::siFloat, caps,
						L"Pixel Filter Width", CValue(),
						6.47666, 1.0, 32.0, 1.0, 16.0, param );

	prop.AddParameter(	L"PixelFilterY", CValue::siFloat, caps,
						L"Pixel Filter Height", CValue(),
						6.47666, 1.0, 32.0, 1.0, 16.0, param );

	prop.AddParameter(	L"AssociateAlpha", CValue::siBool, caps,
						L"Associate Alpha", CValue(),
						true, param );

	prop.AddParameter(	L"ExtremeMotionDepthOfField", CValue::siBool, siReadOnly,
						L"Extreme Motion Depth of Field", CValue(),
						false, param );

	prop.AddParameter(	L"Gain", CValue::siFloat, caps,
						L"Gain", CValue(),
						1.0, 0.001, 100.0, 0.1, 2.0, param );

	prop.AddParameter(	L"Gamma", CValue::siFloat, caps,
						L"Gamma", CValue(),
						1.0, 0.001, 100.0, 0.1, 3.2, param );

    // REYES Group

	prop.AddParameter(	L"SampleMotion", CValue::siBool, caps,
						L"Sample Motion", CValue(),
						true, param );

	prop.AddParameter(	L"SampleJitter", CValue::siBool, caps,
						L"Sample Jitter", CValue(),
						true, param );

	prop.AddParameter(	L"EyeSplits", CValue::siUInt1, caps,
						L"Eye Splits", CValue(),
						10l, 1l, 255l, 1l, 32l, param );

	prop.AddParameter(	L"GeometrySplits", CValue::siUInt1, caps,
						L"Geometry Splits", CValue(),
						4l, 1l, 255l, 1l, 32l, param );

	prop.AddParameter(	L"BucketOrder", CValue::siString, caps,
						L"Bucket Order", CValue(),
						L"horizontal", param );

	prop.AddParameter(	L"BucketX", CValue::siUInt2, caps,
						L"Bucket Width", CValue(),
						16l, 1l, 512l, 1l, 256l, param );

	prop.AddParameter(	L"BucketY", CValue::siUInt2, caps,
						L"Bucket Height", CValue(),
						16l, 1l, 512l, 1l, 256l, param );

	prop.AddParameter(	L"GridSize", CValue::siUInt4, caps,
						L"Grid Size", CValue(),
						256l, 1l, 262144l, 1l, 65536l, param );

	prop.AddParameter(	L"TextureMemory", CValue::siUInt4, caps,
						L"Texture Memory", CValue(),
						64l, 1l, 8129l, 1l, 1024l, param );

	prop.AddParameter(	L"OpacityThreshold", CValue::siFloat, caps,
						L"Opacity Threshold", CValue(),
						0.996, 0.0, 1.0, 0.1, 1.0, param );

	// Rays tab -------------------

	// Global group
	prop.AddParameter(	L"RayTracing", CValue::siBool, caps,
						L"Ray Tracing", CValue(),
						true, param );

	// Trace Group
	prop.AddParameter(	L"RayDepth", CValue::siUInt2, caps,
						L"Ray Depth", CValue(),
						1l, 1l, 64l, 1l, 64l, param );

	prop.AddParameter(	L"TraceBias", CValue::siFloat, caps,
						L"Trace Bias", CValue(),
						0.01, 0.001, 1.0, 0.001, 1.0, param );

	prop.AddParameter(	L"TraceDisplacements", CValue::siBool, caps,
						L"Trace Displacements", CValue(),
						false, param );

	prop.AddParameter(	L"TraceMotion", CValue::siBool, caps,
						L"Trace Motion", CValue(),
						false, param );

	// Irradiance Group
	prop.AddParameter(	L"IrradianceSamples", CValue::siUInt2, caps,
						L"Irradiance Samples", CValue(),
						64l, 1l, 1024l, 1l, 256l, param );

	prop.AddParameter(	L"IrradianceShadingRate", CValue::siUInt2, caps,
						L"Irradiance Shading Rate", CValue(),
						64l, 1l, 16384l, 1l, 16384l, param );

	// Subsurface Group
	prop.AddParameter(	L"SubSurfaceShadingRate", CValue::siFloat, caps,
						L"Subsurface Shading Rate", CValue(),
						8.0, 0.01, 100.0, 0.001, 500.0, param );

	// Options Tab ----------------

	// Renderding
	prop.AddParameter(	L"DirectToRenderer", CValue::siBool, caps,
						L"Direct To Renderer", CValue(),
						false, param );

	// RIB/Data Group

	prop.AddParameter(	L"DataGranularity", CValue::siUInt1, caps,
						L"Data Granularity", CValue(),
						1l, 0l, 4l, 0l, 4l, param );

	prop.AddParameter(	L"TransformsInParentData", CValue::siBool, caps,
						L"Put Geometry Transforms in Parent Block", CValue(),
						true, param );

	prop.AddParameter(	L"RelativeTransforms", CValue::siBool, caps,
						L"Relative Transforms", CValue(),
						false, param );

	prop.AddParameter(	L"DelayData", CValue::siBool, caps,
						L"Delay Data", CValue(),
						true, param );

	prop.AddParameter(	L"AttributeDataType", CValue::siUInt1, caps,
						L"Attribute Data Type", CValue(),
						0l, 0l, 1l, 0l, 1l, param );

	prop.AddParameter(	L"WriteBinaryData", CValue::siBool, caps,
						L"Write Binary Data", CValue(),
						true, param );

	prop.AddParameter(	L"CompressData", CValue::siBool, caps,
						L"Compress Data", CValue(),
						false, param );

#ifdef RSP
	prop.AddParameter(	L"HubSupport", CValue::siBool, caps,
						L"HUB Support", CValue(),
						true, param );
#endif

	prop.AddParameter(	L"HierarchicalDataScanning", CValue::siBool, caps,
						L"Hierarchical Data Scanning", CValue(),
						false, param );

	prop.AddParameter(	L"AttributeDataScanningOrder", CValue::siUInt1, caps,
						L"Attribute Data Scanning Order", CValue(),
						0l, 0l, 1l, 0l, 1l, param );

	prop.AddParameter(	L"FrameData", CValue::siBool, caps,
						L"Output Frame Data", CValue(),
						true, param );

	prop.AddParameter(	L"OptionsData", CValue::siBool, caps,
						L"Output Options Data", CValue(),
						true, param );

	prop.AddParameter(	L"CameraData", CValue::siBool, caps,
						L"Output Camera Data", CValue(),
						true, param );

	prop.AddParameter(	L"WorldData", CValue::siBool, caps,
						L"Output World Data", CValue(),
						true, param );

	prop.AddParameter(	L"SpacesData", CValue::siBool, caps,
						L"Output Spaces Data", CValue(),
						true, param );

	prop.AddParameter(	L"LightsData", CValue::siBool, caps,
						L"Output Lights Data", CValue(),
						true, param );

	prop.AddParameter(	L"LooksData", CValue::siBool, caps,
						L"Output Look Data", CValue(),
						true, param );

	prop.AddParameter(	L"GeometryData", CValue::siBool, caps,
						L"Output Geometry Data", CValue(),
						true, param );

	prop.AddParameter(	L"AttributeData", CValue::siBool, caps,
						L"Output Geometry Data", CValue(),
						true, param );

	prop.AddParameter(	L"ShaderParameterData", CValue::siBool, caps,
						L"Output Shader Parameters", CValue(),
						true, param );

	prop.AddParameter(	L"ShaderNumericParameterData", CValue::siBool, caps,
						L"Output Numeric Shader Parameters", CValue(),
						true, param );

	prop.AddParameter(	L"ShadowData", CValue::siBool, caps,
						L"Output Shadow Data", CValue(),
						true, param );

	prop.AddParameter(	L"NormalizeNurbKnotVector", CValue::siBool, caps,
						L"Normalize Nurb Knot Vector", CValue(),
						true, param );

	prop.AddParameter(	L"NonRationalNurbSurface", CValue::siBool, caps,
						L"Non-Rational Nurb Surfaces", CValue(),
						true, param );

	prop.AddParameter(	L"NonRationalNurbCurve", CValue::siBool, caps,
						L"Non-Rational Nurb Curves", CValue(),
						true, param );

	prop.AddParameter(	L"NurbCurveWidth", CValue::siFloat, caps,
						L"Nurb Curve Width", CValue(),
						0.1, 0.0001, 1000.0, 0.001, 100.0, param );

	prop.AddParameter(	L"RenderCache", CValue::siUInt1, caps,
						L"Render Cache", CValue(),
						1l, 0l, 4l, 0l, 4l, param );

	prop.AddParameter(	L"RenderCacheDir", CValue::siString, caps,
						L"Render Cache Dir", CValue(),
						L"/var/tmp/3delightCache/", param );

	prop.AddParameter(	L"RenderCacheSize", CValue::siUInt1, caps,
						L"Render Cache Size", CValue(),
						2l, 1l, 255l, 1l, 16l, param );

	// Renderer Group
	prop.AddParameter(	L"RenderCommand", CValue::siString, caps,
						L"Render Command", CValue(),
						L"renderdl", param );

	/*prop.AddParameter(	L"ShadowRIBGranularity", CValue::siUInt1, caps,
						L"Shadow RIB Granularity", CValue(),
						1l, 0l, 3l, 0l, 3l, param );*/

	// Feedback Group
	prop.AddParameter(	L"PreviewDisplay", CValue::siUInt1, caps,
						L"Preview Display Type", CValue(),
						1l, 0l, 4l, 0l, 4l, param );

	prop.AddParameter(	L"ShaderDebugging", CValue::siBool, caps,
						L"Shader Debugging", CValue(),
						false, param );

	prop.AddParameter(	L"VerbosityLevel", CValue::siUInt1, caps,
						L"Verbosity Level", CValue(),
						1l, 0l, 4l, 0l, 4l, param );

	prop.AddParameter(	L"Stopwatch", CValue::siBool, caps,
						L"Stopwatch", CValue(),
						false, param );

	// Default Shader
	prop.AddParameter(	L"DefaultSurfaceShader", CValue::siString, caps,
						L"Default Surface Shader", CValue(),
						L"plastic", param );

	prop.AddParameter(	L"DefaultDisplacementShader", CValue::siString, caps,
						L"Default Displacement Shader", CValue(),
						CValue(), param );

	prop.AddParameter(	L"DefaultVolumeShader", CValue::siString, caps,
						L"Default Volume Shader", CValue(),
						CValue(), param );

	prop.AddParameter(	L"OverrideAllShaders", CValue::siBool, caps,
						L"Override All Shaders", CValue(),
						false, param );

	// Baking
	prop.AddParameter(	L"BakeMode", CValue::siBool, caps,
						L"Bake Mode", CValue(),
						false, param );

	// Job Tab ----------------
	prop.AddParameter(	L"LaunchType", CValue::siUInt1, caps,
						L"Launch Type", CValue(),
						1l, 0l, 2l, 0l, 2l, param );

	prop.AddParameter(	L"LaunchSubJobs", CValue::siBool, caps,
						L"Launch Subjobs", CValue(),
						true, param );

	prop.AddParameter(	L"JobScriptInterpreter", CValue::siString, siReadOnly,
						L"Job Script Interpreter", CValue(),
#ifdef RSP
						L"relaxjob"
#else
						CValue()
#endif
						, param );

	prop.AddParameter(	L"JobScriptFormat", CValue::siUInt1, caps,
						L"Job Script Format", CValue(),
						2l, 0l, 3l, 0l, 3l, param );

	prop.AddParameter(	L"JobScriptChunkSize", CValue::siUInt2, caps,
						L"Job Script Chunk Size", CValue(),
						0l, 0l, 100l, 0l, 100l, param );

	prop.AddParameter(	L"JobBlockName", CValue::siString, caps,
						L"Job Block Name", CValue(),
						CValue(), param );

	prop.AddParameter(	L"PreJobCommand", CValue::siString, caps,
						L"Pre Job Command", CValue(),
						CValue(), param );

	prop.AddParameter(	L"PreFrameCommand", CValue::siString, caps,
						L"Pre Job Command", CValue(),
						CValue(), param );

	prop.AddParameter(	L"PostJobCommand", CValue::siString, caps,
						L"Post Frame Command", CValue(),
						CValue(), param );

	prop.AddParameter(	L"PostFrameCommand", CValue::siString, caps,
						L"Post Frame Command", CValue(),
						CValue(), param );

	prop.AddParameter(	L"CacheDir", CValue::siString, caps,
						L"Cache Directory", CValue(),
						CValue(), param );

	prop.AddParameter(	L"CacheSize", CValue::siUInt2, caps,
						L"Cache Size", CValue(),
						2l, 0l, 65535l, 0l, 16l, param );

	prop.AddParameter(  L"CacheWriteData", CValue::siBool, caps,
						L"Write Data To Cache", CValue(),
						false, param );

	prop.AddParameter(  L"CacheWriteMap", CValue::siBool, caps,
						L"Write Maps To Cache", CValue(),
						false, param );

	prop.AddParameter(  L"CacheWriteImage", CValue::siBool, caps,
						L"Write Images To Cache", CValue(),
						false, param );

	prop.AddParameter(  L"CacheCopyMap", CValue::siBool, caps,
						L"Copy Maps From Cache", CValue(),
						false, param );

	prop.AddParameter(  L"CacheCopyData", CValue::siBool, caps,
						L"Copy Data From Cache", CValue(),
						false, param );

	prop.AddParameter(  L"CacheCopyImage", CValue::siBool, caps,
						L"Copy Images From Cache", CValue(),
						false, param );

	prop.AddParameter(  L"CacheSourceData", CValue::siBool, caps,
						L"Source Maps From Cache", CValue(),
						false, param );

	prop.AddParameter(  L"CacheSourceMap", CValue::siBool, caps,
						L"Source Maps From Cache", CValue(),
						false, param );

	prop.AddParameter(	L"Threads", CValue::siUInt1, caps,
						L"Number of Threads", CValue(),
						0l, 0l, 16l, 0l, 16l, param );

	prop.AddParameter(	L"RemoteRenderHosts", CValue::siString, caps,
						L"Remote Render Hosts", CValue(),
						CValue(), param );

	prop.AddParameter(  L"UseSSHForRemote", CValue::siBool, caps,
						L"Use SSH to Connect to Remote Render Hosts", CValue(),
						true, param );


	// Baking Tab -------------

	return CStatus::OK;
}

bool updateAffogatoGlobalsState( CustomProperty& prop, const Parameter& changed ) {
	bool refresh( false );
	CString paramName( changed.GetScriptName() );

	if( CString( L"CameraName" ) == paramName ) {
		PPGLayout layout( prop.GetPPGLayout() );

		PPGItem item( layout.GetItem( L"CameraName" ) );
		CValueArray tmpArray;
		Application app;
		Model sceneRoot( app.GetActiveSceneRoot() );
		CRefArray cameras( sceneRoot.FindChildren( CValue(), siCameraPrimType, CStringArray() ) );
		for( unsigned i = 0; i < ( unsigned )cameras.GetCount(); i++ ) {
			CValue name( ( SceneItem( cameras[ i ] ) ).GetFullName() );
			tmpArray.Add( name );
			tmpArray.Add( name );
		}
		item.PutUIItems( tmpArray );

		refresh = true;
	} else
	if( CString( L"DepthOfField" ) == paramName ) {
		if( changed.GetValue() ) {
			Parameter param( prop.GetParameters().GetItem( L"FocusFactor" ) );
			param.PutCapabilityFlag( siReadOnly, false );
			param = prop.GetParameters().GetItem( L"ExtremeMotionDepthOfField" );
			param.PutCapabilityFlag( siReadOnly, false );
		} else {
			Parameter param( prop.GetParameters().GetItem( L"FocusFactor" ) );
			param.PutCapabilityFlag( siReadOnly, true );
			param = prop.GetParameters().GetItem( L"ExtremeMotionDepthOfField" );
			param.PutCapabilityFlag( siReadOnly, true );
		}
	} else
	if( CString( L"FrameOutput" ) == paramName ) {
		Parameter param( prop.GetParameters().GetItem( L"Frames" ) );
		switch( ( long )changed.GetValue() ) {
			case 0: // Current
				param.PutCapabilityFlag( siReadOnly, true );
				break;
			case 3: // Frame Sequence
				param.PutCapabilityFlag( siReadOnly, false );
				break;
		}
		refresh = true;
	} else
#ifdef RSP
	if( CString( L"Frames" ) == paramName ) {
		string frames( CStringToString( changed.GetValue() ) );
		vector< float > frameSeq( getSequence( frames ) );
		sort( frameSeq.begin(), frameSeq.end() );
		prop.PutParameterValue( L"StartFrame", ( short )floor( frameSeq.front() ) );
		prop.PutParameterValue( L"EndFrame", ( short )ceil( frameSeq.back() ) );
		prop.PutParameterValue( L"FrameStep", ( short )1 );
	} else
#endif
	if( CString( L"ShaderPath" ) == paramName ) {
		const_cast< Parameter& >( changed ).PutValue( stringToCString( sanitizeWindowsMultiPath( CStringToString( changed.GetValue() ) ) ) );
		refresh = true;
	} else
	if( CString( L"TexturePath" ) == paramName ) {
		const_cast< Parameter& >( changed ).PutValue( stringToCString( sanitizeWindowsMultiPath( CStringToString( changed.GetValue() ) ) ) );
		refresh = true;
	} else
	if( CString( L"ArchivePath" ) == paramName ) {
		const_cast< Parameter& >( changed ).PutValue( stringToCString( sanitizeWindowsMultiPath( CStringToString( changed.GetValue() ) ) ) );
		refresh = true;
	} else
	if( CString( L"ProceduralPath" ) == paramName ) {
		const_cast< Parameter& >( changed ).PutValue( stringToCString( sanitizeWindowsMultiPath( CStringToString( changed.GetValue() ) ) ) );
		refresh = true;
	} else
	if( CString( L"HypeOverscan" ) == paramName ) {
		if( changed.GetValue() ) {
			Parameter param( prop.GetParameters().GetItem( L"ResolutionX" ) );
			param.PutCapabilityFlag( siReadOnly, true );
			param = prop.GetParameters().GetItem( L"ResolutionY" );
			param.PutCapabilityFlag( siReadOnly, true );
		} else {
			Parameter param( prop.GetParameters().GetItem( L"ResolutionX" ) );
			param.PutCapabilityFlag( siReadOnly, false );
			param = prop.GetParameters().GetItem( L"ResolutionY" );
			param.PutCapabilityFlag( siReadOnly, false );
		}
		refresh = true;
	} else
	if( CString( L"RayTracing" ) == paramName ) {
		if( changed.GetValue() ) {
			Parameter param( prop.GetParameters().GetItem( L"RayDepth" ) );
			param.PutCapabilityFlag( siReadOnly, false );
			param = prop.GetParameters().GetItem( L"TraceBias" );
			param.PutCapabilityFlag( siReadOnly, false );
			param = prop.GetParameters().GetItem( L"TraceDisplacements" );
			param.PutCapabilityFlag( siReadOnly, false );
			param = prop.GetParameters().GetItem( L"TraceMotion" );
			param.PutCapabilityFlag( siReadOnly, false );
			param = prop.GetParameters().GetItem( L"IrradianceSamples" );
			param.PutCapabilityFlag( siReadOnly, false );
			param = prop.GetParameters().GetItem( L"IrradianceShadingRate" );
			param.PutCapabilityFlag( siReadOnly, false );
		} else {
			Parameter param( prop.GetParameters().GetItem( L"RayDepth" ) );
			param.PutCapabilityFlag( siReadOnly, true );
			param = prop.GetParameters().GetItem( L"TraceBias" );
			param.PutCapabilityFlag( siReadOnly, true );
			param = prop.GetParameters().GetItem( L"TraceDisplacements" );
			param.PutCapabilityFlag( siReadOnly, true );
			param = prop.GetParameters().GetItem( L"TraceMotion" );
			param.PutCapabilityFlag( siReadOnly, true );
			param = prop.GetParameters().GetItem( L"IrradianceSamples" );
			param.PutCapabilityFlag( siReadOnly, true );
			param = prop.GetParameters().GetItem( L"IrradianceShadingRate" );
			param.PutCapabilityFlag( siReadOnly, true );
		}
		refresh = true;
	} else
	if( CString( L"LaunchType" ) == paramName ) {
		switch( ( long )changed.GetValue() ) {
			case 0: { // Off
				Parameter param( prop.GetParameters().GetItem( L"LaunchSubJobs" ) );
				param.PutCapabilityFlag( siReadOnly, true );
				param = prop.GetParameters().GetItem( L"JobScriptInterpreter" );
				param.PutCapabilityFlag( siReadOnly, true );
				break;
			}
			case 1: { // Renderer
				Parameter param( prop.GetParameters().GetItem( L"LaunchSubJobs" ) );
				param.PutCapabilityFlag( siReadOnly, false );
				param = prop.GetParameters().GetItem( L"JobScriptInterpreter" );
				param.PutCapabilityFlag( siReadOnly, true );
				break;
			}
			case 2: { // Job Interpreter
				Parameter param( prop.GetParameters().GetItem( L"LaunchSubJobs" ) );
				param.PutCapabilityFlag( siReadOnly, false );
				param = prop.GetParameters().GetItem( L"JobScriptInterpreter" );
				param.PutCapabilityFlag( siReadOnly, false );
				break;
			}
		}
		refresh = true;
	} else
	if( CString( L"DirectToRenderer" ) == paramName ) {
		if( changed.GetValue() ) {
			Parameter param( prop.GetParameters().GetItem( L"LaunchType" ) );
			param.PutCapabilityFlag( siReadOnly, true );
			param = prop.GetParameters().GetItem( L"LaunchSubJobs" );
			param.PutCapabilityFlag( siReadOnly, true );
			param = prop.GetParameters().GetItem( L"JobScriptInterpreter" );
			param.PutCapabilityFlag( siReadOnly, true );
			param = prop.GetParameters().GetItem( L"ShadowData" );
			param.PutCapabilityFlag( siReadOnly, true );
		} else {
			Parameter param( prop.GetParameters().GetItem( L"LaunchType" ) );
			param.PutCapabilityFlag( siReadOnly, false );
			param = prop.GetParameters().GetItem( L"ShadowData" );
			param.PutCapabilityFlag( siReadOnly, false );

			switch( ( long )prop.GetParameterValue( L"LaunchType" ) ) {
				case 0: { // Off
					Parameter param( prop.GetParameters().GetItem( L"LaunchSubJobs" ) );
					param.PutCapabilityFlag( siReadOnly, true );
					param = prop.GetParameters().GetItem( L"JobScriptInterpreter" );
					param.PutCapabilityFlag( siReadOnly, true );
					break;
				}
				case 1: { // Renderer
					Parameter param( prop.GetParameters().GetItem( L"LaunchSubJobs" ) );
					param.PutCapabilityFlag( siReadOnly, false );
					param = prop.GetParameters().GetItem( L"JobScriptInterpreter" );
					param.PutCapabilityFlag( siReadOnly, true );
					break;
				}
				case 2: { // Job Interpreter
					Parameter param( prop.GetParameters().GetItem( L"LaunchSubJobs" ) );
					param.PutCapabilityFlag( siReadOnly, false );
					param = prop.GetParameters().GetItem( L"JobScriptInterpreter" );
					param.PutCapabilityFlag( siReadOnly, false );
					break;
				}
			}
		}
		refresh = true;
	} else
	if( CString( L"DisplayDriver" ) == paramName ) {
		string display( CStringToString( changed.GetValue() ) );
		if( ( "exr" == display ) || ( "cineon" == display ) || ( "radiance" == display ) || ( "zfile" == display ) ) {
			prop.PutParameterValue( L"DisplayQuantization", 0l );
			Parameter param( prop.GetParameters().GetItem( L"DisplayQuantization" ) );
			param.PutCapabilityFlag( siReadOnly, true );
		} else {
			Parameter param( prop.GetParameters().GetItem( L"DisplayQuantization" ) );
			param.PutCapabilityFlag( siReadOnly, false );
		}
		refresh = true;
	} else
	if( CString( L"CacheWriteData" ) == paramName ) {
		if( changed.GetValue() ) {
			Parameter param( prop.GetParameters().GetItem( L"CacheCopyData" ) );
			param.PutCapabilityFlag( siReadOnly, false );
		} else {
			Parameter param( prop.GetParameters().GetItem( L"CacheCopyData" ) );
			param.PutCapabilityFlag( siReadOnly, true );
		}
		refresh = true;
	} else
	if( CString( L"CacheWriteMap" ) == paramName ) {
		if( changed.GetValue() ) {
			Parameter param( prop.GetParameters().GetItem( L"CacheCopyMap" ) );
			param.PutCapabilityFlag( siReadOnly, false );
		} else {
			Parameter param( prop.GetParameters().GetItem( L"CacheCopyMap" ) );
			param.PutCapabilityFlag( siReadOnly, true );
		}
		refresh = true;
	} else
	if( CString( L"CacheWriteImage" ) == paramName ) {
		if( changed.GetValue() ) {
			Parameter param( prop.GetParameters().GetItem( L"CacheCopyImage" ) );
			param.PutCapabilityFlag( siReadOnly, false );
		} else {
			Parameter param( prop.GetParameters().GetItem( L"CacheCopyImage" ) );
			param.PutCapabilityFlag( siReadOnly, true );
		}
		refresh = true;
	} else
	if( CString( L"RenderCache" ) == paramName ) {
		if( changed.GetValue() ) {
			Parameter param( prop.GetParameters().GetItem( L"RenderCacheDir" ) );
			param.PutCapabilityFlag( siReadOnly, false );
			param = prop.GetParameters().GetItem( L"RenderCacheSize" );
			param.PutCapabilityFlag( siReadOnly, false );
		} else {
			Parameter param( prop.GetParameters().GetItem( L"RenderCacheDir" ) );
			param.PutCapabilityFlag( siReadOnly, true );
			param = prop.GetParameters().GetItem( L"RenderCacheSize" );
			param.PutCapabilityFlag( siReadOnly, true );
		}
		refresh = true;
	}

	return refresh;
}

#define AFFOGATO_VERSION_CONTROL \
			layout.AddRow(); \
				layout.AddGroup( CValue(), true ); \
					item = layout.AddItem( L"___AffogatoVersion", L"Globals Created With Affogato" ); \
					item.PutLabelPercentage( 100 ); \
				layout.EndGroup(); \
			layout.EndRow();




XSIPLUGINCALLBACK CStatus AffogatoGlobals_PPGEvent( const CRef& inContext ) {

	Application app;
	PPGEventContext ctx( inContext );
	PPGEventContext::PPGEvent eventID( ctx.GetEventID() );

	switch( eventID ) {
		case PPGEventContext::siOnInit: {
			CustomProperty prop( ctx.GetSource() );
			PPGLayout layout( prop.GetPPGLayout() );

			layout.Clear();
			layout.PutAttribute( siUIHelpFile, L"http://affogato.sf.net/" );

			CValueArray tmpArray;

			PPGItem item;

			/*layout.AddGroup( CString(), false );
			item = layout.AddItem( L"___AffogatoVersion", CString(), siControlBitmap );
			item.PutAttribute( siUIFilePath, L"/home/moritzm/test.bmp" );
			item.PutAttribute( siUINoLabel, true );
			layout.EndGroup();*/

				layout.AddTab( L"Frame" );

					layout.AddRow();
						layout.AddGroup( CValue(), false, 49 );
							layout.AddGroup( L"Camera", true );

								item = layout.AddEnumControl( L"CameraName", tmpArray, L"Camera", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								updateAffogatoGlobalsState( prop, prop.GetParameters().GetItem( L"CameraName" ) );
								item = layout.AddItem( L"CameraFreezeScale" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"FrontPlane" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"BackPlane" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								tmpArray.Clear();
								tmpArray.Add( L"Off" );
								tmpArray.Add( 0l );
								tmpArray.Add( L"Zoom" );
								tmpArray.Add( 1l );
								tmpArray.Add( L"Crop" );
								tmpArray.Add( 2l );
								item = layout.AddEnumControl( L"CameraRotoViewStyle", tmpArray, L"Roto View Style", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"Depth of Field", true );
								item = layout.AddItem( L"DepthOfField", L"Use" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"FocusFactor" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"ExtremeMotionDepthOfField", L"Extreme Motion w. Depth of Field" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"Image", true );
								tmpArray.Clear();
								tmpArray.Add( L"Open EXR" );
								tmpArray.Add( L"exr" );
								tmpArray.Add( L"TIFF" );
								tmpArray.Add( L"tiff" );
								tmpArray.Add( L"Cineon" );
								tmpArray.Add( L"cineon" );
								tmpArray.Add( L"Radiance" );
								tmpArray.Add( L"radiance" );
								tmpArray.Add( L"EPS" );
								tmpArray.Add( L"eps" );
								tmpArray.Add( L"Z File" );
								tmpArray.Add( L"zfile" );
								item = layout.AddEnumControl( L"DisplayDriver", tmpArray, L"Type", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								updateAffogatoGlobalsState( prop, prop.GetParameters().GetItem( L"DisplayDriver" ) );
								tmpArray.Clear();
								tmpArray.Add( L"Float" );
								tmpArray.Add( 0l );
								tmpArray.Add( L"8 Bit" );
								tmpArray.Add( 1l );
								tmpArray.Add( L"16 Bit" );
								tmpArray.Add( 2l );
								tmpArray.Add( L"16 Bit, White Point @ 1k" );
								tmpArray.Add( 3l );
								tmpArray.Add( L"16 Bit, White Point @ 2k" );
								tmpArray.Add( 4l );
								tmpArray.Add( L"16 Bit, White Point @ 4k" );
								tmpArray.Add( 5l );
								item = layout.AddEnumControl( L"DisplayQuantization", tmpArray, L"Depth", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"AssociateAlpha", L"Premultiply Alpha" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								tmpArray.Clear();
								tmpArray.Add( L"None" );
								tmpArray.Add( 0l );
								tmpArray.Add( L"Framebuffer" );
								tmpArray.Add( 1l );
								tmpArray.Add( L"i-Display" );
								tmpArray.Add( 2l );
								tmpArray.Add( L"it" );
								tmpArray.Add( 3l );
								tmpArray.Add( L"Houdini" );
								tmpArray.Add( 4l );
								item = layout.AddEnumControl( L"PreviewDisplay", tmpArray, L"Preview Display", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"Resolution & Aspects", true );
								tmpArray.Clear();
								tmpArray.Add( L"Resolution" );
								tmpArray.Add( 0l );
								tmpArray.Add( L"Hype Data" );
								tmpArray.Add( 1l );
								item = layout.AddEnumControl( L"HypeOverscan", tmpArray, L"Use", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								updateAffogatoGlobalsState( prop, prop.GetParameters().GetItem( L"HypeOverscan" ) );
								item = layout.AddItem( L"ResolutionX", L"Width" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"ResolutionY", L"Height" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								tmpArray.Clear();
								tmpArray.Add( L"Quadruple" );
								tmpArray.Add( 4.0 );
								tmpArray.Add( L"Double" );
								tmpArray.Add( 2.0 );
								tmpArray.Add( L"Full" );
								tmpArray.Add( 1.0 );
								tmpArray.Add( L"Three Quarter" );
								tmpArray.Add( 0.75 );
								tmpArray.Add( L"Half" );
								tmpArray.Add( 0.5 );
								tmpArray.Add( L"Third" );
								tmpArray.Add( 0.333333333 );
								tmpArray.Add( L"Fourth" );
								tmpArray.Add( 0.25 );
								tmpArray.Add( L"Eighth" );
								tmpArray.Add( 0.125 );
								item = layout.AddEnumControl( L"ResolutionMultiplier", tmpArray, L"Res. Multiplier", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								tmpArray.Clear();
								tmpArray.Add( L"Resolution" );
								tmpArray.Add( 0l );
								tmpArray.Add( L"Camera" );
								tmpArray.Add( 1l );
								item = layout.AddEnumControl( L"UseCameraAspect", tmpArray, L"Image Aspect From", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"PixelAspect" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

						layout.EndGroup();

						layout.AddGroup( CValue(), false, 2 );
						layout.EndGroup();

						layout.AddGroup( CValue(), false, 49 );

							layout.AddGroup( L"Animation", true, 49 );
								tmpArray.Clear();
								tmpArray.Add( L"Current" );
								tmpArray.Add( 0l );
								/*tmpArray.Add( L"Start to End" );
								tmpArray.Add( 1l );
								tmpArray.Add( L"Start Only" );
								tmpArray.Add( 2l );*/
								tmpArray.Add( L"Sequence" );
								tmpArray.Add( 3l );
								item = layout.AddEnumControl( L"FrameOutput", tmpArray, L"Output Frame", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								updateAffogatoGlobalsState( prop, prop.GetParameters().GetItem( L"FrameOutput" ) );
								/*item = layout.AddItem( L"StartFrame" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"EndFrame" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"FrameStep" );
								item.PutLabelMinPixels( LABEL_WIDTH );*/
								item = layout.AddItem( L"Frames", L"Frame Sequence" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"Motion Blur", true );
								tmpArray.Clear();
								tmpArray.Add( L"Frame" );
								tmpArray.Add( 0l );
								tmpArray.Add( L"Sub-Frame" );
								tmpArray.Add( 1l );
								item = layout.AddEnumControl( L"SubFrameMotionBlur", tmpArray, L"Accuracy", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"GeometryMotionBlur", L"Geometry" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"GeometryParameterMotionBlur", L"Geometry Parameters" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"GeometryVariableMotionBlur", L"Geometry Variables" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"CameraMotionBlur", L"Camera" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"ShadowMapMotionBlur", L"Shadow Maps" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								// We don't need these in a REYES renderer
								/*item = layout.AddItem( L"LightMotionBlur", L"Lights" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"AttributeMotionBlur", L"Attributes" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"ShaderMotionBlur", L"Shaders" );
								item.PutLabelMinPixels( LABEL_WIDTH );*/
							layout.EndGroup();

							layout.AddGroup( L"Shutter", true, 49 );
								item = layout.AddItem( L"ShutterAngle", L"Angle" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								tmpArray.Clear();
								tmpArray.Add( L"Open on Frame" );
								tmpArray.Add( 0l );
								tmpArray.Add( L"Center on Frame" );
								tmpArray.Add( 1l );
								tmpArray.Add( L"Center Between Frames" );
								tmpArray.Add( 2l );
								tmpArray.Add( L"Close on Next Frame" );
								tmpArray.Add( 3l );
								item = layout.AddEnumControl( L"ShutterTiming", tmpArray, L"Timing", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								tmpArray.Clear();
								tmpArray.Add( L"Moving" );
								tmpArray.Add( 0l );
								tmpArray.Add( L"Stationary" );
								tmpArray.Add( 1l );
								item = layout.AddEnumControl( L"ShutterConfiguration", tmpArray, L"Configuration", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"ShutterOffset", L"Offset" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"ShutterEfficiency", L"Efficiency" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"MotionFactor" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"SampleMotion", L"Sample Motion Blur" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"Motion Segments", true );
								item = layout.AddItem( L"TransformationMotionSegments", L"Transformation" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"DeformationMotionSegments", L"Deformation" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

						layout.EndGroup();

					layout.EndRow();

					AFFOGATO_VERSION_CONTROL
/*
				layout.AddTab( L"Display" );

					item = layout.AddItem( L"Displays", NULL, siControlGrid );
					item.PutAttribute( siUIValueOnly, true );
					item.PutAttribute( siUIGridColumnWidths, L"100:140" );
					item.PutAttribute( siUIValueOnly, true );

					// Specify a specific height.  This forces the use of
					// a scroll bar
					item.PutAttribute( siUICY, 200l );

					// Make sure that the Column header doesn't get scrolled out
					// of view
					item.PutAttribute( siUIGridLockColumnHeader, true );

					AFFOGATO_VERSION_CONTROL
*/
				layout.AddTab( L"File" );

					layout.AddGroup( L"Name" );
						item = layout.AddItem( L"BaseName", L"Base Name" );
						item.PutLabelMinPixels( LABEL_WIDTH );
					layout.EndGroup();

					layout.AddGroup( L"Base Directory" );
						item = layout.AddItem( L"BaseDir", L"Base (Project Root)", siControlFolder );
						item.PutLabelMinPixels( LABEL_WIDTH );
						item = layout.AddItem( L"RelativePaths", L"Base-Relative Paths" );
						item.PutLabelMinPixels( LABEL_WIDTH );
					layout.EndGroup();

					layout.AddGroup( L"Output [Sub-]Directories" );
						item = layout.AddItem( L"ImageDir", L"Rendered Images", siControlFolder );
						item.PutLabelMinPixels( LABEL_WIDTH );
						item = layout.AddItem( L"MapDir", L"Maps & Textures", siControlFolder );
						item.PutLabelMinPixels( LABEL_WIDTH );
						item = layout.AddItem( L"DataDir", L"Data (RIBs)", siControlFolder );
						item.PutLabelMinPixels( LABEL_WIDTH );
						item = layout.AddItem( L"ObjectDataDir", L"Object Data", siControlFolder );
						item.PutLabelMinPixels( LABEL_WIDTH );
						item = layout.AddItem( L"AttributeDataDir", L"Attribute Data", siControlFolder );
						item.PutLabelMinPixels( LABEL_WIDTH );
						item = layout.AddItem( L"HubDir", L"Hubs", siControlFolder );
						item.PutLabelMinPixels( LABEL_WIDTH );
						item = layout.AddItem( L"TempDir", L"Temporary Files", siControlFolder );
						item.PutLabelMinPixels( LABEL_WIDTH );
					layout.EndGroup();

					layout.AddGroup( L"Global Options" );
						item = layout.AddItem( L"CreateDirs", L"Create Missing Directories" );
						item.PutLabelMinPixels( LABEL_WIDTH );
					layout.EndGroup();

					layout.AddGroup( L"Search Paths" );
						item = layout.AddItem( L"ShaderPath", L"Shaders", siControlFolder );
						item.PutLabelMinPixels( LABEL_WIDTH );
						item = layout.AddItem( L"TexturePath", L"Textures", siControlFolder );
						item.PutLabelMinPixels( LABEL_WIDTH );
						item = layout.AddItem( L"ArchivePath", L"Archives", siControlFolder );
						item.PutLabelMinPixels( LABEL_WIDTH );
						item = layout.AddItem( L"ProceduralPath", L"Procedurals", siControlFolder );
						item.PutLabelMinPixels( LABEL_WIDTH );
						item = layout.AddItem( L"ShaderPaths", L"Write Full Shader Paths" );
						item.PutLabelMinPixels( LABEL_WIDTH );
					layout.EndGroup();

					AFFOGATO_VERSION_CONTROL

				layout.AddTab( L"Image" );

					layout.AddRow();

						layout.AddGroup( CValue(), false, 49 );
							layout.AddGroup( L"Shading", true );
								item = layout.AddItem( L"ShadingRate" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"SmoothShading" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								tmpArray.Clear();
								tmpArray.Add( L"Shading Rate" );
								tmpArray.Add( 0l );
								tmpArray.Add( L"Once (Fast)" );
								tmpArray.Add( 1l );
								tmpArray.Add( L"Twice" );
								tmpArray.Add( 2l );
								tmpArray.Add( L"Thrice" );
								tmpArray.Add( 3l );
								tmpArray.Add( L"Quadruple" );
								tmpArray.Add( 4l );
								item = layout.AddEnumControl( L"CurveWidthShading", tmpArray, CValue(), L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"Shadow Pass Shading", true );
								item = layout.AddItem( L"ShadowShadingRate", L"Shading Rate" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"Pixel Sampling & Filtering", true );
								item = layout.AddItem( L"PixelSamplesX", L"Samples X" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"PixelSamplesY", L"Samples Y" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"SampleJitter" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								tmpArray.Clear();
								tmpArray.Add( L"Box" );
								tmpArray.Add( L"box" );
								tmpArray.Add( L"Triangle" );
								tmpArray.Add( L"triangle" );
								tmpArray.Add( L"Catmull-Rom" );
								tmpArray.Add( L"catmull-rom" );
								tmpArray.Add( L"Gaussian" );
								tmpArray.Add( L"gaussian" );
								tmpArray.Add( L"Sinc" );
								tmpArray.Add( L"sinc" );
								tmpArray.Add( L"Blackmann-Harris" );
								tmpArray.Add( L"blackmann-harris" );
								tmpArray.Add( L"Mitchell" );
								tmpArray.Add( L"mitchell" );
								tmpArray.Add( L"Bessel" );
								tmpArray.Add( L"bessel" );
								item = layout.AddEnumControl( L"PixelFilter", tmpArray, L"Filter", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"PixelFilterX", L"Filter Width" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"PixelFilterY", L"Filter Height" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

						layout.EndGroup();

						layout.AddGroup( CValue(), false, 2 );
						layout.EndGroup();

						layout.AddGroup( CValue(), false, 49 );

							layout.AddGroup( L"Calibration", true );
								item = layout.AddItem( L"Gain", L"Contrast" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"Gamma" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"R.E.Y.E.S.", true );
								tmpArray.Clear();
								tmpArray.Add( L"Horizontal" );
								tmpArray.Add( L"horizontal" );
								tmpArray.Add( L"Zig-Zag" );
								tmpArray.Add( L"zigzag" );
								tmpArray.Add( L"Vertical" );
								tmpArray.Add( L"vertical" );
								tmpArray.Add( L"Spiral" );
								tmpArray.Add( L"spiral" );
								tmpArray.Add( L"Circle" );
								tmpArray.Add( L"circle" );
								//Too baad for memory usage and users don"t RTFM :)
								tmpArray.Add( L"Random (Bad!)" );
								tmpArray.Add( L"random" );
								item = layout.AddEnumControl( L"BucketOrder", tmpArray, CValue(), L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"BucketX" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"BucketY" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"GridSize" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"TextureMemory", L"Texture Mem. (MB)" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"EyeSplits" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"GeometrySplits" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"OpacityThreshold" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

						layout.EndGroup();

					layout.EndRow();

					AFFOGATO_VERSION_CONTROL

				layout.AddTab( L"Rays" );
					layout.AddRow();

						layout.AddGroup( L"", false, 49 );

							layout.AddGroup( L"Global", true );
								item = layout.AddItem( L"RayTracing", L"Enable Ray-Tracing" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								updateAffogatoGlobalsState( prop, prop.GetParameters().GetItem( L"RayTracing" ) );
								item = layout.AddItem( L"RayDepth" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"Trace", true );
								item = layout.AddItem( L"TraceBias", L"Bias" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"TraceDisplacements", L"Displacements" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"TraceMotion", L"Motion" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();
						layout.EndGroup();

						layout.AddGroup( CValue(), false, 2 );
						layout.EndGroup();

						layout.AddGroup( L"", false, 49 );
							layout.AddGroup( L"Irradiance", true );
								item = layout.AddItem( L"IrradianceSamples", L"Samples" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"IrradianceShadingRate", L"Shading Rate" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"Subsurface", true );
								item = layout.AddItem( L"SubSurfaceShadingRate", L"Shading Rate" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();
						layout.EndGroup();

					layout.EndRow();

					AFFOGATO_VERSION_CONTROL

				layout.AddTab( L"Options" );

					layout.AddRow();
						layout.AddGroup( CValue(), false, 49 );

							layout.AddGroup( L"Data Layout & Format", true );

								item = layout.AddItem( L"TransformsInParentData", L"Object Transforms in Parent" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"RelativeTransforms" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"DelayData", L"Delayed Archives" );
								item.PutLabelMinPixels( LABEL_WIDTH );

								tmpArray.Clear();
								tmpArray.Add( L"Renderer" );
								tmpArray.Add( 0l );
								tmpArray.Add( L"XML" );
								tmpArray.Add( 1l );
								item = layout.AddEnumControl( L"AttributeDataType", tmpArray, L"Write Attributes to", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );

								item = layout.AddItem( L"WriteBinaryData", L"Binary" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"CompressData", L"Compressed" );
								item.PutLabelMinPixels( LABEL_WIDTH );
#ifdef RSP
								item = layout.AddItem( L"HubSupport" );
								item.PutLabelMinPixels( LABEL_WIDTH );
#endif

							layout.EndGroup();

							layout.AddGroup( L"Data Aquisition", true );

								item = layout.AddItem( L"HierarchicalDataScanning", L"Scene Tree Scanning" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								tmpArray.Clear();
								tmpArray.Add( L"Groups" );
								tmpArray.Add( 0l );
								tmpArray.Add( L"Objects" );
								tmpArray.Add( 1l );
								item = layout.AddEnumControl( L"AttributeDataScanningOrder", tmpArray, L"Final Attributes From", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );

							layout.EndGroup();

							layout.AddGroup( L"Feedback", true );
								tmpArray.Clear();
								tmpArray.Add( L"Sewn-Up Lips" );
								tmpArray.Add( 0l );
								tmpArray.Add( L"Errors Only" );
								tmpArray.Add( 1l );
								tmpArray.Add( L"Warnings & Errors" );
								tmpArray.Add( 2l );
								tmpArray.Add( L"Aunt Carla" );
								tmpArray.Add( 3l );
#ifdef DEBUG
								tmpArray.Add( L"Debug Biznani" );
								tmpArray.Add( 4l );
#endif
								item = layout.AddEnumControl( L"VerbosityLevel", tmpArray, L"Verbosity Level", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"Stopwatch" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"ShaderDebugging" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"NURB Surfaces & Curves", true );
								item = layout.AddItem( L"NormalizeNurbKnotVector", L"Normalize UV Space" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"NonRationalNurbSurface", L"Non-Rational Surfaces" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"NurbCurveWidth", L"Curve Width" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"NonRationalNurbCurve", L"Non-Rational Curves" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

						layout.EndGroup();

						layout.AddGroup( CValue(), false, 2 );
						layout.EndGroup();

						layout.AddGroup( CValue(), false, 49 );

							layout.AddGroup( L"Granularity & Output Sections", true );
								tmpArray.Clear();
								//tmpArray.Add( L"Frame" );
								//tmpArray.Add( 0l );
								// One RIB for options & world
								tmpArray.Add( L"Sub-Frame" );
								tmpArray.Add( 1l );
								// One RIB for options, world
								// split into lights, coord, objects
								tmpArray.Add( L"Sections" );
								tmpArray.Add( 2l );
								// One RIB for options, world
								// split into lights, spaces, geometry
								// each of the latter split into archives
								tmpArray.Add( L"Objects" );
								tmpArray.Add( 3l );
								tmpArray.Add( L"Attributes" );
								tmpArray.Add( 4l );
								item = layout.AddEnumControl( L"DataGranularity", tmpArray, L"Granularity", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"FrameData", L"Frame" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"OptionsData", L"Options" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"CameraData", L"Camera" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								//item = layout.AddItem( L"WorldData", L"World" );
								//item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"SpacesData", L"Spaces (Nulls)" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"LightsData", L"Lights" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"LooksData", L"Looks (Groups)" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"GeometryData", L"Objects" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"AttributeData", L"Attributes" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"ShaderParameterData", L"Shader Parameters" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								//item = layout.AddItem( L"ShaderNumericParameterData", L"Numeric Shader Parameters" );
								//item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"Lights", true );
								item = layout.AddItem( L"ShadowData", L"Create Shadow Map(s)" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"Default Shader", true );
								item = layout.AddItem( L"DefaultSurfaceShader", L"Surface" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"DefaultDisplacementShader", L"Displacement" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"DefaultVolumeShader", L"Volume" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"OverrideAllShaders" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"Shader Baking", true );
								item = layout.AddItem( L"BakeMode" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

						layout.EndGroup();

					layout.EndRow();

					AFFOGATO_VERSION_CONTROL

				layout.AddTab( L"Job" );
					layout.AddRow();

						layout.AddGroup( CValue(), false, 49 );

							layout.AddGroup( L"Output", true );

								tmpArray.Clear();
								tmpArray.Add( L"Data File (RIB)" );
								tmpArray.Add( 0l );
								tmpArray.Add( L"Renderer" );
								tmpArray.Add( 1l );
								item = layout.AddEnumControl( L"DirectToRenderer", tmpArray, L"Output to", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );

							layout.EndGroup();

							layout.AddGroup( L"Global", true );
								tmpArray.Clear();
								tmpArray.Add( L"Off" );
								tmpArray.Add( 0l );
								tmpArray.Add( L"Renderer" );
								tmpArray.Add( 1l );
								tmpArray.Add( L"Job Interpreter" );
								tmpArray.Add( 2l );
								item = layout.AddEnumControl( L"LaunchType", tmpArray, L"Launch", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								updateAffogatoGlobalsState( prop, prop.GetParameters().GetItem( L"LaunchType" ) );
								item = layout.AddItem( L"LaunchSubJobs", L"Do Sub-Jobs" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"JobScriptInterpreter", L"Job Interpreter" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"Job Script", true );
								tmpArray.Clear();
								tmpArray.Add( L"Off" );
								tmpArray.Add( 0l );
								tmpArray.Add( L"XML" );
								tmpArray.Add( 1l );
								tmpArray.Add( L"Job Engine" );
								tmpArray.Add( 2l );
								/*tmpArray.Add( L"Alfred" );
								tmpArray.Add( 3l );*/
								item = layout.AddEnumControl( L"JobScriptFormat", tmpArray, L"Type", L"Combo" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"JobScriptChunkSize", L"Chunk Size" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"Before", true );
								//item = layout.AddItem( L"PreJobCommand", L"Pre Job Cmd." );
								//item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"PreFrameCommand", L"Pre Frame Cmd." );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( L"After", true );
								item = layout.AddItem( L"PostFrameCommand", L"Post Frame Cmd." );
								item.PutLabelMinPixels( LABEL_WIDTH );
								//item = layout.AddItem( L"PostJobCommand", L"Post Job Cmd." );
								//item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();
						layout.EndGroup();

						layout.AddGroup( CValue(), false, 2 );
						layout.EndGroup();

						layout.AddGroup( CValue(), false, 49 );

							layout.AddGroup( L"Multithreading & Multiprocessing", true );
								item = layout.AddItem( L"Threads", L"Number of Threads" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"RemoteRenderHosts", L"Remote Host(s)" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"UseSSHForRemote", L"Use SSH for Remote Connections" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

						layout.EndGroup();

					layout.EndRow();

					AFFOGATO_VERSION_CONTROL

				layout.AddTab( L"Caching" );

					layout.AddGroup( L"Affogato Cache", true );

						item = layout.AddItem( L"CacheDir", L"Directory", siControlFolder );
						item.PutLabelMinPixels( LABEL_WIDTH );
						item = layout.AddItem( L"CacheSize", L"Size (GB)" );
						item.PutLabelMinPixels( LABEL_WIDTH );

						layout.AddRow();

							layout.AddGroup( L"Write To", true, 32 );
								item = layout.AddItem( L"CacheWriteData", L"Data" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								updateAffogatoGlobalsState( prop, prop.GetParameters().GetItem( L"CacheWriteData" ) );
								item = layout.AddItem( L"CacheWriteMap", L"Maps & Textures" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								updateAffogatoGlobalsState( prop, prop.GetParameters().GetItem( L"CacheWriteMap" ) );
								item = layout.AddItem( L"CacheWriteImage", L"Rendered Images" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								updateAffogatoGlobalsState( prop, prop.GetParameters().GetItem( L"CacheWriteImage" ) );
							layout.EndGroup();

							layout.AddGroup( CValue(), false, 2 );
							layout.EndGroup();

							layout.AddGroup( L"Source From", true, 32 );
								item = layout.AddItem( L"CacheSourceData", L"Data" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"CacheSourceMap", L"Maps & Textures" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

							layout.AddGroup( CValue(), false, 2 );
							layout.EndGroup();

							layout.AddGroup( L"Copy Back From", true, 32 );
								item = layout.AddItem( L"CacheCopyData", L"Data" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"CacheCopyMap", L"Maps & Textures" );
								item.PutLabelMinPixels( LABEL_WIDTH );
								item = layout.AddItem( L"CacheCopyImage", L"Rendered Images" );
								item.PutLabelMinPixels( LABEL_WIDTH );
							layout.EndGroup();

						layout.EndRow();
					layout.EndGroup();

					layout.AddGroup( L"Render Data Cache", true );
						tmpArray.Clear();
						tmpArray.Add( L"Off" );
						tmpArray.Add( 0l );
						tmpArray.Add( L"Reads" );
						tmpArray.Add( 1l );
						tmpArray.Add( L"Writes" );
						tmpArray.Add( 2l );
						tmpArray.Add( L"Reads & Writes" );
						tmpArray.Add( 4l );
						item = layout.AddEnumControl( L"RenderCache", tmpArray, L"Cache", L"Combo" );
						item.PutLabelMinPixels( LABEL_WIDTH );
						updateAffogatoGlobalsState( prop, prop.GetParameters().GetItem( L"RenderCache" ) );
						item = layout.AddItem( L"RenderCacheDir", L"Directory", siControlFolder );
						item.PutLabelMinPixels( LABEL_WIDTH );
						item = layout.AddItem( L"RenderCacheSize", L"Size (GB)" );
						item.PutLabelMinPixels( LABEL_WIDTH );

					layout.EndGroup();



			ctx.PutAttribute( L"Refresh", true );
			break;
		}
		case PPGEventContext::siButtonClicked: {
			break;
		}
		case PPGEventContext::siParameterChange: {
			Parameter changed( ctx.GetSource() );
			CustomProperty prop( changed.GetParent() );
			if( updateAffogatoGlobalsState( prop, changed ) )
				ctx.PutAttribute( L"Refresh", true );
			break;
		}
	}

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus AffogatoPass_Define( const CRef& inContext ) {

	// Here is where we add all the parameters to the
	// Custom Property.  This will be called each time
	// an new instance of the Custom Property is called.
	// It is not called when an persisted Custom Property
	// is loaded.
	Application app;
	CustomProperty prop = Context( inContext ).GetSource();
	Parameter param;

	// Default capabilities for most of these parameters
	int caps = siPersistable;
	CValue dft;	// Used for arguments we don't want to set

	// Frame Tab ------------------
	// File Group

	prop.AddParameter(	L"FileFormat", CValue::siString, caps,
						L"File Format", CValue(),
						L"exr", param );

	// Resolution Group
	prop.AddParameter(	L"OutputName", CValue::siString, caps,
						L"Output Name", CValue(),
						dft, param );

	// Float
	// Color
	// Point
	// Vector
	// Normal
	// Matrix
	prop.AddParameter(	L"OutputType", CValue::siUInt1, caps,
						L"Output Type", CValue(),
						2l, 0l, 8l, 0l, 8l, param );

	prop.AddParameter(	L"Quantization", CValue::siUInt1, caps,
						L"Quantization", CValue(),
						0l, 0l, 5l, 0l, 5l, param );


	// Filtering Group
	prop.AddParameter(	L"InheritPixelFilter", CValue::siBool, caps,
						L"Inherit Pixel Filter From Globals", CValue(),
						true, param );

	prop.AddParameter(	L"PixelFilter", CValue::siString, caps,
						L"Pixel Filter", CValue(),
						L"bessel", param );

	prop.AddParameter(	L"PixelFilterX", CValue::siFloat, caps,
						L"Pixel Filter Width", CValue(),
						6.476666, 1.0, 16.0, 1.0, 32.0, param );

	prop.AddParameter(	L"PixelFilterY", CValue::siFloat, caps,
						L"Pixel Filter Height", CValue(),
						6.476666, 1.0, 16.0, 1.0, 32.0, param );

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus AffogatoPass_DefineLayout( const CRef& inContext )
{
	// XSI will call this to define the visual appearance of the CustomProperty
	// The layout is shared between all instances of the CustomProperty
	// and is cached.  You can force the code to re-execute by using the
	// XSIUtils.Refresh feature.

	PPGLayout layout = Context( inContext ).GetSource();
	PPGItem item;

	layout.Clear();

	CValueArray tmpArray;

	item = layout.AddItem( L"Affogato Pass" );

		layout.AddRow();
			layout.AddGroup( L"Output", true, 49 );

				item = layout.AddItem( L"OutputName" );
				item.PutLabelMinPixels( LABEL_WIDTH_SMALL );

				tmpArray.Clear();
				tmpArray.Add( L"Float" );
				tmpArray.Add( 0l );
				tmpArray.Add( L"Color" );
				tmpArray.Add( 2l );
				tmpArray.Add( L"Point" );
				tmpArray.Add( 3l );
				tmpArray.Add( L"Vector" );
				tmpArray.Add( 5l );
				tmpArray.Add( L"Normal" );
				tmpArray.Add( 6l );
				tmpArray.Add( L"Matrix" );
				tmpArray.Add( 7l );
				item = layout.AddEnumControl( L"OutputType", tmpArray, CValue(), L"Combo" );
				item.PutLabelMinPixels( LABEL_WIDTH_SMALL );

				tmpArray.Clear();
				tmpArray.Add( L"Tiff" );
				tmpArray.Add( L"tiff" );
				tmpArray.Add( L"Open EXR" );
				tmpArray.Add( L"exr" );
				tmpArray.Add( L"Cineon" );
				tmpArray.Add( L"cineon" );
				tmpArray.Add( L"Radiance" );
				tmpArray.Add( L"radiance" );
				tmpArray.Add( L"EPS" );
				tmpArray.Add( L"eps" );
				tmpArray.Add( L"z-File" );
				tmpArray.Add( L"zfile" );
				tmpArray.Add( L"Shadowmap" );
				tmpArray.Add( L"shadowmap" );
				tmpArray.Add( L"Deep Shadowmap" );
				tmpArray.Add( L"dsm" );
				item = layout.AddEnumControl( L"FileFormat", tmpArray, CValue(), L"Combo" );
				item.PutLabelMinPixels( LABEL_WIDTH_SMALL );

				tmpArray.Clear();
				tmpArray.Add( L"Off (Float)" );
				tmpArray.Add( 0l );
				tmpArray.Add( L"8 Bit" );
				tmpArray.Add( 1l );
				tmpArray.Add( L"16 Bit" );
				tmpArray.Add( 2l );
				tmpArray.Add( L"16 Bit, 1k White Point" );
				tmpArray.Add( 3l );
				tmpArray.Add( L"16 Bit, 2k White Point" );
				tmpArray.Add( 4l );
				tmpArray.Add( L"16 Bit, 4k White Point" );
				tmpArray.Add( 5l );
				item = layout.AddEnumControl( L"Quantization", tmpArray, CValue(), L"Combo" );
				item.PutLabelMinPixels( LABEL_WIDTH_SMALL );

			layout.EndGroup();

			layout.AddGroup( CValue(), false, 2 );
			layout.EndGroup();
//Application.AddProp( "AffogatoPass")
			layout.AddGroup( L"Filtering", true, 49 );

				item = layout.AddItem( L"InheritPixelFilter", L"Inherit From Globals" );
				item.PutLabelMinPixels( LABEL_WIDTH_SMALL );

				tmpArray.Clear();
				tmpArray.Add( L"Box" );
				tmpArray.Add( L"box" );
				tmpArray.Add( L"Triangle" );
				tmpArray.Add( L"triangle" );
				tmpArray.Add( L"Catmull-Rom" );
				tmpArray.Add( L"catmull-rom" );
				tmpArray.Add( L"Gaussian" );
				tmpArray.Add( L"gaussian" );
				tmpArray.Add( L"Sinc" );
				tmpArray.Add( L"sinc" );
				tmpArray.Add( L"Blackmann-Harris" );
				tmpArray.Add( L"blackmann-harris" );
				tmpArray.Add( L"Mitchell" );
				tmpArray.Add( L"mitchell" );
				tmpArray.Add( L"Bessel" );
				tmpArray.Add( L"bessel" );
				item = layout.AddEnumControl( L"PixelFilter", tmpArray, CValue(), L"Combo" );
				item.PutLabelMinPixels( LABEL_WIDTH_SMALL );
				item = layout.AddItem( L"PixelFilterX" );
				item.PutLabelMinPixels( LABEL_WIDTH_SMALL );
				item = layout.AddItem( L"PixelFilterY" );
				item.PutLabelMinPixels( LABEL_WIDTH_SMALL );

			layout.EndGroup();

		layout.EndRow();

	return CStatus::OK;
}

