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
#pragma shader_stage( fragment )

/*UMO means Universal Metal Objects*/
layout(binding = 1) uniform UMO
{
    vec4 uColor;
};

layout(binding = 2) uniform sampler2D uSample0;
layout(binding = 3) uniform sampler2D uSample1;

layout(location = 0) in vec2 uTexCoord0;
layout(location = 1) in vec2 uTexCoord1;

layout(location = 0) out vec4 vColor;

void main()
{
    vColor = texture(uSample0, uTexCoord0.xy) * texture(uSample1, uTexCoord1x.y) * uColor;
}