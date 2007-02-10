/** The Affogato plugin takes care of registering the plugin with
 *  XSI.
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

#include <map>

#include <xsi_argument.h>
#include <xsi_application.h>
#include <xsi_command.h>
#include <xsi_context.h>
#include <xsi_menu.h>
#include <xsi_model.h>
#include <xsi_pluginregistrar.h>
#include <xsi_selection.h>
#include <xsi_uitoolkit.h>

#include "affogato.hpp"
#include "affogatoExecute.hpp"
#include "affogatoHelpers.hpp"
#include "affogatoWorker.hpp"


using namespace XSI;
using namespace affogato;


/*!
	Entry-point called by XSI when this file gets loaded. All plugin items
	exposed here must be registered in this function.

	\param inRegistrar: PluginRegistrar object required for registering the plugin
	items and some information about this plugin.

	*note* This function is mandatory and must be implemented. The plugin will
	not be loaded if this function is missing.
*/



XSIPLUGINCALLBACK CStatus XSILoadPlugin( PluginRegistrar &inRegistrar ) {

	/*static bool notDone( true );
	if( notDone ) { // Have to do this or else boost::filesystem throws exceptions like crazy
#ifdef _WIN32
		boost::filesystem::path::default_name_check( boost::filesystem::native );
#else
		boost::filesystem::path::default_name_check( boost::filesystem::windows_name );
#endif
		notDone = false;
	}*/

#ifndef _WIN32
#ifdef DEBUG
	FILE *f;
	f = fopen( "/tmp/affogato.log", "w" );
	fprintf( f, "Initialized Debug Output\n" );
	fflush( f );
	fclose( f );
#endif
#endif
	debugMessage( L"Loading Affogato" );

	inRegistrar.PutAuthor( L"Moritz Moeller | Rising Sun Pictures" );
	inRegistrar.PutName( L"RSP Affogato Render Bridge" );
	string version( AFFOGATOVERSION );

	long major;
	long minor;
	size_t pos = version.find( "." );
	if( string::npos != pos ) {
		major = atol( version.substr( 0, pos ).c_str() );
		size_t pos1 = version.rfind( "." );
		if( ( pos == pos1 ) || ( string::npos == pos1 ) )
			minor = atol( version.substr( pos + 1 ).c_str() );
		else
			minor = atol( version.substr( pos + 1, pos1 - pos ).c_str() );

	} else {
		major = atol( version.c_str() );
		minor = 0;
	}

	inRegistrar.PutVersion( major, minor );
	inRegistrar.PutURL( L"www.rsp.com.au" );
	inRegistrar.PutEmail( L"moritzm@rsp.com.au" );

	Application app;
	app.LogMessage( L"Loading RSP Affogato Render Bridge " + stringToCString( AFFOGATOVERSION ) );

	debugMessage( L"Registering commands" );
	// register a command plugin item.
	inRegistrar.RegisterCommand( L"Affogato Render", L"AffogatoRender" );
	inRegistrar.RegisterCommand( L"Affogato Render Selected", L"AffogatoRenderSelected" );
	//inRegistrar.RegisterCommand( L"Affogato Write Passes XML", L"AffogatoWritePassesXML" );
	inRegistrar.RegisterCommand( L"Affogato Export Archive", L"AffogatoExportArchive" );
	inRegistrar.RegisterCommand( L"Affogato Create Shader", L"AffogatoCreateShader" );
	inRegistrar.RegisterCommand( L"Affogato Open Globals", L"AffogatoOpenGlobals" );
	inRegistrar.RegisterCommand( L"Affogato Update All Globals", L"AffogatoUpdateAllGlobals" );
	inRegistrar.RegisterCommand( L"Affogato About", L"AffogatoAbout" );

#ifdef RSP
	inRegistrar.RegisterCommand( L"Affogato Create HUB Property", L"AffogatoCreateHUBProperty" );
#endif

	// register a help menu for the simple command
	inRegistrar.RegisterMenu( siMenuMainHelpID, L"&Affogato Help", false, false );

	debugMessage( L"Registering properties" );
	inRegistrar.RegisterProperty( L"AffogatoGlobals" );
	inRegistrar.RegisterProperty( L"AffogatoHUB" );
	inRegistrar.RegisterProperty( L"AffogatoEnvironmentMapGenerator" );


	string affogatoHomePath( getEnvironment( "AFFOGATOHOME" ) );
	if( !affogatoHomePath.empty() ) {
		/*CValue retVal;
		CValueArray args( 1 );
		args[ 0 ] = stringToCString( ( boost::filesystem::path( affogatoHomePath ) / boost::filesystem::path( "python/affogatoInit.py" ) ).native_file_string() );
		app.LogMessage( L"Affogato: Running '" + CString( args[ 0 ] ) + L"'" );
		app.LoadPlugin( L"LogMessage", args, retVal );
		if( CStatus::OK != CStatus( retVal ) ) {
			app.LogMessage( L"Affogato: Could not run '" + CString( args[ 0 ] ) + L"'. Exiting", siErrorMsg );
			return CStatus( retVal );
		}*/
	} else {
		app.LogMessage( L"Affogato: AFFOGATOHOME environment variable is not defined. Exiting", siErrorMsg );
		return CStatus::Fail;
	}

#ifdef RSP
	string rspMenuHome( getEnvironment( "RSPXSIMENUHOME" ) );
	string rspMenuPath( getEnvironment( "RSPXSIMENU" ) );
	pos = rspMenuPath.find( affogatoHomePath );
	if( rspMenuHome.empty() || rspMenuPath.empty() || ( string::npos == pos ) )
#endif
		inRegistrar.RegisterMenu( siMenuMainTopLevelID, L"&Affogato", false, false );

	return CStatus::OK;
}


/*!
	Entry-point called by XSI when this file gets unloaded. All plugin items
	registered by this plugin are automatically unregistered by XSI. Use this
	function to perform internal cleanup.

	\param inRegistrar PluginRegistrar object.

	*note* This function is optional.
*/
XSIPLUGINCALLBACK CStatus XSIUnloadPlugin( const XSI::PluginRegistrar &inRegistrar ) {

	Application app;
	app.LogMessage( inRegistrar.GetName() + L" has been unloaded");

	debugMessage( L"Unloaded Affogato" + stringToCString( AFFOGATOVERSION ) );

	return XSI::CStatus::OK;
}


XSIPLUGINCALLBACK CStatus AffogatoUpdateAllGlobals_Init( const CRef &inContext) {

	Context ctxt( inContext);
	Command cmd( ctxt.GetSource() );

	Application app;
	app.LogMessage( L"Initalizing '" + cmd.GetName() + L"' command" );

	ArgumentArray args( cmd.GetArguments() );
	args.Add( L"Force Update", CValue() );

	return CStatus::OK;
}


XSIPLUGINCALLBACK CStatus AffogatoUpdateAllGlobals_Execute( const CRef &inContext) {

	Application app;
	Context ctxt( inContext);
	CValueArray args = ctxt.GetAttribute( L"Arguments" );

	bool force( ( bool )args[ 0 ] );

	SceneItem sceneRoot( app.GetActiveSceneRoot() );

	bool noGlobals = true;
	CRefArray props( sceneRoot.GetProperties() );

	CStatus status( props.Filter( L"AffogatoGlobals", CStringArray(), CString(), props ) );
	if( CStatus::OK == status ) {

		map< string, map< string, CValue > > globalParamsMap;

		for( unsigned i( 0 ); i < ( unsigned )props.GetCount(); i++ ) {
			Property prop( props[ i ] );
			CParameterRefArray params( prop.GetParameters() );
			CString propName( prop.GetName() );
			string name( propName.GetAsciiString() );
			if( force || ( AFFOGATOVERSION != CString( prop.GetParameterValue( L"___AffogatoVersion" ) ).GetAsciiString() ) ) {
				app.LogMessage( L"Affogato: Updating globals '" + propName + L"'", siInfoMsg );
				for( unsigned j = 0; j < ( unsigned )params.GetCount(); j++ ) {
					Parameter param( params[ j ] );
					string paramName( param.GetScriptName().GetAsciiString() );
					if( "___AffogatoVersion" != paramName )
						globalParamsMap[ name ][ paramName ] = param.GetValue();
				}
				CValue retval;
				CValueArray args( 1 );
				args[ 0 ] = stringToCString( name );
				app.ExecuteCommand( L"DeleteObj", args, retval );
			}
		}

		for( map< string, map< string, CValue > >::iterator it = globalParamsMap.begin(); it != globalParamsMap.end(); it++ ) {
			Property prop( sceneRoot.AddProperty( L"AffogatoGlobals", false, stringToCString( it->first ) ) );
			CParameterRefArray params( prop.GetParameters() );
			for( unsigned j = 0; j < ( unsigned )params.GetCount(); j++ ) {
				Parameter param( params[ j ] );
				string name( param.GetScriptName().GetAsciiString() );
				for( map< string, CValue >::const_iterator jt = it->second.begin(); jt != it->second.end(); jt++ ) {
					if( name == jt->first ) {
						CValue tmp( jt->second ); // Make sure we match data type, or else this will change the parameter type (e.g. bool->int)
						tmp.ChangeType( param.GetValueType () );
						param.PutValue( tmp );
					}
				}
			}

#ifdef RSP // For backwards-compatibility with old pipeline tools
			if( it->second.end() == it->second.find( "Frames" ) ) {
				switch( ( long )prop.GetParameterValue( L"FrameOutput" ) ) {
					case 1:   // Start to End
					case 3: { // Sequence
						char s[ 32 ];
						sprintf( s, "%d-%d@%d", long( it->second[ "StartFrame" ] ), long( it->second[ "EndFrame" ] ), long( it->second[ "FrameStep" ] ) );
						// prop.PutParameterValue( L"Frames", stringToCString( ( boost::format( "%d-%d@%d" ) % long( it->second[ "StartFrame" ] ) % long( it->second[ "EndFrame" ] ) % long( it->second[ "FrameStep" ] ) ).str() ) );
						prop.PutParameterValue( L"Frames", stringToCString( s ) );
						prop.PutParameterValue( L"FrameOutput", 3l );
					}
					case 0: {
						prop.PutParameterValue( L"FrameOutput", 0l );
						break;
					}
					case 2: {
						char s[ 32 ];
						sprintf( s, "%d@%d", long( it->second[ "StartFrame" ] ), long( it->second[ "FrameStep" ] ) );
						// prop.PutParameterValue( L"Frames", stringToCString( ( boost::format( "%d@%d" ) % long( it->second[ "StartFrame" ] ) % long( it->second[ "FrameStep" ] ) ).str() ) );
						prop.PutParameterValue( L"Frames", stringToCString( s ) );
						prop.PutParameterValue( L"FrameOutput", 3l );
					}
				}
			}
#endif
		}

		if( !globalParamsMap.empty() ) {
			UIToolkit kit( app.GetUIToolkit() );
			long result;
			kit.MsgBox( L"Some Affogato Globals were updated\nyou should save your scene", siMsgOkOnly | siMsgExclamation, L"Affogato Update All Globals", result );
			app.LogMessage( L"Affogato: Globals were updated -- you should save your scene", siWarningMsg );
		}
	}

	return status;
}


XSIPLUGINCALLBACK CStatus AffogatoRender_Init( const CRef &inContext) {

	debugMessage( L"Initializing Affogato " + stringToCString( AFFOGATOVERSION ) );

	Context ctxt( inContext);
	Command cmd( ctxt.GetSource() );

	Application app;
	app.LogMessage( L"Initalizing '" + cmd.GetName() + L"' command" );

	ArgumentArray args( cmd.GetArguments() );
	args.Add( L"AffogatoGlobals Property", CValue() );
	args.Add( L"Object Collection", CRefArray() );

	// allocate memory for storing the user data
	//CValue hardWorker = (CValue::siPtrType) new affogato::worker();
	//ctxt.PutUserData( hardWorker );

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus AffogatoRender_Execute( CRef &inContext) {
	Application app;

	Context ctxt( inContext);
	CValueArray args = ctxt.GetAttribute( L"Arguments" );

	CValueArray vals = args[ 1 ];
	CRefArray objects;
	objects.Set( vals );

	// Expand any models passed
	for( unsigned i( 0 ); i < ( unsigned )vals.GetCount(); i++ ) {
		CRef obj( vals[ i ] );
		if( siModelID == obj.GetClassID() )
				objects += Model( obj ).FindChildren ( L"*", CString(), CStringArray() );
	}

	if( !objects.GetCount() ) {
		Model sceneRoot = app.GetActiveSceneRoot();
		objects  = sceneRoot.FindChildren( CString(), siPolyMeshType,    CStringArray() );
		objects += sceneRoot.FindChildren( CString(), siSrfMeshPrimType, CStringArray() );
		objects += sceneRoot.FindChildren( CString(), siCrvListPrimType, CStringArray() );
		objects += sceneRoot.FindChildren( CString(), siHairKeyword,     CStringArray() );
		objects += sceneRoot.FindChildren( CString(), siCloudPrimType,   CStringArray() );
		objects += sceneRoot.FindChildren( CString(), siNullPrimType,    CStringArray() );
		objects += sceneRoot.FindChildren( CString(), siLightPrimType,   CStringArray() );
		objects += sceneRoot.FindChildren( CString(), siSpherePrimType,  CStringArray() );
	}

	if( !app.IsInteractive() )
		AffogatoUpdateAllGlobals_Execute( CRef() );

	affogato::worker hardWorker;
	string property( CString( args[ 0 ] ).GetAsciiString() );
	if( property.empty() )
		debugMessage( L"Affogato: Kicking off empty worker" );
	else
		debugMessage( L"Affogato: Kicking off worker with '" + stringToCString( property ) + L"'" );

	hardWorker.work( property, objects );


	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus AffogatoAbout_Execute( CRef &inContext) {
	Application app;
	UIToolkit kit( app.GetUIToolkit() );
	long result;
	kit.MsgBox(
		L"RSP Affogato Render Bridge\nVersion " + stringToCString( AFFOGATOVERSION ) + L"\n" +
		L"\nInitial code by Moritz Moeller" +
		L"\nContributors: Alan Jones, Dan Wills, Sam Hodge" +
		L"\n\nCopyright (c) 2005, 2006 Rising Sun Pictures",
		siMsgOkOnly, L"About Affogato", result );

	return CStatus::OK;
}


XSIPLUGINCALLBACK CStatus AffogatoRenderSelected_Init( const CRef &inContext) {
	Context ctxt( inContext);
	Command cmd( ctxt.GetSource() );

	Application app;
	app.LogMessage( L"Initalizing '" + cmd.GetName() + L"' command" );

	ArgumentArray args = cmd.GetArguments();
	args.Add( L"AffogatoGlobalsProperty", CValue() );

	// allocate memory for storing the user data
	//CValue hardWorker = (CValue::siPtrType) new affogato::worker();
	//ctxt.PutUserData( hardWorker );

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus AffogatoRenderSelected_Execute( const CRef &inContext) {
	Application app;
	Context ctxt( inContext);
	CValueArray args = ctxt.GetAttribute( L"Arguments" );

	affogato::worker hardWorker;

	CRefArray objectList( app.GetSelection().GetArray() );

	if( args[ 0 ] != CValue() ) {
		string property;
		if( CValue::siString == args[ 0 ].m_t )
			property = CString( args[ 0 ] ).GetAsciiString();
		else
			property = CRef( args[ 0 ] ).GetAsText().GetAsciiString();
		message( L"Affogato: Kicking off worker with '" + stringToCString( property ) + L"' for selection", messageInfo );
		hardWorker.work( property, objectList );
	} else {
		message( L"Affogato: Kicking off empty worker for selection", messageInfo );
		hardWorker.work( string(), objectList );
	}

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus AffogatoExportArchive_Init( const CRef &inContext) {
	Context ctxt( inContext);
	Command cmd( ctxt.GetSource() );

	Application app;
	app.LogMessage( L"Initalizing '" + cmd.GetName() + L"' command" );

	ArgumentArray args = cmd.GetArguments();
	args.Add( L"Archive File Name", CValue() );
	args.Add( L"Object Collection", CRefArray() );
	args.Add( L"AffogatoGlobals Property", CValue() );

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus AffogatoExportArchive_Execute( const CRef &inContext) {
	Application app;

	Context ctxt( inContext);
	CValueArray args = ctxt.GetAttribute( L"Arguments" );

	affogato::worker hardWorker;
	if( ( CValue() != args[ 0 ] ) && ( CValue::siArray == args[ 1 ].m_t ) ) {
		app.LogMessage( L"Affogato: Exporting Archive" );
		CValueArray vals = args[ 1 ];

		CRefArray objects;
		objects.Set( vals );

		// Expand any models passed
		for( unsigned i( 0 ); i < ( unsigned )vals.GetCount(); i++ ) {
			CRef obj( vals[ i ] );
			if( siModelID == obj.GetClassID() )
					objects += Model( obj ).FindChildren ( L"*", CString(), CStringArray() );
		}

		//app.LogMessage( L"Passing: " + CRef( args[ 2 ] ).GetAsText() );

		string property;
		if( CValue::siString == args[ 2 ].m_t )
			property = CString( args[ 2 ] ).GetAsciiString();
		else
			property = CRef( args[ 2 ] ).GetAsText().GetAsciiString();

		hardWorker.work( property, objects, CString( args[ 0 ] ).GetAsciiString() );
	} else {
		app.LogMessage( L"Affogato: Not enough arguments", siErrorMsg );
		return CStatus::Fail;
	}

	return CStatus::OK;
}



XSIPLUGINCALLBACK CStatus AffogatoHelp_Init( const CRef &inContext ) {
	Context ctxt = inContext;
	Menu menu = ctxt.GetSource();

	CStatus st;
	MenuItem item;
	menu.AddCallbackItem( L"Affogato Help", L"OnAffogatoHelpMenu", item );

	return CStatus::OK;
}

// help menu item callback
XSIPLUGINCALLBACK CStatus OnAffogatoHelpMenu( const CRef &inContext ) {
	Application app;

#ifdef _WIN32
	execute( "http://affogato.sf.net/", "", ".", false );
#else
#ifdef RSP
	execute( "need firefox && firefox", "http://admin.rsp.com.au/rspwiki/index.php/Affogato", ".", false );
#else
	execute( "firefox", "http://affogato.sf.net/", ".", false );
#endif
#endif

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus Affogato_Init( CRef &inContext ) {
	Context ctxt = inContext;
	Menu menu = ctxt.GetSource();

	MenuItem item;

	menu.AddCommandItem( L"&Render", L"Affogato Render", item );
	menu.AddCommandItem( L"Render &Selected", L"Affogato Render Selected", item );
	menu.AddCommandItem( L"&Globals...", L"Affogato Open Globals", item );
#ifdef RSP
	Menu subMenu;
	menu.AddItem( L"&Properties", siMenuItemSubmenu, subMenu );
	subMenu.AddCommandItem( L"Add &HUB...", L"Affogato Create HUB Property", item );
#endif
	menu.AddSeparatorItem();
	menu.AddCommandItem( L"&Update All Globals", L"Affogato Update All Globals", item );
	menu.AddSeparatorItem();
	menu.AddCommandItem( L"&About...", L"Affogato About", item );


	return CStatus::OK;
}


/*XSIPLUGINCALLBACK CStatus AffogatoOpenGlobals_Init( const CRef &inContext) {
	return CStatus::OK;
}*/

XSIPLUGINCALLBACK CStatus AffogatoAttributes_Define( const CRef &inContext ) {

	// Here is where we add all the parameters to the
	// Custom Property.  This will be called each time
	// an new instance of the Custom Property is called.
	// It is not called when an persisted Custom Property is loaded.
	Application app;
	CustomProperty prop( Context( inContext ).GetSource() );
	Parameter param;

	// Default capabilities for most of these parameters
	int caps( siPersistable );
	CValue dft; // Used for arguments we don't want to set

	return CStatus::OK;
}




