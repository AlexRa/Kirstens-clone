/** 
 * @file bumpF.glsl
 *
 * Copyright (c) 2007-$CurrentYear$, Linden Research, Inc.
 * $License$
 */

uniform sampler2D diffuseMap;
uniform sampler2D bumpMap;

varying vec3 vary_mat0;
varying vec3 vary_mat1;
varying vec3 vary_mat2;

void main() 
{
	vec3 col = texture2D(diffuseMap, gl_TexCoord[0].xy).rgb;
	vec3 norm = texture2D(bumpMap, gl_TexCoord[0].xy).rgb * 2.0 - 1.0;

	vec3 tnorm = vec3(dot(norm,vary_mat0),
					  dot(norm,vary_mat1),
					  dot(norm,vary_mat2));
						
	gl_FragData[0] = vec4(gl_Color.rgb*col, 0.0);
	gl_FragData[1] = vec4(vec3(gl_Color.a), gl_Color.a+(1.0-gl_Color.a)*gl_Color.a);
	gl_FragData[2] = vec4(normalize(tnorm)*0.5+0.5, 0.0);
}
