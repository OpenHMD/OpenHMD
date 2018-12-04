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

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#ifndef __APPLE__
#include <GL/glew.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>

static inline void glewInit(void) {}
#endif

typedef struct {
	int w, h;
	SDL_Window* window;
	SDL_GLContext glcontext;
	int is_fullscreen;
} gl_ctx;

void init_gl(gl_ctx* ctx, int w, int h, GLuint *VAOs, GLuint *appshader);
GLuint compile_shader(const char* vertex, const char* fragment);
void create_fbo(int eye_width, int eye_height, GLuint* fbo, GLuint* color_tex, GLuint* depth_tex);

#endif
