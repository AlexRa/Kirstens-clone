/** 
 * @file alphaF.glsl
 *
 * Copyright (c) 2007-$CurrentYear$, Linden Research, Inc.
 * $License$
 */

#extension GL_ARB_texture_rectangle : enable

uniform sampler2D diffuseMap;
uniform sampler2DRectShadow shadowMap0;
uniform sampler2DRectShadow shadowMap1;
uniform sampler2DRectShadow shadowMap2;
uniform sampler2DRectShadow shadowMap3;
uniform sampler2D noiseMap;
uniform sampler2DRect depthMap;

uniform mat4 shadow_matrix[6];
uniform vec4 shadow_clip;
uniform vec2 screen_res;

vec3 atmosLighting(vec3 light);
vec3 scaleSoftClip(vec3 light);

varying vec3 vary_ambient;
varying vec3 vary_directional;
varying vec3 vary_fragcoord;
varying vec3 vary_position;
varying vec3 vary_light;

uniform float alpha_soften;

uniform mat4 inv_proj;

vec4 getPosition(vec2 pos_screen)
{
	float depth = texture2DRect(depthMap, pos_screen.xy).a;
	vec2 sc = pos_screen.xy*2.0;
	sc /= screen_res;
	sc -= vec2(1.0,1.0);
	vec4 ndc = vec4(sc.x, sc.y, 2.0*depth-1.0, 1.0);
	vec4 pos = inv_proj * ndc;
	pos /= pos.w;
	pos.w = 1.0;
	return pos;
}

void main() 
{
	vec2 frag = vary_fragcoord.xy/vary_fragcoord.z*0.5+0.5;
	frag *= screen_res;
	
	vec3 samp_pos = getPosition(frag).xyz;
	
	float shadow = 1.0;
	vec4 pos = vec4(vary_position, 1.0);
	
	vec4 spos = pos;
	
	if (spos.z > -shadow_clip.w)
	{	
		vec4 lpos;
		
		if (spos.z < -shadow_clip.z)
		{
			lpos = shadow_matrix[3]*spos;
			lpos.xy *= screen_res;
			shadow = shadow2DRectProj(shadowMap3, lpos).x;
			shadow += max((pos.z+shadow_clip.z)/(shadow_clip.z-shadow_clip.w)*2.0-1.0, 0.0);
		}
		else if (spos.z < -shadow_clip.y)
		{
			lpos = shadow_matrix[2]*spos;
			lpos.xy *= screen_res;
			shadow = shadow2DRectProj(shadowMap2, lpos).x;
		}
		else if (spos.z < -shadow_clip.x)
		{
			lpos = shadow_matrix[1]*spos;
			lpos.xy *= screen_res;
			shadow = shadow2DRectProj(shadowMap1, lpos).x;
		}
		else
		{
			lpos = shadow_matrix[0]*spos;
			lpos.xy *= screen_res;
			shadow = shadow2DRectProj(shadowMap0, lpos).x;
		}
	}
	
	vec4 col = vec4(vary_ambient + vary_directional.rgb*shadow, gl_Color.a);
	vec4 color = texture2D(diffuseMap, gl_TexCoord[0].xy) * col;
	
	color.rgb = atmosLighting(color.rgb);

	color.rgb = scaleSoftClip(color.rgb);

	if (samp_pos.z != 0.0)
	{
		float dist_factor = alpha_soften;
		float a = gl_Color.a;
		a *= a;
		dist_factor *= 1.0/(1.0-a);
		color.a *= min((pos.z-samp_pos.z)*dist_factor, 1.0);
	}
	
	//gl_FragColor = gl_Color;
	gl_FragColor = color;
	//gl_FragColor = vec4(1,0,1,1)*shadow;
	
}

