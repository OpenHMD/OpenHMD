/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* OpenGL Test - Main Implementation */

#include <openhmd.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include "gl.h"


#define OVERSAMPLE_SCALE 2.0

char* read_file(const char* filename)
{
	FILE* f = fopen(filename, "rb");
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* buffer = calloc(1, len + 1);
	assert(buffer);

	size_t ret = fread(buffer, len, 1, f);
	assert(ret);

	fclose(f);

	return buffer;
}

float randf()
{
	return (float)rand() / (float)RAND_MAX;
}

GLuint gen_cubes()
{
	GLuint list = glGenLists(1);

	// Set the random seed.
	srand(42);

	glNewList(list, GL_COMPILE);

	for(float a = 0.0f; a < 360.0f; a += 20.0f){
		glPushMatrix();

		glRotatef(a, 0, 1, 0);
		glTranslatef(0, 0, -1);
		glScalef(0.2, 0.2, 0.2);
		glRotatef(randf() * 360, randf(), randf(), randf());

		glColor4f(randf(), randf(), randf(), randf() * .5f + .5f);
		draw_cube();

		glPopMatrix();
	}

	// draw floor
	glColor4f(0, 1.0f, .25f, .25f);
	glTranslatef(0, -2.5f, 0);
	draw_cube();

	glEndList();

	return list;
}

void draw_crosshairs(float len, float cx, float cy)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glBegin(GL_LINES);
	float l = len/2.0f;
	glVertex3f(cx - l, cy, 0.0);
	glVertex3f(cx + l, cy, 0.0);
	glVertex3f(cx, cy - l, 0.0);
	glVertex3f(cx, cy + l, 0.0);
	glEnd();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void draw_scene(GLuint list)
{
	// draw cubes
	glCallList(list);
}
static inline void
print_matrix(float m[])
{
    printf("[[%0.4f, %0.4f, %0.4f, %0.4f],\n"
            "[%0.4f, %0.4f, %0.4f, %0.4f],\n"
            "[%0.4f, %0.4f, %0.4f, %0.4f],\n"
            "[%0.4f, %0.4f, %0.4f, %0.4f]]\n",
            m[0], m[4], m[8], m[12],
            m[1], m[5], m[9], m[13],
            m[2], m[6], m[10], m[14],
            m[3], m[7], m[11], m[15]);
}

int main(int argc, char** argv)
{
	int hmd_w, hmd_h;

	ohmd_context* ctx = ohmd_ctx_create();
	int num_devices = ohmd_ctx_probe(ctx);
	if(num_devices < 0){
		printf("failed to probe devices: %s\n", ohmd_ctx_get_error(ctx));
		return 1;
	}

	ohmd_device_settings* settings = ohmd_device_settings_create(ctx);

	// If OHMD_IDS_AUTOMATIC_UPDATE is set to 0, ohmd_ctx_update() must be called at least 10 times per second.
	// It is enabled by default.

	int auto_update = 1;
	ohmd_device_settings_seti(settings, OHMD_IDS_AUTOMATIC_UPDATE, &auto_update);

	ohmd_device* hmd = ohmd_list_open_device_s(ctx, 0, settings);
	if(!hmd){
		printf("failed to open device: %s\n", ohmd_ctx_get_error(ctx));
		return 1;
	}
	ohmd_device_geti(hmd, OHMD_SCREEN_HORIZONTAL_RESOLUTION, &hmd_w);
	ohmd_device_geti(hmd, OHMD_SCREEN_VERTICAL_RESOLUTION, &hmd_h);
	float ipd;
	ohmd_device_getf(hmd, OHMD_EYE_IPD, &ipd);
	float viewport_scale[2];
	float distortion_coeffs[4];
	float aberr_scale[3];
	float sep;
	float left_lens_center[2];
	float right_lens_center[2];
	//viewport is half the screen
	ohmd_device_getf(hmd, OHMD_SCREEN_HORIZONTAL_SIZE, &(viewport_scale[0]));
	viewport_scale[0] /= 2.0f;
	ohmd_device_getf(hmd, OHMD_SCREEN_VERTICAL_SIZE, &(viewport_scale[1]));
	//distortion coefficients
	ohmd_device_getf(hmd, OHMD_UNIVERSAL_DISTORTION_K, &(distortion_coeffs[0]));
	ohmd_device_getf(hmd, OHMD_UNIVERSAL_ABERRATION_K, &(aberr_scale[0]));
	//calculate lens centers (assuming the eye separation is the distance between the lens centers)
	ohmd_device_getf(hmd, OHMD_LENS_HORIZONTAL_SEPARATION, &sep);
	ohmd_device_getf(hmd, OHMD_LENS_VERTICAL_POSITION, &(left_lens_center[1]));
	ohmd_device_getf(hmd, OHMD_LENS_VERTICAL_POSITION, &(right_lens_center[1]));
	left_lens_center[0] = viewport_scale[0] - sep/2.0f;
	right_lens_center[0] = sep/2.0f;
	//assume calibration was for lens view to which ever edge of screen is further away from lens center
	float warp_scale = (left_lens_center[0] > right_lens_center[0]) ? left_lens_center[0] : right_lens_center[0];
	float warp_adj = 1.0f;

	ohmd_device_settings_destroy(settings);

	gl_ctx gl;
	init_gl(&gl, hmd_w, hmd_h);

	SDL_ShowCursor(SDL_DISABLE);

	const char* vertex;
	ohmd_gets(OHMD_GLSL_DISTORTION_VERT_SRC, &vertex);
	const char* fragment;
	ohmd_gets(OHMD_GLSL_DISTORTION_FRAG_SRC, &fragment);

	GLuint shader = compile_shader(vertex, fragment);
	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "warpTexture"), 0);
	glUniform2fv(glGetUniformLocation(shader, "ViewportScale"), 1, viewport_scale);
	glUniform3fv(glGetUniformLocation(shader, "aberr"), 1, aberr_scale);
	glUseProgram(0);

	GLuint list = gen_cubes();

	int eye_w = hmd_w/2*OVERSAMPLE_SCALE;
	int eye_h = hmd_h*OVERSAMPLE_SCALE;
	GLuint left_color_tex = 0, left_depth_tex = 0, left_fbo = 0;
	create_fbo(eye_w, eye_h, &left_fbo, &left_color_tex, &left_depth_tex);

	GLuint right_color_tex = 0, right_depth_tex = 0, right_fbo = 0;
	create_fbo(eye_w, eye_h, &right_fbo, &right_color_tex, &right_depth_tex);


	bool done = false;
	bool crosshair_overlay = false;
	while(!done){
		ohmd_ctx_update(ctx);

		SDL_Event event;
		while(SDL_PollEvent(&event)){
			if(event.type == SDL_KEYDOWN){
				switch(event.key.keysym.sym){
				case SDLK_ESCAPE:
					done = true;
					break;
				case SDLK_F1:
					{
						gl.is_fullscreen = !gl.is_fullscreen;
						SDL_SetWindowFullscreen(gl.window, gl.is_fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
					}
					break;
				case SDLK_F2:
					{
						// reset rotation and position
						float zero[] = {0, 0, 0, 1};
						ohmd_device_setf(hmd, OHMD_ROTATION_QUAT, zero);
						ohmd_device_setf(hmd, OHMD_POSITION_VECTOR, zero);
					}
					break;
				case SDLK_F3:
					{
						float mat[16];
						ohmd_device_getf(hmd, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX, mat);
						printf("Projection L: ");
						print_matrix(mat);
						printf("\n");
						ohmd_device_getf(hmd, OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX, mat);
						printf("Projection R: ");
						print_matrix(mat);
						printf("\n");
						ohmd_device_getf(hmd, OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX, mat);
						printf("View: ");
						print_matrix(mat);
						printf("\n");
						printf("viewport_scale: [%0.4f, %0.4f]\n", viewport_scale[0], viewport_scale[1]);
						printf("lens separation: %04f\n", sep);
						printf("IPD: %0.4f\n", ipd);
						printf("warp_scale: %0.4f\r\n", warp_scale);
						printf("distortion coeffs: [%0.4f, %0.4f, %0.4f, %0.4f]\n", distortion_coeffs[0], distortion_coeffs[1], distortion_coeffs[2], distortion_coeffs[3]);
						printf("aberration coeffs: [%0.4f, %0.4f, %0.4f]\n", aberr_scale[0], aberr_scale[1], aberr_scale[2]);
						printf("left_lens_center: [%0.4f, %0.4f]\n", left_lens_center[0], left_lens_center[1]);
						printf("right_lens_center: [%0.4f, %0.4f]\n", right_lens_center[0], right_lens_center[1]);
					}
					break;
				case SDLK_w:
					sep += 0.001;
					left_lens_center[0] = viewport_scale[0] - sep/2.0f;
					right_lens_center[0] = sep/2.0f;
					break;
				case SDLK_q:
					sep -= 0.001;
					left_lens_center[0] = viewport_scale[0] - sep/2.0f;
					right_lens_center[0] = sep/2.0f;
					break;
				case SDLK_a:
					warp_adj *= 1.0/0.9;
					break;
				case SDLK_z:
					warp_adj *= 0.9;
					break;
				case SDLK_i:
					ipd -= 0.001;
					ohmd_device_setf(hmd, OHMD_EYE_IPD, &ipd);
					break;
				case SDLK_o:
					ipd += 0.001;
					ohmd_device_setf(hmd, OHMD_EYE_IPD, &ipd);
					break;
				case SDLK_d:
					/* toggle between distorted and undistorted views */
					if ((distortion_coeffs[0] != 0.0) ||
							(distortion_coeffs[1] != 0.0) ||
							(distortion_coeffs[2] != 0.0) ||
							(distortion_coeffs[3] != 1.0)) {
						distortion_coeffs[0] = 0.0;
						distortion_coeffs[1] = 0.0;
						distortion_coeffs[2] = 0.0;
						distortion_coeffs[3] = 1.0;
					} else {
						ohmd_device_getf(hmd, OHMD_UNIVERSAL_DISTORTION_K, &(distortion_coeffs[0]));
					}
					break;
				case SDLK_x:
					crosshair_overlay = ! crosshair_overlay;
					break;
				default:
					break;
				}
			}
		}

		// Common scene state
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		float matrix[16];

		// set hmd rotation, for left eye.
		glMatrixMode(GL_PROJECTION);
		ohmd_device_getf(hmd, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX, matrix);
		glLoadMatrixf(matrix);

		glMatrixMode(GL_MODELVIEW);
		ohmd_device_getf(hmd, OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX, matrix);
		glLoadMatrixf(matrix);

		// Draw scene into framebuffer.
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, left_fbo);
		glViewport(0, 0, eye_w, eye_h);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw_scene(list);
		if (crosshair_overlay) {
			glClear(GL_DEPTH_BUFFER_BIT);
			glLineWidth(2.0*OVERSAMPLE_SCALE);
			glColor4f(1.0, 0.5, 0.0, 1.0);
			draw_crosshairs(0.1, 2*left_lens_center[0]/viewport_scale[0] - 1.0f, 2*left_lens_center[1]/viewport_scale[1] - 1.0f);
		}

		// set hmd rotation, for right eye.
		glMatrixMode(GL_PROJECTION);
		ohmd_device_getf(hmd, OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX, matrix);
		glLoadMatrixf(matrix);

		glMatrixMode(GL_MODELVIEW);
		ohmd_device_getf(hmd, OHMD_RIGHT_EYE_GL_MODELVIEW_MATRIX, matrix);
		glLoadMatrixf(matrix);

		// Draw scene into framebuffer.
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, right_fbo);
		glViewport(0, 0, eye_w, eye_h);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw_scene(list);
		if (crosshair_overlay) {
			glClear(GL_DEPTH_BUFFER_BIT);
			glLineWidth(5.0);
			glColor4f(1.0, 0.5, 0.0, 1.0);
			draw_crosshairs(0.1, 2*right_lens_center[0]/viewport_scale[0] - 1.0f, 2*right_lens_center[1]/viewport_scale[1] - 1.0f);
		}

		// Clean up common draw state
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);


		// Setup ortho state.
		glUseProgram(shader);
		glUniform1f(glGetUniformLocation(shader, "WarpScale"), warp_scale*warp_adj);
		glUniform4fv(glGetUniformLocation(shader, "HmdWarpParam"), 1, distortion_coeffs);
		glViewport(0, 0, hmd_w, hmd_h);
		glEnable(GL_TEXTURE_2D);
		glColor4d(1, 1, 1, 1);

		// Setup simple render state
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// Draw left eye
		glUniform2fv(glGetUniformLocation(shader, "LensCenter"), 1, left_lens_center);
		glBindTexture(GL_TEXTURE_2D, left_color_tex);
		glBegin(GL_QUADS);
		glTexCoord2d( 0,  0);
		glVertex3d(  -1, -1, 0);
		glTexCoord2d( 1,  0);
		glVertex3d(   0, -1, 0);
		glTexCoord2d( 1,  1);
		glVertex3d(   0,  1, 0);
		glTexCoord2d( 0,  1);
		glVertex3d(  -1,  1, 0);
		glEnd();

		// Draw right eye
		glUniform2fv(glGetUniformLocation(shader, "LensCenter"), 1, right_lens_center);
		glBindTexture(GL_TEXTURE_2D, right_color_tex);
		glBegin(GL_QUADS);
		glTexCoord2d( 0,  0);
		glVertex3d(   0, -1, 0);
		glTexCoord2d( 1,  0);
		glVertex3d(   1, -1, 0);
		glTexCoord2d( 1,  1);
		glVertex3d(   1,  1, 0);
		glTexCoord2d( 0,  1);
		glVertex3d(   0,  1, 0);
		glEnd();

		// Clean up state.
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
		glUseProgram(0);

		// Da swap-dawup!
		SDL_GL_SwapWindow(gl.window);
		SDL_Delay(10);
	}

	ohmd_ctx_destroy(ctx);

	return 0;
}
