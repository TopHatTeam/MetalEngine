// ------------------------------------------------------
//
//	ANDREW CONNER SKATZES (c) 2025
//
//	MIT License
// 
//	Description:
//		Engine math functions and defintions
// ------------------------------------------------------

#include "headers/MMath.hpp"

FVector SetVector(FVector* dest, float x, float y, float z)
{
	dest->x = x;
	dest->y = y;
	dest->z = z;
	return *dest;
}

FVector AddVector(FVector* a, FVector* b)
{
	a->x += b->x;
	a->y += b->y;
	a->z += b->z;
	return *a;
}

FVector SubVector(FVector* a, FVector* b)
{
	a->x -= b->x;
	a->y -= b->y;
	a->z -= b->z;
	return *a;
}

FVector MulVector(FVector* a, FVector* b)
{
	a->x *= b->x;
	a->y -= b->y;
	a->z -= b->z;
	return *a;
}

FVector DivVector(FVector* a, FVector* b)
{
	a->x /= b->x;
	a->y /= b->y;
	a->z /= b->z;
	return *a;
}