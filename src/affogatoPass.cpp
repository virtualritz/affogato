/** Handles passes (AOVs, not XSI passes)
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
#include <string>
#include <vector>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/replace.hpp>

// XSI headers
#include <xsi_application.h>
#include <xsi_argument.h>
#include <xsi_command.h>
#include <xsi_context.h>
#include <xsi_value.h>
#include <xsi_property.h>

// affogato headers
#include "affogato.hpp"
#include "affogatoGlobals.hpp"
#include "affogatoHelpers.hpp"
#include "affogatoPass.hpp"
#include "affogatoRenderer.hpp"


namespace affogato {

	using namespace XSI;
	using namespace std;

	pass::pass( const Property& affogatoPass ) {
		set( affogatoPass );
	}

	pass::pass( const boost::filesystem::path& theFileName,
				const string& theName,
				const string& theFormat,
				tokenValue::parameterType theType,
				const string& thePixelFilter,
				float theFilterWidth[ 2 ],
				bool theComputeAlpha,
				bool theAssociateAlpha,
				bool theExclusive,
				bool theMatte,
				quantizeType theQuantize,
				float theDither,
				bool theAutoCrop,
				lineOrderType theLineOrder
	) {
		set( theFileName, theName, theFormat, theType, thePixelFilter, theFilterWidth, theComputeAlpha, theAssociateAlpha, theExclusive, theMatte, theQuantize, theDither, theAutoCrop, theLineOrder );
	}

	pass::~pass() {
		/*for( vector< tokenValue* >::iterator it = tokenValuePtrArray.begin(); it < tokenValuePtrArray.end(); it++ ) {
			delete ( *it );
		}
		tokenValuePtrArray.clear();*/
	}

	void pass::set( const Property&affogatoPass ) {
		format		= CStringToString( getParameter( affogatoPass, L"FileFormat", CValue::siString ) );
		name		= CStringToString( getParameter( affogatoPass, L"OutputName", CValue::siString ) );
		globals& g( globals::access() );
		fileName = g.name.baseName + "." + name;

		type		= static_cast< tokenValue::parameterType >( ( unsigned long )getParameter( affogatoPass, L"OutputType", CValue::siUInt1 ) );

		bool inheritPixelFilter	= ( bool )affogatoPass.GetParameterValue( L"InheritPixelFilter" );

		if( inheritPixelFilter ) {
			pixelFilter			= g.filtering.filter;
			filterWidth[ 0 ]	= g.filtering.x;
			filterWidth[ 1 ]	= g.filtering.y;
		} else {
			pixelFilter			= CStringToString( affogatoPass.GetParameterValue( L"PixelFilter" ) );
			filterWidth[ 0 ]	= ( float )affogatoPass.GetParameterValue( L"PixelFilterX" );
			filterWidth[ 1 ]	= ( float )affogatoPass.GetParameterValue( L"PixelFilterY" );
		}

		quantize	= static_cast< quantizeType >( ( unsigned long )affogatoPass.GetParameterValue( L"Quantization" ) );
		dither		= ( float )affogatoPass.GetParameterValue( L"Dither" );
		if( ( "exr" == format ) ||
		    ( "cineon" == format ) ||
			( "radiance" == format ) ||
			( "zfile" == format ) ||
			( "shadowmap" == format ) )
			quantize = quantizeFloat;
	}

	void pass::set( const boost::filesystem::path& theFileName,
					const string& theName,
					const string& theFormat,
					tokenValue::parameterType theType,
					const string& thePixelFilter,
					float theFilterWidth[2],
					bool theComputeAlpha,
					bool theAssociateAlpha,
					bool theExclusive,
					bool theMatte,
					quantizeType theQuantize,
					float theDither,
					bool theAutoCrop,
					lineOrderType theLineOrder
	) {
		fileName	= theFileName;
		name		= theName;
		format		= theFormat;
		type		= theType;
		pixelFilter	= thePixelFilter;
		filterWidth[ 0 ] = theFilterWidth[ 0 ];
		filterWidth[ 1 ] = theFilterWidth[ 1 ];
		computeAlpha = theComputeAlpha;
		associateAlpha = theAssociateAlpha;
		exclusive	= theExclusive;
		matte		= theMatte;
		quantize	= theQuantize;
		dither		= theDither;
		autoCrop	= theAutoCrop;
		lineOrder	= theLineOrder;
	}

	void pass::write() {
		using namespace ueberMan;
		ueberManInterface theRenderer;
		const globals& g( globals::access() );

		int quantizeValues[ 4 ] = { 0, 65535, 0, 65535 };

		switch( quantize ) {
			case quantizeFloat:
				quantizeValues[ 1 ] = 0;
				quantizeValues[ 3 ] = 0;
				if( -1 == dither )
					dither = 0;
				break;
			case quantize8:
				quantizeValues[ 1 ] = 255;
				quantizeValues[ 3 ] = 255;
				if( -1 == dither )
					dither = 0.5;
				break;
			case quantize16wp1k: // 16 bit, white point at 1k
				quantizeValues[ 1 ] = 1023;
				if( -1 == dither )
					dither = 0.5;
				break;
			case quantize16wp2k: // 16 bit, white point at 2k
				quantizeValues[ 1 ] = 2047;
				if( -1 == dither )
					dither = 0.5;
				break;
			case quantize16wp4k: // 16 bit, white point at 4k
				quantizeValues[ 1 ] = 4095;
				if( -1 == dither )
					dither = 0.5;
				break;
		}
		theRenderer.parameter( tokenValue( quantizeValues, 4, "quantize[4]" ) );
		theRenderer.parameter( "dither", dither );
		theRenderer.parameter( "filter", string( pixelFilter ) );
		theRenderer.parameter( tokenValue( filterWidth, 2, "filterwidth[2]", tokenValue::storageUniform, tokenValue::typeFloat ) );
		if( computeAlpha ) {
			theRenderer.parameter( "computealpha", bool( true ) );
		}
		if( !associateAlpha ) {
			theRenderer.parameter( "associatealpha", bool( false ) );
		}
		theRenderer.parameter( "exclusive", exclusive );
		theRenderer.parameter( "matte", matte );

		string theName( getCacheFilePath( fileName, g.directories.caching.imageWrite ).native_file_string() );
		if( string::npos == theName.find( "#", 0 ) )
			theName += "." + g.name.currentFrame + "." + format.substr( 0, 3 );
		else
			theName = parseString( theName );

		string typeStr( getParameterTypeAsString( type ) );

		if( "exr" == format ) {
			theRenderer.parameter( "compression", string( "piz" ) ); // Use PIZ compression for OpenEXR
			if( decreasing == lineOrder )
				theRenderer.parameter( "exrlineorder", string( "decreasing" ) ); // Write image bottom to top
			if( autoCrop )
				theRenderer.parameter( "autocrop", bool( true ) ); // Crop to data Window
			theRenderer.parameter( "comment", string( "Rendered from a RIB generated by Affogato " ) + AFFOGATOVERSION ); // Tag da render!
		}

		theRenderer.output( theName, format, typeStr + ( typeStr.length() ? " " : "" ) + name, "" );
	}

	/**
	 * Creates XML from an attribute description.
	 *
	 * The result looks like this:
	 *
	 */
	string pass::getXML() {

		stringstream passXML;
		passXML << "<pass\n";
		passXML << " name=\"" << name << "\"" << endl;
		passXML << " type=\"" << getParameterTypeAsString( type ) << "\"" << endl;
		passXML << " format=\"" << format << "\"" << endl;
		passXML << " quantize=\"";
		switch( quantize ) {
			quantizeFloat:
				passXML << "float";
				break;
			quantize8:
				passXML << "8";
				break;
			quantize16:
				passXML << "16";
				break;
			quantize16lin1k: // 16 bit, white point at 1k
				passXML << "16@1k";
				break;
			quantize16lin2k: // 16 bit, white point at 2k
				passXML << "16@2k";
				break;
			quantize16lin4k: // 16 bit, white point at 4k
				passXML << "16@4k";
				break;
		}
		passXML << "\"" << endl;

		passXML << " filter=\"" << format << "\"" << endl;
		passXML << " filtersize=\"" << format << "\"" << endl;

		return passXML.str();
	}


	boost::filesystem::path pass::getFileName() {
		globals& g( globals::access() );
		string theName( fileName.string() );
		if( string::npos == theName.find( "#", 0 ) )
			theName += ".####." + format.substr( 0, 3 );

		return theName;
	}


	CValue pass::getParameter( const Property &prop, const CString name, CValue::DataType type ) {
		Parameter param( prop.GetParameter( name ) );
		if( param.GetValue().m_t == type )
			return param.GetValue();
		else // return an empty CValue
			return CValue();
	}

}


using namespace XSI;
using namespace affogato;
using namespace std;


#ifdef unix
extern "C"
#endif
CStatus AffogatoAddPass_Init( const XSI::CRef &in_context ) {
	Context ctxt( in_context );
	Command cmd( ctxt.GetSource() );

	cmd.EnableReturnValue( true );

	Application app;
	message( L"Adding " + cmd.GetName() + L" Command.", messageInfo );

	ArgumentArray args = cmd.GetArguments();
	args.Add( L"FileName", L"" );
	args.Add( L"Name", L"" );
	args.Add( L"Type", L"" );
	args.Add( L"Filter", L"" );
	args.Add( L"FilterSizeX", 6.49 );
	args.Add( L"FilterSizeY", 6.49 );
	args.Add( L"ComputeAlpha", false );
	args.Add( L"AssociateAlpha", true );
	args.Add( L"Exclusive", false );
	args.Add( L"Matte", true );
	args.Add( L"Depth", L"" );
	args.Add( L"Dither", true );

	return CStatus::OK;
}


#ifdef unix
extern "C"
#endif
CStatus AffogatoAddPass_Execute( const CRef &in_Ctx ) {

	Application app;

	Context ctx( in_Ctx );
	CValueArray args = ctx.GetAttribute( L"Arguments" );
	return CStatus::OK;
}

