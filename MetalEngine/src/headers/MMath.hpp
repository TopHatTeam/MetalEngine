// ------------------------------------------------------
//
//	ANDREW CONNER SKATZES (c) 2025
//
//	MIT License
// 
//	Description:
//		Engine math functions and defintions
// ------------------------------------------------------

#pragma once

#include <cmath>
//#define pi 3.14159265358979323846	/*Useful for 3D projection*/
constexpr double pi = 3.14159265358979323846;

/*
	Yeah, yeah I know you don't have to do 'typedef' for C++ but I'm a C person okay
*/

typedef struct
{
	float x;
	float y;
	float z;
} FVector;

typedef struct
{
	float pitch;	/* Rotation around the X-axis (degrees)*/
	float yaw;		/* Rotation around the Y-axis (degrees)*/
	float roll;		/* Rotation around the Z-axis (degrees)*/
} FRotator;

typedef struct
{
	FVector location;	/* Translation*/
	FRotator rotation;	/* Orientation*/
	FVector scale;		/* non-uniform scale*/
} FMatrix;

/**
* @brief Sets a vectors x, y, z position
* @param Destination
* @param x
* @param y
* @param z
* @returns The input Vector's new value
*/
FVector SetVector(FVector* dest, float x, float y, float z);

/**
* @brief Adds two vectors together
* @param a
* @param b
* @returns The new added value of vector A
*/
FVector AddVector(FVector* a, FVector* b);

/**
* @brief Subtracts two vectors together
* @param a
* @param b
* @returns The new subtracted value of vector A
*/
FVector SubVector(FVector* a, FVector* b);

/**
* @brief Multiples two vectors together
* @param a
* @param b
* @returns The new multipled value of vector A
*/
FVector MulVector(FVector* a, FVector* b);

/**
* @brief Divides two vectors together
* @param a
* @param b
* @returns The new divided value of vector A
*/
FVector DivVector(FVector* a, FVector* b);

namespace engine
{
	template<typename T> struct vec3
	{
		T x;
		T y;
		T z;
		vec3<T>() : x{}, y{}, z{} {}
		vec3<T>(T X, T Y, T Z) : x{ X }, y{ Y }, z{ Z } {}
	};

	template<typename T> struct rot3
	{
		T pitch;
		T yaw;
		T roll;
		rot3<T>() : pitch{}, yaw{}, roll{} {}
		rot3<T>(T p, T y, T r) : pitch{ p }, yaw{ y }, roll{ r } {}
	};

	template<typename T> struct trans3
	{
		vec3<T> location;
		rot3<T> rotation;
		vec3<T> scale;

		trans3<T>() : location{}, rotation{}, scale{} {}
		trans3<T>(T loc, T rot, T sca) : location{ loc }, rotation{ rot }, scale{ sca } {}
	};
}