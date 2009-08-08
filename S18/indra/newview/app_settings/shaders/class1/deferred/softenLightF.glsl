/** 
 * @file softenLightF.glsl
 *
 * Copyright (c) 2007-$CurrentYear$, Linden Research, Inc.
 * $License$
 */

#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect diffuseRect;
uniform sampler2DRect specularRect;
uniform sampler2DRect normalMap;
uniform sampler2DRect lightMap;
uniform sampler2D	  noiseMap;
uniform samplerCube environmentMap;
uniform sampler2D		diffuseGIMap;
uniform sampler2D		normalGIMap;
uniform sampler2D		minpGIMap;
uniform sampler2D		maxpGIMap;

uniform float blur_size;
uniform float blur_fidelity;

// Inputs
uniform vec4 morphFactor;
uniform vec3 camPosLocal;
//uniform vec4 camPosWorld;
uniform vec4 gamma;
uniform vec4 lightnorm;
uniform vec4 sunlight_color;
uniform vec4 ambient;
uniform vec4 blue_horizon;
uniform vec4 blue_density;
uniform vec4 haze_horizon;
uniform vec4 haze_density;
uniform vec4 cloud_shadow;
uniform vec4 density_multiplier;
uniform vec4 distance_multiplier;
uniform vec4 max_y;
uniform vec4 glow;
uniform float scene_light_strength;
uniform vec3 env_mat[3];
uniform vec4 shadow_clip;
uniform mat3 ssao_effect_mat;

uniform mat4 gi_mat;  //gPipeline.mGIMatrix - eye space to sun space
uniform mat4 gi_mat_proj; //gPipeline.mGIMatrixProj - eye space to projected sun space
uniform mat4 gi_norm_mat; //gPipeline.mGINormalMatrix - eye space normal to sun space normal matrix
uniform mat4 gi_inv_proj; //gPipeline.mGIInvProj - projected sun space to sun space
uniform float gi_radius;
uniform float gi_intensity;
uniform vec2 gi_kern[25];
uniform vec2 gi_scale;
uniform vec3 gi_quad;
uniform float gi_direction_weight;
uniform float gi_light_offset;

uniform sampler2DRect depthMap;
uniform mat4 inv_proj;
uniform vec2 screen_res;

varying vec4 vary_light;
varying vec2 vary_fragcoord;

vec3 vary_PositionEye;

vec3 vary_SunlitColor;
vec3 vary_AmblitColor;
vec3 vary_AdditiveColor;
vec3 vary_AtmosAttenuation;

vec4 getPosition(vec2 pos_screen)
{ //get position in screen space (world units) given window coordinate and depth map
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

vec4 giAmbient(vec3 pos, vec3 norm)
{
	vec4 gi_c = gi_mat_proj * vec4(pos, 1.0);
	gi_c.xyz /= gi_c.w;

	vec3 nz = texture2D(noiseMap, vary_fragcoord.xy/128.0).xyz;
			
	vec4 gi_pos = gi_mat*vec4(pos,1.0);
	vec3 gi_norm = (gi_norm_mat*vec4(norm,1.0)).xyz;
	gi_norm = normalize(gi_norm);
	
	vec3 eye_dir = vec3(0,0,-1);
	eye_dir = (gi_norm_mat*vec4(eye_dir, 1.0)).xyz;
	eye_dir = normalize(eye_dir);
	
	float round_x = gi_scale.x*0.5;
	float round_y = gi_scale.y*0.5;
	
	gi_c.x = floor(gi_c.x/round_x+0.5)*round_x;
	gi_c.y = floor(gi_c.y/round_y+0.5)*round_y;
	
	float fda = 0.0;
	vec3 fdiff = vec3(0,0,0);
	
	vec3 rcol = vec3(0,0,0);
	
	float fsa = 0.0;
	
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; ++j)
		{
			vec2 tc = vec2((i-2+0.5)/2.0, (j-2+0.5)/2.0);
			tc *= gi_scale.xy;
			tc += gi_c.xy;
			vec3 lnorm = texture2D(normalGIMap, tc.xy).xyz;
			vec3 center = texture2D(minpGIMap, tc.xy).xyz;
			vec3 size = texture2D(maxpGIMap, tc.xy).xyz;
			vec3 diff = texture2D(diffuseGIMap, tc.xy).xyz;
				
			vec3 ldir = reflect(vec3(0,0,-1), lnorm);
			
			vec3 lpos = center;
					
			vec3 at = lpos-gi_pos.xyz;
					
			if (at.z > 0.0)
			{
				float z = max(at.z-size.z, 0.0);
				at.z = z;
			}
			else
			{
				float z = min(at.z+size.z, 0.0);
				at.z = z;
			}
			
			at.z = max(at.z, gi_radius*0.5);
			
			float dist = length(at);
			float lpdist = -dot(vec3(center.xy, center.z-size.z)-gi_pos, lnorm);
			lpdist = clamp(lpdist+gi_light_offset+length(at.xy)*size.x+gi_direction_weight, size.y, 1.0);
			
			float da =	clamp((dist/lpdist-gi_radius*0.3)/(gi_radius*0.7), 0.0, 1.0);
			da = 1.0-da;
			
			//add angular attenuation
			ldir = normalize(at-ldir*(1.0-size.x)*0.5*dist);
			float ang_atten = clamp(dot(ldir, gi_norm), 0.0, 1.0);
			ang_atten = clamp(ang_atten+da*da*0.1, 0.0, 1.0);
			
			
			//add specular attenuation
			float sa = clamp(dot(reflect(eye_dir, gi_norm), ldir), 0.0, 1.0);
			sa *= sa;
			sa *= sa;
			fsa += sa*da;
			
			da *= ang_atten;
			fda += da;
			fdiff += diff*da;
		}
	}

	fdiff /= max(fda, gi_quad.z);
	fsa /= max(fda, 0.25);
	

	fda /= 16.0;
	fda = sqrt(fda);
	fda = fda*fda*gi_quad.x+fda*gi_quad.y+gi_quad.z;			

	fda *= nz.z;
	
	//rcol.rgb *= gi_intensity;
	//return rcol.rgb+vary_AmblitColor.rgb*0.25;
	//return debug;
	return vec4(clamp(fda*fdiff,vec3(gi_quad.z), vec3(1.0)), fsa);
}

vec3 getPositionEye()
{
	return vary_PositionEye;
}
vec3 getSunlitColor()
{
	return vary_SunlitColor;
}
vec3 getAmblitColor()
{
	return vary_AmblitColor;
}
vec3 getAdditiveColor()
{
	return vary_AdditiveColor;
}
vec3 getAtmosAttenuation()
{
	return vary_AtmosAttenuation;
}


void setPositionEye(vec3 v)
{
	vary_PositionEye = v;
}

void setSunlitColor(vec3 v)
{
	vary_SunlitColor = v;
}

void setAmblitColor(vec3 v)
{
	vary_AmblitColor = v;
}

void setAdditiveColor(vec3 v)
{
	vary_AdditiveColor = v;
}

void setAtmosAttenuation(vec3 v)
{
	vary_AtmosAttenuation = v;
}

void calcAtmospherics(vec3 inPositionEye, float ambFactor) {

	vec3 P = inPositionEye;
	setPositionEye(P);
	
	//(TERRAIN) limit altitude
	if (P.y > max_y.x) P *= (max_y.x / P.y);
	if (P.y < -max_y.x) P *= (-max_y.x / P.y);

	vec3 tmpLightnorm = lightnorm.xyz;

	vec3 Pn = normalize(P);
	float Plen = length(P);

	vec4 temp1 = vec4(0);
	vec3 temp2 = vec3(0);
	vec4 blue_weight;
	vec4 haze_weight;
	vec4 sunlight = sunlight_color;
	vec4 light_atten;

	//sunlight attenuation effect (hue and brightness) due to atmosphere
	//this is used later for sunlight modulation at various altitudes
	light_atten = (blue_density * 1.0 + vec4(haze_density.r) * 0.25) * (density_multiplier.x * max_y.x);
		//I had thought blue_density and haze_density should have equal weighting,
		//but attenuation due to haze_density tends to seem too strong

	temp1 = blue_density + vec4(haze_density.r);
	blue_weight = blue_density / temp1;
	haze_weight = vec4(haze_density.r) / temp1;

	//(TERRAIN) compute sunlight from lightnorm only (for short rays like terrain)
	temp2.y = max(0.0, tmpLightnorm.y);
	temp2.y = 1. / temp2.y;
	sunlight *= exp( - light_atten * temp2.y);

	// main atmospheric scattering line integral
	temp2.z = Plen * density_multiplier.x;

	// Transparency (-> temp1)
	// ATI Bugfix -- can't store temp1*temp2.z*distance_multiplier.x in a variable because the ati
	// compiler gets confused.
	temp1 = exp(-temp1 * temp2.z * distance_multiplier.x);

	//final atmosphere attenuation factor
	setAtmosAttenuation(temp1.rgb);
	
	//compute haze glow
	//(can use temp2.x as temp because we haven't used it yet)
	temp2.x = dot(Pn, tmpLightnorm.xyz);
	temp2.x = 1. - temp2.x;
		//temp2.x is 0 at the sun and increases away from sun
	temp2.x = max(temp2.x, .03);	//was glow.y
		//set a minimum "angle" (smaller glow.y allows tighter, brighter hotspot)
	temp2.x *= glow.x;
		//higher glow.x gives dimmer glow (because next step is 1 / "angle")
	temp2.x = pow(temp2.x, glow.z);
		//glow.z should be negative, so we're doing a sort of (1 / "angle") function

	//add "minimum anti-solar illumination"
	temp2.x += .25;
	
	//increase ambient when there are more clouds
	vec4 tmpAmbient = ambient + (vec4(1.) - ambient) * cloud_shadow.x * 0.5;
	
	/*  decrease value and saturation (that in HSV, not HSL) for occluded areas
	 * // for HSV color/geometry used here, see http://gimp-savvy.com/BOOK/index.html?node52.html
	 * // The following line of code performs the equivalent of:
	 * float ambAlpha = tmpAmbient.a;
	 * float ambValue = dot(vec3(tmpAmbient), vec3(0.577)); // projection onto <1/rt(3), 1/rt(3), 1/rt(3)>, the neutral white-black axis
	 * vec3 ambHueSat = vec3(tmpAmbient) - vec3(ambValue);
	 * tmpAmbient = vec4(RenderSSAOEffect.valueFactor * vec3(ambValue) + RenderSSAOEffect.saturationFactor *(1.0 - ambFactor) * ambHueSat, ambAlpha);
	 */
	tmpAmbient = vec4(mix(ssao_effect_mat * tmpAmbient.rgb, tmpAmbient.rgb, ambFactor), tmpAmbient.a);

	//haze color
	setAdditiveColor(
		vec3(blue_horizon * blue_weight * (sunlight*(1.-cloud_shadow.x) + tmpAmbient)
	  + (haze_horizon.r * haze_weight) * (sunlight*(1.-cloud_shadow.x) * temp2.x
		  + tmpAmbient)));

	//brightness of surface both sunlight and ambient
	setSunlitColor(vec3(sunlight * .5));
	setAmblitColor(vec3(tmpAmbient * .25));
	setAdditiveColor(getAdditiveColor() * vec3(1.0 - temp1));
}

vec3 atmosLighting(vec3 light)
{
	light *= getAtmosAttenuation().r;
	light += getAdditiveColor();
	return (2.0 * light);
}

vec3 atmosTransport(vec3 light) {
	light *= getAtmosAttenuation().r;
	light += getAdditiveColor() * 2.0;
	return light;
}
vec3 atmosGetDiffuseSunlightColor()
{
	return getSunlitColor();
}

vec3 scaleDownLight(vec3 light)
{
	return (light / scene_light_strength );
}

vec3 scaleUpLight(vec3 light)
{
	return (light * scene_light_strength);
}

vec3 atmosAmbient(vec3 light)
{
	return getAmblitColor() + light / 2.0;
}

vec3 atmosAffectDirectionalLight(float lightIntensity)
{
	return getSunlitColor() * lightIntensity;
}

vec3 scaleSoftClip(vec3 light)
{
	//soft clip effect:
	light = 1. - clamp(light, vec3(0.), vec3(1.));
	light = 1. - pow(light, gamma.xxx);

	return light;
}

void main() 
{
	vec2 tc = vary_fragcoord.xy;
	vec3 pos = getPosition(tc).xyz;
	vec3 norm = texture2DRect(normalMap, tc).xyz*2.0-1.0;
	vec3 nz = texture2D(noiseMap, vary_fragcoord.xy/128.0).xyz;
	
	float da = max(dot(norm.xyz, vary_light.xyz), 0.0);
	
	vec4 diffuse = vec4(texture2DRect(diffuseRect, tc).rgb, 1.0);
	vec4 spec = texture2DRect(specularRect, vary_fragcoord.xy);
	
	vec2 scol_ambocc = texture2DRect(lightMap, vary_fragcoord.xy).rg;
	float scol = scol_ambocc.r; 
	float ambocc = scol_ambocc.g;
	
	calcAtmospherics(pos.xyz, ambocc);
	
	vec3 col = vec3(0,0,0);
	
	col += atmosAffectDirectionalLight(min(da, scol));
	
	vec4 amb_col = giAmbient(pos, norm);
	
	col = min(col+amb_col.rgb*ambocc, vec3(0.8,0.8,0.8));
	
	//col = giAmbient(pos,norm).rgb; //*ambocc;
	/*if (spec.a > 0.2)
	{
		vec3 ref = reflect(pos.xyz, norm.xyz);
		vec3 rc;
		rc.x = dot(ref, env_mat[0]);
		rc.y = dot(ref, env_mat[1]);
		rc.z = dot(ref, env_mat[2]);
		
		vec3 refcol = textureCube(environmentMap, rc).rgb;
		col.rgb = col.rgb*refcol * spec.rgb+col.rgb*diffuse.rgb; 
	}
	else*/
	{
		col *= diffuse.rgb;
	}
	
	col.rgb += spec.rgb*amb_col.rgb*amb_col.a;
	
	col = atmosLighting(col);
	col = scaleSoftClip(col);
		
	
	gl_FragColor.rgb = col*vec3(1.0+1.0/2.2);
	gl_FragColor.a = 0.0;
	
	//gl_FragColor.rg = scol_ambocc.rg;
	//gl_FragColor.rgb = texture2DRect(lightMap, vary_fragcoord.xy).rgb;
	//gl_FragColor.rgb = norm.rgb*0.5+0.5;
	//gl_FragColor.rgb = vec3(ambocc);
	//gl_FragColor.rgb = vec3(scol);
}
