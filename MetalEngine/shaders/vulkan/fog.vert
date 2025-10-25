// ------------------------------------------------------
//
//	ANDREW CONNER SKATZES (c) 2025
//
//	MIT License
// 
//	Description:
//		Fog shader (fragment version)
// ------------------------------------------------------

#version 450
#pragma shader_stage( vertex )

layout(binding = 0) uniform UMO
{
	vec4 uMVPMatrixX;
	vec4 uMVPMatrixY;
	vec4 uMVPMatrixZ;
	vec4 uMVPMatrixW;
	vec4 uTexGen0X;
	vec4 uTexGen0Y;
	vec4 uTexGen1X;
	vec4 uTexGen1Y;
};

layout(location = 0) in vec3 uPosition;

layout(location = 0) out vec2 vTexCoord0;
layout(location = 1) out vec2 vTexCoord1;

void main()
{
	vec4 vPosition = vec4(uPosition, 1.0);
	gl_Position.x = dot(vPosition, uMVPMatrixX);
	gl_Position.y = dot(vPosition, uMVPMatrixY);
	gl_Position.z = dot(vPosition, uMVPMatrixZ);
	gl_Position.w = dot(vPosition, uMVPMatrixW);

	vTexCoord0.x = dot(vPosition, uTexGen0X);
	vTexCoord0.y = dot(vPosition, uTexGen0Y);

	vTexCoord1.x = dot(vPosition, uTexGen1X);
	vTexCoord1.y = dot(vPosition, uTexGen1Y);
}