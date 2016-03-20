/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* OpenGL Test - Interface For GL Helper Functions */

#ifndef GL_H
#define GL_H

#include <SDL/SDL.h>

#include <GL/glew.h>
#include <GL/gl.h>

typedef struct {
	int w, h;
	SDL_Surface* screen;
} gl_ctx;

void ortho(gl_ctx* ctx);
void perspective(gl_ctx* ctx);
void init_gl(gl_ctx* ctx, int w, int h);
void draw_cube();
GLuint compile_shader(const char* vertex, const char* fragment);
void create_fbo(int eye_width, int eye_height, GLuint* fbo, GLuint* color_tex, GLuint* depth_tex);


#endif
