/** Attribute abstraction class.
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
#include <sstream>
#include <vector>

// XSI headers
#include <xsi_application.h>
#include <xsi_parameter.h>
#include <xsi_sceneitem.h>

// Affogato headers
#include "affogato.hpp"
#include "affogatoAttribute.hpp"
#include "affogatoGlobals.hpp"
#include "affogatoHelpers.hpp"
#include "affogatoRenderer.hpp"
#include "affogatoTokenValue.hpp"


namespace affogato {

	using namespace XSI;
	using namespace std;


	attribute::attribute( const Parameter &param, bool userType ) {
		const globals& g( globals::access() );

		Application app;

		string	xsiName( CStringToString( param.GetScriptName() ) ),
				name;
		tokenValue::parameterType type;

		theCategory = categoryUnknown;

		if( "primray" == xsiName ) {
			theCategory = categoryVisibility;
			name = "visibility:camera";
		} else
		if( "scndray" == xsiName ) {
			theCategory = categoryVisibility;
			name = "visibility:trace";
		} else
		if( "shdw" == xsiName ) {
			theCategory = categoryUser;
			name = "user:castsshadows";
		} else
		if( "shadingrate" == xsiName ) {
			name = "shading:rate";
			theCategory = categoryShading;
		} else
		if( "matte" == xsiName ) {
			theCategory = categoryShading;
			name = "shading:matte";
		} else
		if( "sides" == xsiName ) {
			theCategory = categoryShading;
			name = "shading:sides";
		} else
		if( "color" == xsiName ) {
			theCategory = categoryShading;
			name = "shading:color";
			Application app;
			debugMessage( L"Found color attribute" );
		} else
		if( "opacity" == xsiName ) {
			theCategory = categoryShading;
			name = "shading:opacity";
		} else
		if( userType ) {
			theCategory = categoryUser;
			name = xsiName;
		}

		if( theCategory != categoryUnknown )
			debugMessage( L"Found attribute " + param.GetScriptName() + L" " + CValue( (long)param.GetValueType() ).GetAsText() );

		if( categoryUnknown != theCategory ) {
			if( name.length() ) {
				switch( param.GetValue().m_t ) {
					case CValue::siEmpty: {
						break;
					}
					case CValue::siBool:
					case CValue::siInt1:
					case CValue::siInt2:
					case CValue::siInt4: {
						long value( param.GetValue( g.animation.time ) );
						if( value > numeric_limits< int >::max() )
							value = numeric_limits< int >::max();
						else if ( value < numeric_limits< int >::min() )
							value = numeric_limits< int >::min();
						type = tokenValue::typeInteger;
						tokenValuePtr = tokenValue::tokenValuePtr( new tokenValue( ( int )value, name ) );
						break;
					}
					case CValue::siUInt1:
					case CValue::siUInt2:
					case CValue::siUInt4: {
						unsigned long value( param.GetValue( g.animation.time ) );
						if( value > numeric_limits< int >::max() )
							value = numeric_limits< int >::max();
						else if ( value < numeric_limits< int >::min() )
							value = numeric_limits< int >::min();
						type = tokenValue::typeInteger;
						tokenValuePtr = tokenValue::tokenValuePtr( new tokenValue( ( int )value, name ) );
						break;
					}
					case CValue::siFloat:
					case CValue::siDouble: {
						float value( param.GetValue( g.animation.time ) );
						type = tokenValue::typeFloat;
						tokenValuePtr = tokenValue::tokenValuePtr( new tokenValue( value, name ) );
						break;
					}
					case CValue::siString: {
						string value( CStringToString( param.GetValue( g.animation.time ) ) );
						type = tokenValue::typeString;
						tokenValuePtr = tokenValue::tokenValuePtr( new tokenValue( value, name ) );
						break;
					}

					/*
					case CValue::siIDispatch:
						break;

					siWStr:
					siRef:
					siArray:
					siPtr:
					siRefArray:*/

					case CValue::siIUnknown:
					default:
						message( L"'" + stringToCString( xsiName ) + L"' is of unknown attribute Type", messageWarning );
						break;
				}
			}
		}
	}

	attribute::attribute() {
		//tokenValuePtr = NULL;
	}

	attribute::attribute( const attribute &attrib ) {
		if( attrib.tokenValuePtr )
			tokenValuePtr = tokenValue::tokenValuePtr( new tokenValue( *( attrib.tokenValuePtr ) ) );

		//theCategory = attrib.theCategory;
	}

	attribute::attribute( const tokenValue &aTokenValue ) {
		tokenValuePtr = tokenValue::tokenValuePtr( new tokenValue( aTokenValue ) );
	}

	attribute::~attribute() {
		//if( NULL != tokenValuePtr )
		//	delete tokenValuePtr;
	}

	attribute& attribute::operator=( const attribute &attrib ) {
		if( &attrib != this ) {
			if( attrib.tokenValuePtr )
				tokenValuePtr = tokenValue::tokenValuePtr( new tokenValue( *( attrib.tokenValuePtr ) ) );
		}
		return *this;
	}

	bool attribute::operator==( const attribute &comp ) const {
		bool retVal = false;
		if( tokenValuePtr || comp.tokenValuePtr ) {
			if( tokenValuePtr->name() == comp.tokenValuePtr->name() )
				retVal = true;
		}
		return retVal;
	}

	bool attribute::valid() const {
		return ( bool )tokenValuePtr;
	}

	void attribute::write() const {
		if( valid() ) {
			using namespace ueberMan;
			ueberManInterface theRenderer;
			theRenderer.attribute( *tokenValuePtr );
		}
	}

	/**
	 * Creates XML from an attribute description.
	 *
	 * The result looks like this:
	 *
	 *	<attrib name="visibility:trace" type="int">
	 *		1
	 * 	</attrib>
	 *
	 */
	string attribute::xml() {

		std::stringstream attrib;
		attrib << "<attrib" << endl;
		attrib << " name=\"" << tokenValuePtr->name() << "\"";
		attrib << " type=\"";

		switch( tokenValuePtr->type() ) {
			case tokenValue::typeFloat:
				attrib << "float";
				break;
			case tokenValue::typeInteger:
				attrib << "int";
				break;
			case tokenValue::typeColor:
				attrib << "color";
				break;
			case tokenValue::typePoint:
				attrib << "point";
				break;
			case tokenValue::typeHomogenousPoint:
				attrib << "hpoint";
				break;
			case tokenValue::typeVector:
				attrib << "vector";
				break;
			case tokenValue::typeNormal:
				attrib << "normal";
				break;
			case tokenValue::typeMatrix:
				attrib << "matrix";
				break;
			case tokenValue::typeString:
				attrib << "string" ;
				break;
		}
		attrib << "\"";

		size_t size = tokenValuePtr->size();
		if( size > 1 )
			attrib << " size=\"" << size << "\"";
		attrib << ">" << endl;
		attrib << "\t" << tokenValuePtr->dataAsString();
		attrib << "</attrib>" << endl;

		return attrib.str();
	}


}
