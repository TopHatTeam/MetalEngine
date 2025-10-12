// ------------------------------------------------------
//
//	ANDREW CONNER SKATZES (c) 2025
//
//	MIT License
// 
//	Description:
//		Engine texture definitions
// ------------------------------------------------------

#pragma once
#include "MTypes.hpp"

enum texturetype
{
	TT_DIFFUSE					= 1,
	TT_METALLIC					= 2,
	TT_SPECULAR					= 3,
	TT_ROUGHNESS				= 4,
	TT_NORMAL					= 5,
	TT_DISPLACEMENT				= 6,
	TT_AMBIENT_OCCLUSION		= 7,	
	TT_SUBSURFACE_SCATTERING	= 8,
	TT_EMISSIVE					= 9,
	TT_OPACITY					= 10,
	TT_UNDEFINED				= 0
};

struct imagefile
{
	DWord		width;
	DWord		height;
	texturetype type;
};

