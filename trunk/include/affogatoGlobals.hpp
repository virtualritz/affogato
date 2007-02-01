#ifndef affogatoGlobals_H
#define affogatoGlobals_H
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
#include <map>
#include <set>
#include <string>
#include <vector>

// Boost headers
#include <boost/filesystem/path.hpp>
#include <boost/detail/numeric_traits.hpp>
#include <boost/shared_ptr.hpp>

// XSI headers
#include <xsi_property.h>
#include <xsi_value.h>

// Affogato headers
#include "affogatoPass.hpp"

struct XMLNode;

#define MAXMOTIONSAMPLES 16

namespace affogato {

	using namespace XSI;
	using namespace std;

	class globals {
		public:
			// Methods
							globals() {};
							globals( const Property &affogatoGlobals );
							globals( const string &xmlFile );

			static			globals& access();
			void			aquire( const Property& affogatoGlobals );
			void			set( const Property& affogatoGlobals );
			void			set( const string& xmlFile );
			string			getPassesXML() const;
			vector< boost::filesystem::path > getPassNames() const;
			void			writePasses() const;
			void			writePassesXML( const string& destination ) const;
			bool			nextTime();
			void			resetTime();
			float			getNormalizedTime() const;

			// Data
			bool initialized;

			struct camera {
				string cameraName;
				bool depthOfField;
				bool hypeOverscan;
				float fieldOfView;
				float fStop;
				float focalDistance;
				float nearClip;
				float farClip;
				float aspect;
				bool useAspect;
				bool frontPlane;
				bool backPlane;
				bool freezeScale;
				typedef enum rotoViewStyleType {
					rotoViewStyleNone = 0,
					rotoViewStyleFillScreen = 1,
					rotoViewStyleCrop = 2,
				} rotoViewStyleType;
				rotoViewStyleType rotoViewStyle;
			} camera;

			struct resolution {
				unsigned x;
				unsigned y;
				float pixelAspect;
				float multiplier;
			} resolution;

			struct animation {
				float time;
				vector< float >times;
			} animation;

			struct motionBlur {
				unsigned deformMotionSamples;
				unsigned transformMotionSamples;
				bool geometryBlur;
				bool geometryParameterBlur;
				bool geometryVariableBlur;
				bool cameraBlur;
				bool lightBlur;
				bool attributeBlur;
				bool shaderBlur;
				bool shadowMapBlur;
				bool subFrame;
				float shutterOpen;
				float shutterClose;
				float shutterEfficiency;
				typedef enum shutterConfigurationType {
					shutterMoving = 0,
					shutterStationary = 1
				} shutterConfigurationType;
				shutterConfigurationType shutterConfiguration;
				float shutterOffset;
			} motionBlur;

			struct name {
				string baseName;
				string blockName;
				string currentFrame;
			} name;

			struct directories {
				boost::filesystem::path affogatoHome;

				boost::filesystem::path base;
				boost::filesystem::path image;
				boost::filesystem::path map;
				boost::filesystem::path data;
				boost::filesystem::path object;
				boost::filesystem::path attribute;
				boost::filesystem::path temp;
				boost::filesystem::path hub;

				boost::filesystem::path cache;

				bool relativePaths;
				bool createMissing;

				struct caching {
					bool dataWrite;
					bool dataSource;
					bool dataCopy;
					bool mapWrite;
					bool mapSource;
					bool mapCopy;
					bool imageWrite;
					bool imageCopy;
					boost::intmax_t size;
				} caching;
			} directories;

			struct searchPaths {
				string shader;
				string texture;
				string archive;
				string procedural;
				bool shaderPaths;
			} searchPath;

			struct sampling {
				unsigned short x;
				unsigned short y;
			} sampling;

			struct shading {
				float rate;
				bool smooth;
				unsigned hair;
				float shadowRate;
			} shading;

			struct filtering {
				float x;
				float y;
				string filter;
			} filtering;

			struct image {
				string displayDriver;
				pass::quantizeType displayQuantization;
				bool associateAlpha;
				float gain;
				float gamma;
			} image;

			struct reyes {
				struct bucketSize {
					unsigned x;
					unsigned y;
				} bucketSize;
				unsigned long gridSize;
				unsigned long textureMemory;
				string bucketorder;
				bool jitter;
				bool sampleMotion;
				float opacityThreshold;
				float motionFactor;
				float focusFactor;
				bool extremeMotionDepthOfField;
				unsigned eyeSplits;
			} reyes;

			struct rays {
				bool enable;
				struct trace {
					unsigned depth;
					float bias;
					bool motion;
					bool displacements;
				} trace;
				struct irradiance {
					unsigned samples;
					float shadingRate;
				} irradiance;
				struct subSurface {
					float rate;
				} subSurface;
			} rays;

			struct feedback {
				typedef enum verbosityType {
					verbosityUndefined = -1,
					verbositySilent = 0,
					verbosityErrors = 1,
					verbosityWarningsAndErrors = 2,
					verbosityAll = 3,
					verbosityDebug = 4
				} verbosityType;
				verbosityType verbosity;
				typedef enum previewDisplayType {
					previewNone = 0,
					previewFramebuffer = 1,
					previewIDisplay = 2,
					previewIt = 3,
					previewHoudini = 4
				} previewDisplayType;
				previewDisplayType previewDisplay;
				bool stopWatch;
			} feedback;

			struct defaultShaderGroup {
				string surface;
				string displacement;
				string volume;
				bool overrideAll;
			} defaultShader;

			struct baking {
				bool bake;
			} baking;

			struct geometry {
				bool normalizeNurbKnotVector;
				bool nonRationalNurbSurface;
				bool nonRationalNurbCurve;
				float defaultNurbCurveWidth;
			} geometry;

			struct renderer {
				string version;
				string command;
			} renderer;

			struct jobGlobal {
				typedef enum launchType {
					launchOff = 0,
					launchRenderer = 1,
					launchJobInterpreter= 2
				} launchType;
				launchType launch;
				bool launchSub;
				struct jobScript {
					typedef enum jobScriptType {
						jobScriptOff = 0,
						jobScriptXML = 1,
						jobScriptJobEngineXML = 2,
						jobScriptAlfred = 3
					} jobScriptType;
					jobScriptType type;
					string interpreter;
					unsigned chunkSize;
				} jobScript;
				string preJobCommand;
				string preFrameCommand;
				string postFrameCommand;
				string postJobCommand;
				unsigned short numCPUs;
				string hosts;
				bool useRemoteSSH;
				typedef enum cacheType {
					cacheTypeOff = 0,
					cacheTypeReads = 1,
					cacheTypeWrites = 2,
					cacheTypeReadsAndWrites = 3,
				} cacheType;
				cacheType cache;
				string cacheDir;
				unsigned long cacheSize;
			} jobGlobal;

			struct data {
				bool directToRenderer;
				typedef enum granularityType {
					granularityFrame,   	// All in one block (bad and hidden
											// from the UI therefore; also
											// rather untested and will break
											// shadow RIBs referencing the world
											// block
					granularitySubFrame,	// Separate World Block
					granularitySections,	// Each Section separate
					granularityObjects,     // Each Object separate
					granularityAttributes   // AtttributeBlock separate
				} granularityType;
				granularityType granularity;
				typedef enum attributeDataTypeType {
					attributeDataNative,
					attributeDataXML
				} attributeDataTypeType;
				attributeDataTypeType attributeDataType;
				bool subSectionParentTransforms;
				bool relativeTransforms;
				bool frame;
				struct sections {
					bool options;
					bool camera;
					bool world;
					bool spaces;
					bool lights;
					bool looks;
					bool geometry;
					bool attributes;
					bool shaderParameters;
					bool shaderNumericParameters;
				} sections;
				struct _data {
					bool shadow;
					granularityType granularity;
				} shadow;
				bool binary;
				bool compress;
				bool delay;
				bool doHub;
				boost::filesystem::path worldBlockName;
				bool hierarchical; // Whether we scan the scene tree hierachical from the leaf node upwards for attributes & shaders
				typedef enum scanningOrderType {
					scanningOrderGroups = 0,
					scanningOrderObjects = 1
				} scanningOrderType;
				scanningOrderType attributeScanningOrder;
				typedef enum attributeModeType {
					attributeModeLook = 0,
					attributeModeInline = 1
				} attributeModeType;
				attributeModeType attributeMode;
			} data;

			bool writeAttributeTypes;

			struct system {
				boost::filesystem::path tempDir;
			} system;

			vector< boost::shared_ptr< pass > > passPtrArray;

			// Dodgy as hell: the global set of blobby groups :(
			map< float, std::set< long > > blobbyGroupsMap;
		private:
			void			scan( const string& xmlFile );
			void			sanitize();
			void			sanitizePaths();
			bool			getBoolAttribute( XMLNode& xNode, const string& s, bool& value );
			bool			getIntAttribute( XMLNode& xNode, const string& s, int& value );
			bool			getFloatAttribute( XMLNode& xNode, const string& s, float& value );
			bool			getStringAttribute( XMLNode& xNode, const string& s, string& value );

			struct time {
				bool			doAnimation;

				typedef enum frameOutputType {
					frameCurrent = 0,
					frameStartToEnd = 1,
					frameStart = 2,
					frameSequence = 3
				} frameOutputType;
				frameOutputType frameOutput;
				string	sequence;
				float	startFrame;
				float	endFrame;
				float	frameStep;
				unsigned timeIndex;
			} time;


			// Declare singleton instance
	};
}

#endif
