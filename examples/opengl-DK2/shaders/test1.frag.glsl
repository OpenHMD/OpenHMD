#version 120

// Taken from mts3d forums, from user fredrik.

uniform sampler2D warpTexture;

const vec2 LeftLensCenter = vec2(0.25, 0.5);
const vec2 RightLensCenter = vec2(0.75, 0.5);
const vec2 LeftScreenCenter = vec2(0.25, 0.5);
const vec2 RightScreenCenter = vec2(0.75, 0.5);
const vec2 Scale = vec2(0.1469278, 0.2350845);
const vec2 ScaleIn = vec2(4, 2.5);
const vec4 HmdWarpParam   = vec4(1, 0.2, 0.1, 0);
const float aberr_r = 0.985;
const float aberr_b = 1.015;
// const vec4 HmdWarpParam   = vec4(1, 0.5, 0.05, 0);


void main()
{
	// The following two variables need to be set per eye
	vec2 LensCenter = gl_FragCoord.x < 960 ? LeftLensCenter : RightLensCenter;
	vec2 ScreenCenter = gl_FragCoord.x < 960 ? LeftScreenCenter : RightScreenCenter;

	vec2 oTexCoord = gl_FragCoord.xy / vec2(1920, 1080);


	vec2 theta = (oTexCoord - LensCenter) * ScaleIn; // Scales to [-1, 1]
	float rSq = theta.x * theta.x + theta.y * theta.y;
	vec2 rvector = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq +
		HmdWarpParam.z * rSq * rSq +
		HmdWarpParam.w * rSq * rSq * rSq);
	vec2 tc_r = LensCenter + aberr_r * Scale * rvector;
	vec2 tc_g = LensCenter +           Scale * rvector;
	vec2 tc_b = LensCenter + aberr_b * Scale * rvector;
	tc_r.x = gl_FragCoord.x < 960 ? (2.0 * tc_r.x) : (2.0 * (tc_r.x - 0.5));
	tc_g.x = gl_FragCoord.x < 960 ? (2.0 * tc_g.x) : (2.0 * (tc_g.x - 0.5));
	tc_b.x = gl_FragCoord.x < 960 ? (2.0 * tc_b.x) : (2.0 * (tc_b.x - 0.5));

	float rval = 0.0;
	float gval = 0.0;
	float bval = 0.0;

	rval = texture2D(warpTexture, tc_r).x;
	gval = texture2D(warpTexture, tc_g).y;
	bval = texture2D(warpTexture, tc_b).z;

	gl_FragColor = vec4(rval,gval,bval,1.0);
}
