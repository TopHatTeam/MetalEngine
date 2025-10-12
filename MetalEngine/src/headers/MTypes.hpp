// ------------------------------------------------------
//
//	ANDREW CONNER SKATZES (c) 2025
//
//	MIT License
// 
//	Description:
//		Engine Types and structs definitions
// ------------------------------------------------------

#pragma once
#include "MMath.hpp"

#define EMPTY {}

using namespace engine::types;

typedef unsigned char		Byte;	/*8-bit unsigned integer*/
typedef unsigned short		Word;	/*16-bit unsigned integer*/
typedef unsigned int		DWord;	/*32-bit unsigned integer*/
typedef unsigned long long	QWord;	/*64-bit unsigned integer*/
typedef unsigned long long	usize;	
typedef signed long long	ssize;

typedef vec2<int>		vec2i;
typedef vec2<float>		vec2f;
typedef vec2<double>	vec2d;
typedef vec3<int>		vec3i;
typedef vec3<float>		vec3f;
typedef vec3<double>	vec3d;
typedef vec4<int>		vec4i;
typedef vec4<float>		vec4f;
typedef vec4<double>	vec4d;
typedef rot3<int>		rot3i;
typedef rot3<float>		rot3f;
typedef rot3<double>	rot3d;
typedef trans3<int>		trans3i;
typedef trans3<float>	trans3f;
typedef trans3<double>	trans3d;
