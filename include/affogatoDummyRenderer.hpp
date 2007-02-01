#ifndef dummyRenderer_H
#define dummyRenderer_H

#include <vector>
#include <string>

#include "affogatoRenderer.h"
#include "affogatoTokenValue.h"

#define MAXMOTIONSAMPLES 16

namespace ueberMan {

	using namespace std;
	using namespace affogato;

	class ueberManDummyRenderer : public ueberMan {
		public:
								ueberManDummyRenderer() {}
							   ~ueberManDummyRenderer() {}
			context	beginScene( const string &destination, bool useBinary = false, bool useCompression = false ) {}
			void	switchScene( context ctx ) {}
			void	endScene( context ctx ) {}

			const	ueberManDummyRenderer& accessRenderer() { return *this; }

			//static const	UeberManDummyRenderer*	accessRendererStatic() {}

			void	input( const string &filename ) {}
			void	input( const string &filename, const float *bound ) {}

			void	world() {}
			void	render( const cameraHandle &cameraname ) {}

			void	camera( cameraHandle& cameraid ) {}
			void	output( const string &name, const string &format,
							const string &dataname, const cameraHandle& camerid ) {}

			void	motion( const vector< float >& times ) {}

			void	parameter( const std::vector< tokenValue > &tokenValueArray ) {}
			void	parameter( const tokenValue &aTokenValue ) {}
			void	parameter( const string &typedname, const string &value ) {}
			void	parameter( const string &typedname, const float value ) {}
			void	parameter( const string &typedname, const int value ) {}
			void	parameter( const string &typedname, const bool value ) {}

			void	attribute( const tokenValue &aTokenValue ) {}
			void	attribute( const string &typedname, const string &value ) {}
			void	attribute( const string &typedname, const float value ) {}
			void	attribute( const string &typedname, const int value ) {}
			void	attribute( const string &typedname, const bool value ) {}

			bool	getAttribute( const string &typedname, float &value ) {}
			bool	getAttribute( const string &typedname, int &value ) {}
			bool	getAttribute( const string &typedname, string &value ) {}

			void	pushAttributes() {}
			void	popAttributes() {}

			void	option( const tokenValue &aTokenValue ) {}
			void	option( const string &typedname, const string &value ) {}
			void	option( const string &typedname, const float value ) {}
			void	option( const string &typedname, const int value ) {}
			void	option( const string &typedname, const bool value ) {}

			void	pushSpace() {}
			void	popSpace() {}

			void	space( const float *matrix ) {}
			void	space( const spaceHandle& spacename ) {}
			void	nameSpace( spaceHandle& spacename ) {}
			void	appendSpace( const float *matrix ) {}

			void	translate( const float x, const float y, const float z ) {}
			void	rotate( const float angle, const float x, const float y, const float z ) {}
			void	scale( const float x, const float y, const float z ) {}

			void	shader( const string &shadertype, const string &shadername, shaderHandle& shaderid ) {}
			void	light( const string &shadername, lightHandle& lightid ) {}
			void	switchLight( const lightHandle &lightid, const bool on ) {}

			void	beginLook( lookHandle& lookid ) {}
			void	endLook() {}
			void	look( const lookHandle& id ) {}
			void	appendLook( const lookHandle& id ) {}

			void	curves( const string& interp, const int ncurves, const int nvertspercurve, const bool closed, primitiveHandle &identifier ) {}
			void	curves( const string& interp, const int ncurves, const int *nvertspercurve, const bool closed, primitiveHandle &identifier ) {}

			void	patch( const string& interp, const int nu, const int nv, primitiveHandle &identifier ) {}
			void	patch(	const int nu, const int uorder, const float *uknot,	const float umin, const float umax,
							const int nv, const int vorder, const float *vknot,	const float vmin, const float vmax, primitiveHandle &identifier ) {}

			void 	mesh( const string& interp, const int nfaces, const int *nverts, const int *verts, const bool interpolateBoundary, primitiveHandle &identifier ) {}

			void	sphere(	const float radius, const float zmin, const float zmax, const float thetamax, primitiveHandle &identifier ) {}

	};
}

#endif

