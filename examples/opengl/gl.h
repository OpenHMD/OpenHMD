/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* OpenGL Test - Interface For GL Helper Functions */

#ifndef GL_H
#define GL_H

#include <SDL.h>

#ifndef __APPLE__
#include <GL/glew.h>
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
static inline void glewInit(void) {}
#endif

typedef struct {
	int w, h;
	SDL_Window* window;
	SDL_GLContext glcontext;
	int is_fullscreen;
} gl_ctx;

void ortho(gl_ctx* ctx);
void perspective(gl_ctx* ctx);
void init_gl(gl_ctx* ctx, int w, int h);
void draw_cube();
GLuint compile_shader(const char* vertex, const char* fragment);
void create_fbo(int eye_width, int eye_height, GLuint* fbo, GLuint* color_tex, GLuint* depth_tex);


#endif
