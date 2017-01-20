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

#include <epoxy/gl.h>
#include <epoxy/glx.h>
#include <epoxy/egl.h>

// Check under windows - doesn't exist in fedora install, so ifdef appropriately
//#include <epoxy/wgl.h>

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
