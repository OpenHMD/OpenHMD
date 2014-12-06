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

#define TEST_WIDTH 1280
#define TEST_HEIGHT 800

#define EYE_WIDTH (TEST_WIDTH / 2 * 2)
#define EYE_HEIGHT (TEST_HEIGHT * 2)

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

void draw_scene(GLuint list)
{
	// draw cubes
	glCallList(list);
}

int main(int argc, char** argv)
{
	ohmd_context* ctx = ohmd_ctx_create();
	int num_devices = ohmd_ctx_probe(ctx);
	if(num_devices < 0){
		printf("failed to probe devices: %s\n", ohmd_ctx_get_error(ctx));
		return 1;
	}

	ohmd_device* hmd = ohmd_list_open_device(ctx, 0);
	
	if(!hmd){
		printf("failed to open device: %s\n", ohmd_ctx_get_error(ctx));
		return 1;
	}

	gl_ctx gl;
	init_gl(&gl, TEST_WIDTH, TEST_HEIGHT);

	SDL_ShowCursor(SDL_DISABLE);

	char* vertex = read_file("shaders/test1.vert.glsl");
	char* fragment = read_file("shaders/test1.frag.glsl");

	GLuint shader = compile_shader(vertex, fragment);
	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "warpTexture"), 0);
	glUseProgram(0);

	GLuint list = gen_cubes();

	GLuint left_color_tex = 0, left_depth_tex = 0, left_fbo = 0;
	create_fbo(EYE_WIDTH, EYE_HEIGHT, &left_fbo, &left_color_tex, &left_depth_tex);

	GLuint right_color_tex = 0, right_depth_tex = 0, right_fbo = 0;
	create_fbo(EYE_WIDTH, EYE_HEIGHT, &right_fbo, &right_color_tex, &right_depth_tex);


	bool done = false;
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
					SDL_WM_ToggleFullScreen(gl.screen);
					break;
				case SDLK_F2:
					{
						// reset rotation and position
						float zero[] = {0, 0, 0, 1};
						ohmd_device_setf(hmd, OHMD_ROTATION_QUAT, zero);
						ohmd_device_setf(hmd, OHMD_POSITION_VECTOR, zero);
					}
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
		glViewport(0, 0, EYE_WIDTH, EYE_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw_scene(list);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);


		// set hmd rotation, for right eye.
		glMatrixMode(GL_PROJECTION);
		ohmd_device_getf(hmd, OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX, matrix);
		glLoadMatrixf(matrix);

		glMatrixMode(GL_MODELVIEW);
		ohmd_device_getf(hmd, OHMD_RIGHT_EYE_GL_MODELVIEW_MATRIX, matrix);
		glLoadMatrixf(matrix);

		// Draw scene into framebuffer.
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, right_fbo);
		glViewport(0, 0, EYE_WIDTH, EYE_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw_scene(list);

		// Clean up common draw state
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);


		// Setup ortho state.
		glUseProgram(shader);
		glViewport(0, 0, TEST_WIDTH, TEST_HEIGHT);
		glEnable(GL_TEXTURE_2D);
		glColor4d(1, 1, 1, 1);

		// Setup simple render state
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// Draw left eye
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
		SDL_GL_SwapBuffers();
		SDL_Delay(10);
	}

	ohmd_ctx_destroy(ctx);

	free(vertex);
	free(fragment);
	
	return 0;
}
