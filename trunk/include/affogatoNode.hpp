#ifndef affogatoNode_H
#define affogatoNode_H
/** Affogato node class stores data for a single scene graph
 *  item.
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  @par
 *  You should have received a copy of the GNU Lesser General Public
 *  License (http://www.gnu.org/licenses/lgpl.txt) along with this
 *  library; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
#include <vector>
#include <map>

// Boosy headers
#include <boost/shared_ptr.hpp>

// XSI headers
#include <xsi_property.h>
#include <xsi_value.h>

// Affogato headers
#include "affogatoHairData.hpp"
#include "affogatoGlobals.hpp"
#include "affogatoData.hpp"
#include "affogatoRenderer.hpp"
#include "affogatoShader.hpp"
#include "affogatoTokenValue.hpp"


namespace affogato {

	using namespace boost;
	using namespace std;
	using namespace ueberMan;
	using namespace XSI::MATH;

	/** Stores node data and handles motion samples.
	 *
	 *  Affogato is able to hold a copy of the entire scene in memory
	 *  before writing anything out to the renderer. The node class is
	 *  the basic container for this data. The feature to hold the entire
	 *  scene in memory is currently not used, but might come handy if
	 *  Afogato was to be proted to anothe 3d app, like Maya.
	 */
	class node {

		public:
					/** Extracts Affogato-relevant data from an X3DObject.
					 */
							node( const X3DObject &obj );
						   ~node();
					/** Writes just the node's attributes out for rendering.
					 */
					void	writeAttributes() const;
					/** Writes just the node's transformation out for rendering.
					 */
					void	writeTransform() const;
					/** Writes just the node's geometry out for rendering.
					 */
					void	writeGeometry() const;
					/** Writes the node out for rendering.
					 */
					void	write() const;
					/** Returns the node's bounding box.
					 *
					 *  @return A vector of floats with 6 elements: X min, X max, Y min, Y max, Z min, Z max.
					 */
			vector< float >	getBoundingBox() const;
					bool	isArchive() const;
					bool	isDataBox() const;
			static	void	setLookContext( const context& ctx );

		private:
					/** Helper function to delete an attribute from the node's attribute map.
					 *
					 *  @param element  The name of the element to delete from the map.
					 */
					void	deleteAttributeMapElement( const string& element );

					/** Scans a property for attributes understood by Affogato.
					 *
					 *  @param prop       The property to scan.
					 *  @param attribMap  A map that will be filled with the attributes found.
					 *                    If the map already contains an entry for an attribute, this attribute will be skipped.
					 */
					void 	scanForAttributes( const Property& prop, map< string, shared_ptr< tokenValue > >& attribMap,
								shared_ptr< shader >& surface, shared_ptr< shader >& displacement,  shared_ptr< shader >& volume );

			enum nodeType {
				nodeUndefined = 0,
				nodeLight,
				nodeSpace,
				nodeMesh,
				nodeNurb,
				nodeCurves,
				nodeHair,
				nodeParticle,
				nodeArchive,
				nodeHub,
				nodeNull,
				nodeSphere
			} node::type;

			string name;
			string fileName;
			string archive;
			string databox;
			unsigned short transformMotionSamples;
			unsigned short deformMotionSamples;
			vector< float > transformSampleTimes;
			vector< float > deformSampleTimes;
			vector< float > bound; // BBox

			bool usePref;
			double prefTime;
			string groupName;

			bool isStatic;
			long staticFrame;

#ifdef RSP
			unsigned hub, hdb;
#endif

			map< string, shared_ptr< tokenValue > >attributeMap;
			shared_ptr< shader > surface, displacement, volume;

			vector< shared_ptr< CMatrix4 > > transformSamples;
			vector< shared_ptr< data > > geometrySamples;
			map< string, vector< shared_ptr< Property > > > lookVectorMap;

			static context lookContext;

			// No support for motion blurred attributes as of now
	};
}

#endif
