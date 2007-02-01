#ifndef affogatoPass_H
#define affogatoPass_H
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


// Boost headers
#include <boost/filesystem/path.hpp>

// Standard headers
#include <string>
#include <vector>

// XSI headers
#include <xsi_value.h>

// Affogato headers
#include "affogatoTokenValue.hpp"

namespace affogato {

	using namespace std;

	class pass {
		public:
			typedef enum {
				quantizeFloat,
				quantize8,
				quantize16,
				quantize16wp1k, // 16 bit, white point at 1k
				quantize16wp2k, // 16 bit, white point at 2k
				quantize16wp4k  // 16 bit, white point at 4k
			} quantizeType;

			typedef enum {
				increasing,
				decreasing
			} lineOrderType;
									pass(	const Property& affogatoPass );
									pass(	const boost::filesystem::path& theFileName,
											const string& theName,
											const string& theFormat,
											tokenValue::parameterType theType,
											const string& thePixelFilter,
											float theFilterWidth[ 2 ],
											bool computeAlpha,
											bool associateAlpha,
											bool theExclusive,
											bool theMatte,
											quantizeType theQuantize,
											float theDither,
											bool theAutoCrop,
											lineOrderType theLineOrder );
								   ~pass();
			void					set(	const Property& affogatoPass );
			void					set(	const boost::filesystem::path& theFileName,
											const string& theName,
											const string& theFormat,
											tokenValue::parameterType theType,
											const string& thePixelFilter,
											float theFilterWidth[ 2 ],
											bool computeAlpha,
											bool associateAlpha,
											bool theExclusive,
											bool theMatte,
											quantizeType theQuantize,
											float dither,
											bool theAutoCrop,
											lineOrderType theLineOrder );
			void					write();
			string					getXML();
			boost::filesystem::path	getFileName();

		private:
			boost::filesystem::path	fileName;
			string					name;
			string					format;
			tokenValue::parameterType type;
			string					pixelFilter;
			float					filterWidth[ 2 ];
			quantizeType			quantize;
			float					dither;
			bool					computeAlpha;
			bool					associateAlpha;
			bool					exclusive;
			bool					matte;
			bool					autoCrop;
			lineOrderType			lineOrder;

			CValue getParameter( const Property &prop, const CString name, CValue::DataType type );
	};
}

#endif
