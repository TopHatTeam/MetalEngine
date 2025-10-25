// ------------------------------------------------------
//
//	ANDREW CONNER SKATZES (c) 2025
//
//	MIT License
// 
//	Description:
//		Engine data packing system (like Unreal Engine 3's UPK or DOOM's WAD)
// ------------------------------------------------------

#ifndef MDATAPACKAGE_H
#define MDATAPACKAGE_H

/*Why are we using C?? 
    BECAUSE its easier then C++ to create file formats
    also we don't need to use Object-Orinted-Programming for simple package file.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>      /*Basic things really*/
#include <stdlib.h>     /*For malloc and realloc*/
#include <string.h>     /*For handling string functions*/

/* Metal Engine Package File
    - void* data    -> Pointer to the raw data 
    - size_t offset -> The Offset to the start of the next file
    - const char* encryption_id -> The id used for encrypting the data in package
    - unsigned int code_package -> Does this package contain only code? (only uses 2 bits in its form)
    - unsigned int assets_package -> Does this package contain only assets? (only uses 2 bits in its form)
*/
typedef struct MEPF
{
    void* data;                 
    size_t offset;
    const char* encryption_id;
    unsigned int code_package : 1;
    unsigned int assets_package : 1;
} MEPF;

#ifdef __cplusplus
}
#endif

#endif