/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2020 Chritoph Haag.
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

#define MATH_3D_IMPLEMENTATION
#include "math_3d.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define degreesToRadians(angleDegrees) ((angleDegrees) * M_PI / 180.0)

#define OVERSAMPLE_SCALE 2.0

#define CUBES 18

#define GL_DEBUG 0

void GLAPIENTRY
gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                  GLsizei length, const GLchar* message, const void* userParam)
{
	fprintf(stderr, "GL DEBUG CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
	        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
	        type, severity, message);
}

float randf()
{
	return (float)rand() / (float)RAND_MAX;
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

void gen_cubes(mat4_t *model_matrices, vec3_t *colors, float *alphas, int num)
{
	int degree = 360. / CUBES;
	
	for(int i = 0; i < num; i ++) {
		model_matrices[i] = m4_identity();
		model_matrices[i] = m4_mul(model_matrices[i], m4_rotation(degreesToRadians(i * degree), vec3(0, 1, 0)));
		model_matrices[i] = m4_mul(model_matrices[i], m4_translation(vec3(0, 1.8, -5)));
		model_matrices[i] = m4_mul(model_matrices[i], m4_rotation(degreesToRadians(randf() * 360), vec3(randf(), randf(), randf())));

		colors[i] = vec3(randf(), randf(), randf());
		alphas[i] = randf();
	}
}

void draw_cubes(GLuint shader, mat4_t *model_matrices, vec3_t *colors, float *alphas, int num)
{
	int modelLoc = glGetUniformLocation(shader, "model");
	int colorLoc = glGetUniformLocation(shader, "uniformColor");
	for(int i = 0; i < num; i ++) {
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*) model_matrices[i].m);
		glUniform4f(colorLoc, colors[i].x, colors[i].y, colors[i].z, alphas[i]);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	// floor is 10x10m, 0.1m thick
	mat4_t floor = m4_identity();
	floor = m4_mul(floor, m4_scaling(vec3(10, 0.1, 10)));
	// we could move the floor to -1.8m height if the HMD tracker sits at zero
	// floor = m4_mul(floor, m4_translation(vec3(0, -1.8, 0)));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*) floor.m);
	glUniform4f(colorLoc, 0, .4f, .25f, .9f);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void draw_controllers(GLuint shader, ohmd_device *lc, ohmd_device *rc)
{
	int modelLoc = glGetUniformLocation(shader, "model");
	int colorLoc = glGetUniformLocation(shader, "uniformColor");

	mat4_t lcmodel;
	ohmd_device_getf(lc, OHMD_GL_MODEL_MATRIX, (float*) lcmodel.m);
	lcmodel = m4_mul(lcmodel, m4_scaling(vec3(0.03, 0.03, 0.1)));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*) lcmodel.m);
	glUniform4f(colorLoc, 1.0, 0.0, 0.0, 1.0);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	mat4_t rcmodel;
	ohmd_device_getf(rc, OHMD_GL_MODEL_MATRIX, (float*) rcmodel.m);
	rcmodel = m4_mul(rcmodel, m4_scaling(vec3(0.03, 0.03, 0.1)));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*) rcmodel.m);
	glUniform4f(colorLoc, 0.0, 1.0, 0.0, 1.0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

static bool parse_args(int argc, char** argv,
                       int *hmd_idx, int *lc_idx, int *rc_idx)
{
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-hmd") == 0 && i < argc - 1) {
			*hmd_idx = strtol(argv[i+1], NULL, 10);
			printf("Using HMD device %d\n", *hmd_idx);
		}
		if (strcmp(argv[i], "-lc") == 0 && i < argc - 1) {
			*lc_idx = strtol(argv[i+1], NULL, 10);
			printf("Using left controller device %d\n", *lc_idx);
		}
		if (strcmp(argv[i], "-rc") == 0 && i < argc - 1) {
			*rc_idx = strtol(argv[i+1], NULL, 10);
			printf("Using right controller device %d\n", *rc_idx);
		}
	}
	return true;
}

int main(int argc, char** argv)
{
	ohmd_context* ctx = ohmd_ctx_create();
	
	int hmd_idx = -1;
	int lc_idx = -1;
	int rc_idx = -1;
	parse_args(argc, argv, &hmd_idx, &lc_idx, &rc_idx);

	int num_devices = ohmd_ctx_probe(ctx);
	if (num_devices < 0) {
		printf("failed to probe devices: %s\n", ohmd_ctx_get_error(ctx));
		return 1;
	}

	/* For all devices that are not set by the user args, choose first match */
	for (int i = 0; i < num_devices; i++) {
		int device_class = 0;
		int device_flags = 0;
		ohmd_list_geti(ctx, i, OHMD_DEVICE_CLASS, &device_class);
		ohmd_list_geti(ctx, i, OHMD_DEVICE_FLAGS, &device_flags);
		if (hmd_idx == -1 && (device_class == OHMD_DEVICE_CLASS_HMD)) {
			hmd_idx = i;
		}
		if (lc_idx == -1 && (device_class == OHMD_DEVICE_CLASS_CONTROLLER) && (device_flags & OHMD_DEVICE_FLAGS_LEFT_CONTROLLER)) {
			lc_idx = i;
		}
		if (rc_idx == -1 && (device_class == OHMD_DEVICE_CLASS_CONTROLLER) && (device_flags & OHMD_DEVICE_FLAGS_RIGHT_CONTROLLER)) {
			rc_idx = i;
		}
	}
	
	ohmd_device_settings* settings = ohmd_device_settings_create(ctx);

	// If OHMD_IDS_AUTOMATIC_UPDATE is set to 0, ohmd_ctx_update() must be called at least 10 times per second.
	// It is enabled by default.

	int auto_update = 1;
	ohmd_device_settings_seti(settings, OHMD_IDS_AUTOMATIC_UPDATE, &auto_update);


	ohmd_device* hmd_dev = ohmd_list_open_device_s(ctx, hmd_idx, settings);
	if(!hmd_dev) {
		printf("failed to open device: %s\n", ohmd_ctx_get_error(ctx));
		return 1;
	}

	ohmd_device* lc_dev = ohmd_list_open_device_s(ctx, lc_idx, settings);
	if(!lc_dev) {
		printf("failed to open device: %s\n", ohmd_ctx_get_error(ctx));
		return 1;
	}
	
	ohmd_device* rc_dev = ohmd_list_open_device_s(ctx, rc_idx, settings);
	if(!rc_dev) {
		printf("failed to open device: %s\n", ohmd_ctx_get_error(ctx));
		return 1;
	}
	
	printf("HMD:\n");
	printf("\t%s\n", ohmd_list_gets(ctx, hmd_idx, OHMD_PRODUCT));

	printf("Left controller:\n");
	printf("\t%s\n", ohmd_list_gets(ctx, lc_idx, OHMD_PRODUCT));

	printf("Right controller:\n");
	printf("\t%s\n", ohmd_list_gets(ctx, rc_idx, OHMD_PRODUCT));

	int hmd_w, hmd_h;
		
	ohmd_device_geti(hmd_dev, OHMD_SCREEN_HORIZONTAL_RESOLUTION, &hmd_w);
	ohmd_device_geti(hmd_dev, OHMD_SCREEN_VERTICAL_RESOLUTION, &hmd_h);
	float ipd;
	ohmd_device_getf(hmd_dev, OHMD_EYE_IPD, &ipd);
	float viewport_scale[2];
	float distortion_coeffs[4];
	float aberr_scale[3];
	float sep;
	float left_lens_center[2];
	float right_lens_center[2];
	//viewport is half the screen
	ohmd_device_getf(hmd_dev, OHMD_SCREEN_HORIZONTAL_SIZE, &(viewport_scale[0]));
	viewport_scale[0] /= 2.0f;
	ohmd_device_getf(hmd_dev, OHMD_SCREEN_VERTICAL_SIZE, &(viewport_scale[1]));
	//distortion coefficients
	ohmd_device_getf(hmd_dev, OHMD_UNIVERSAL_DISTORTION_K, &(distortion_coeffs[0]));
	ohmd_device_getf(hmd_dev, OHMD_UNIVERSAL_ABERRATION_K, &(aberr_scale[0]));
	//calculate lens centers (assuming the eye separation is the distance between the lens centers)
	ohmd_device_getf(hmd_dev, OHMD_LENS_HORIZONTAL_SEPARATION, &sep);
	ohmd_device_getf(hmd_dev, OHMD_LENS_VERTICAL_POSITION, &(left_lens_center[1]));
	ohmd_device_getf(hmd_dev, OHMD_LENS_VERTICAL_POSITION, &(right_lens_center[1]));
	left_lens_center[0] = viewport_scale[0] - sep/2.0f;
	right_lens_center[0] = sep/2.0f;
	//assume calibration was for lens view to which ever edge of screen is further away from lens center
	float warp_scale = (left_lens_center[0] > right_lens_center[0]) ? left_lens_center[0] : right_lens_center[0];
	float warp_adj = 1.0f;

	ohmd_device_settings_destroy(settings);

	gl_ctx gl;
	GLuint VAOs[2];
	GLuint appshader;
	init_gl(&gl, hmd_w, hmd_h, VAOs, &appshader);

	if (GL_DEBUG) {
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(gl_debug_callback, 0);
	}
		
	int eye_w = hmd_w/2*OVERSAMPLE_SCALE;
	int eye_h = hmd_h*OVERSAMPLE_SCALE;

	GLuint textures[2];
	GLuint framebuffers[2];
	GLuint depthbuffers[2];
	for (int i = 0; i < 2; i++)
		create_fbo(eye_w, eye_h, &framebuffers[i], &textures[i], &depthbuffers[i]);

	// cubes arranged in a circle around the user, 20° between them. 360°/20° = 18 cubes
	mat4_t cube_modelmatrix[CUBES];
	vec3_t cube_colors[CUBES];
	float cube_alpha[CUBES];

	gen_cubes(cube_modelmatrix, cube_colors, cube_alpha, CUBES);

	SDL_ShowCursor(SDL_DISABLE);

	const char* vertex;
	ohmd_gets(OHMD_GLSL_330_DISTORTION_VERT_SRC, &vertex);
	const char* fragment;
	ohmd_gets(OHMD_GLSL_330_DISTORTION_FRAG_SRC, &fragment);
	GLuint distortionshader = compile_shader(vertex, fragment);

	bool done = false;
	while (!done) {
		ohmd_ctx_update(ctx);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
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
						ohmd_device_setf(hmd_dev, OHMD_ROTATION_QUAT, zero);
						ohmd_device_setf(hmd_dev, OHMD_POSITION_VECTOR, zero);
					}
					break;
				case SDLK_F3:
					{
						float mat[16];
						ohmd_device_getf(hmd_dev, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX, mat);
						printf("Projection L: ");
						print_matrix(mat);
						printf("\n");
						ohmd_device_getf(hmd_dev, OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX, mat);
						printf("Projection R: ");
						print_matrix(mat);
						printf("\n");
						ohmd_device_getf(hmd_dev, OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX, mat);
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
					ohmd_device_setf(hmd_dev, OHMD_EYE_IPD, &ipd);
					break;
				case SDLK_o:
					ipd += 0.001;
					ohmd_device_setf(hmd_dev, OHMD_EYE_IPD, &ipd);
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
						ohmd_device_getf(hmd_dev, OHMD_UNIVERSAL_DISTORTION_K, &(distortion_coeffs[0]));
					}
					break;
				default:
					break;
				}
			}
		}

		glViewport(0, 0, eye_w, eye_h);
		glScissor(0, 0, eye_w, eye_h);

		glUseProgram(appshader);
		for (int i = 0; i < 2; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[i], 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthbuffers[i], 0);

			glClearColor(0.0, 0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(VAOs[0]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_SCISSOR_TEST);

			float projectionmatrix[16];
			ohmd_device_getf(hmd_dev, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX, projectionmatrix);
			glUniformMatrix4fv(glGetUniformLocation(appshader, "proj"), 1, GL_FALSE, projectionmatrix);

			float viewmatrix[16];
			ohmd_device_getf(hmd_dev, OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX, viewmatrix);
			glUniformMatrix4fv(glGetUniformLocation(appshader, "view"), 1, GL_FALSE, viewmatrix);

			draw_cubes(appshader, cube_modelmatrix, cube_colors, cube_alpha, CUBES);
			draw_controllers(appshader, lc_dev, rc_dev);
		}

		// draw the textures to the screen, applying the distortion shader
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glUseProgram(distortionshader);
		glUniform1i(glGetUniformLocation(distortionshader, "warpTexture"), 0);
		glUniform2fv(glGetUniformLocation(distortionshader, "ViewportScale"), 1, viewport_scale);
		glUniform3fv(glGetUniformLocation(distortionshader, "aberr"), 1, aberr_scale);
		glUniform1f(glGetUniformLocation(distortionshader, "WarpScale"), warp_scale*warp_adj);
		glUniform4fv(glGetUniformLocation(distortionshader, "HmdWarpParam"), 1, distortion_coeffs);

		// The shader is set up to render starting at the middle of the viewport
		// and half its size. Move it to the bottom left and double its size.
		float mvp[16] = {
			2.0, 0.0, 0.0, 0.0,
			0.0, 2.0, 0.0, 0.0,
			0.0, 0.0, 2.0, 0.0,
			-1.0, -1.0, 0.0, 1.0
		};
		glUniformMatrix4fv(glGetUniformLocation(distortionshader, "mvp"), 1, GL_FALSE, mvp);

		for (int i = 0; i < 2; i++) {
			if (i == 0) {
				glViewport(0, 0, hmd_w / 2, hmd_h);
				glScissor(0, 0, hmd_w / 2, hmd_h);
				glClearColor(0.5, 0, 0.0, 1.0);
			} else {
				glViewport(hmd_w / 2, 0, hmd_w / 2, hmd_h);
				glScissor(hmd_w / 2, 0, hmd_w / 2, hmd_h);
				glClearColor(0, 0.5, 0.0, 1.0);
			}
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glUniform2fv(glGetUniformLocation(distortionshader, "LensCenter"), 1, i == 0 ? left_lens_center : right_lens_center);

			glBindVertexArray(VAOs[1]);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[i]);

			glDrawArrays(GL_TRIANGLES, 0, 3);
		}

		// Da swap-dawup!
		SDL_GL_SwapWindow(gl.window);
		SDL_Delay(10);
	}

	ohmd_ctx_destroy(ctx);

	return 0;
}
