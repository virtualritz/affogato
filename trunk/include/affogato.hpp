#ifndef affogato_H
#define affogato_H
/** The Affogato plugin takes care of registering the plugin with XSI
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

#if defined(WIN32) && !defined(DEBUG)
  // Disable double -> float conversion and signed <> unsigned mismatch warnings
  #pragma warning( disable : 4244 4305 4018 )
#endif

// Standard headers
#include <string>

#if defined(WIN32) && !defined(DEFINED_AFFOGATOVERSION)
// unix build gets this from the Makefile
static const std::string AFFOGATOVERSION =
#include "affogato.version"
;
#define DEFINED_AFFOGATOVERSION
#endif

#ifdef unix
#define XSIPLUGINCALLBACK extern "C"
#endif

#define LABEL_WIDTH_SMALL 80
#define LABEL_WIDTH 96
#define LABEL_WIDTH_WIDE 128


#endif
