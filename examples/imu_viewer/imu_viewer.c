/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.

 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz
 * Copyright (C) 2019 Collabora Ltd.
 * Author: Christoph Haag <christoph.haag@collabora.com>
 * Author: Lubosz Sarnecki <lubosz.sarnecki@collabora.com>
 *
 * SPDX-License-Identifier: BSL-1.0
 */

#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include <SDL.h>
#include <GL/glew.h>

#include <openhmd.h>

#define MATH_3D_IMPLEMENTATION
#include "math_3d.h"

typedef struct {
	uint32_t      w, h;
	SDL_Window*   window;
	SDL_GLContext gl_ctx;
	uint32_t      device_id;
	ohmd_context* ctx;
	ohmd_device*  device;
	bool          quit;
	GLuint        vaos[1];
	GLuint        shader;
} imu_viewer;

static char* vertex_src =
"#version 450 core\n"

"layout(location = 0) in vec3 pos;\n"
"layout(location = 1) in vec3 color;\n"
"layout(location = 2) uniform mat4 mvp;\n"
"layout(location = 3) out vec3 out_color;\n"

"void main() {\n"
"	out_color = color;\n"
"	gl_Position = mvp * vec4(pos.x, pos.y, pos.z, 1.0);\n"
"}\n";

static char* fragment_src =
"#version 450 core\n"
"layout(location = 0) out vec4 out_color;\n"
"layout(location = 3) in vec3 vertex_color;\n"

"void main() {\n"
"	out_color = vec4(vertex_color, 1);\n"
"}\n";

static void compile_shader_src(GLuint shader, const char* src)
{
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if(status == GL_FALSE) 	{
		GLint len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		char log[len + 1];
		glGetShaderInfoLog(shader, len, &len, log);
		printf("Error: Compile failed: %s\n", log);
	}
}

GLuint compile_shader(const char* vertex, const char* fragment)
{
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint program = glCreateProgram();

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	compile_shader_src(vertex_shader, vertex);
	compile_shader_src(fragment_shader, fragment);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if(status == GL_FALSE) {
		GLint len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		char log[len + 1];
		glGetProgramInfoLog(program, len, &len, log);
		printf("Error: Compile failed: %s\n", log);
	}
	return program;
}

bool init_openhmd(imu_viewer* self)
{
	self->ctx = ohmd_ctx_create();
	int num_devices = ohmd_ctx_probe(self->ctx);
	if(num_devices < 0) {
		printf("Error: Failed to probe devices: %s\n",
		       ohmd_ctx_get_error(self->ctx));
		return false;
	}

	if(self->device_id > num_devices - 1) {
		printf("Error: Requested device %d, but only %d are available\n",
		       self->device_id, num_devices);
		return false;
	}

	ohmd_device_settings* settings = ohmd_device_settings_create(self->ctx);
	int auto_update = 1;
	ohmd_device_settings_seti(settings, OHMD_IDS_AUTOMATIC_UPDATE, &auto_update);

	self->device = ohmd_list_open_device_s(self->ctx, self->device_id, settings);
	if(!self->device){
		printf("Error: Failed to open device: %s\n", ohmd_ctx_get_error(self->ctx));
		return false;
	}

	printf("Device:\n");
	printf("\t%s\n", ohmd_list_gets(self->ctx, self->device_id, OHMD_PRODUCT));

	ohmd_device_settings_destroy(settings);

	return true;
}

bool init_sdl(imu_viewer* self)
{
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("Error: SDL_Init failed.\n");
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	self->window = SDL_CreateWindow("OpenHMD IMU Viewer",
	                                SDL_WINDOWPOS_UNDEFINED,
	                                SDL_WINDOWPOS_UNDEFINED,
	                                self->w, self->h, SDL_WINDOW_OPENGL);
	if(self->window == NULL) {
		printf("Error: SDL_CreateWindow failed.\n");
		return false;
	}

	self->gl_ctx = SDL_GL_CreateContext(self->window);
	if(self->gl_ctx == NULL){
		printf("Error: SDL_GL_CreateContext failed.\n");
		return false;
	}

	SDL_GL_SetSwapInterval(1);
	SDL_ShowCursor(SDL_DISABLE);

	return true;
}

bool init_gl(imu_viewer* self)
{
	if (glewInit() != GLEW_OK)
	{
		printf("Error: glewInit failed.\n");
		return false;
	}

	printf("OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
	printf("OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
	printf("OpenGL Version: %s\n", glGetString(GL_VERSION));

	self->shader = compile_shader (vertex_src, fragment_src);
	glUseProgram(self->shader);

	float vertices[] = {
		0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	};

	GLuint vbos[1];
	glGenBuffers(1, vbos);

	glGenVertexArrays(1, &self->vaos[0]);

	glBindVertexArray(self->vaos[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(0);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glLineWidth(10.0f);

	return true;
}

void
poll_events(imu_viewer *self)
{
	ohmd_ctx_update(self->ctx);

	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		if(event.type == SDL_KEYDOWN) {
			switch(event.key.keysym.sym) {
				case SDLK_ESCAPE:
					self->quit = true;
					break;
				case SDLK_r:
					{
						// reset rotation and position
						float zero[] = {0, 0, 0, 1};
						ohmd_device_setf(self->device, OHMD_ROTATION_QUAT, zero);
						ohmd_device_setf(self->device, OHMD_POSITION_VECTOR, zero);
					}
					break;
				default:
					break;
				}
			}
		}
}

void
render_frame(imu_viewer *self)
{
	glViewport(0, 0, self->w, self->h);

	glUseProgram(self->shader);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(self->vaos[0]);

	vec3_t from = { .x = 2, .y = 2, .z = 2};
	vec3_t to = { .x = 0, .y = 0, .z = 0};
	vec3_t up = { .x = 0, .y = 1, .z = 0};

	mat4_t view = m4_look_at(from, to, up);
	mat4_t projection = m4_perspective(45.0, (float) self->w / (float) self->h,
	                                   0.1, 1000.0);

	mat4_t model;
	ohmd_device_getf(self->device, OHMD_GL_MODEL_MATRIX, (float*) model.m);
	mat4_t mvp = m4_mul(m4_mul(projection, view), model);

	int mvp_loc = glGetUniformLocation(self->shader, "mvp");
	glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, (float*) mvp.m);
	glDrawArrays(GL_LINES, 0, 6);

	// Da swap-dawup!
	SDL_GL_SwapWindow(self->window);
}

int main(int argc, char** argv)
{
	imu_viewer self = {
		.device_id = 0,
		.w = 800,
		.h = 800,
		.quit = false
	};

	/* choose device */
	if(argc > 1) {
		self.device_id = atoi(argv[1]);
	}

	if (!init_openhmd(&self))
		return -1;

	if (!init_sdl(&self))
		return -1;

	if (!init_gl (&self))
		return -1;

	while(!self.quit) {
		poll_events(&self);
		render_frame (&self);
	}

	ohmd_ctx_destroy(self.ctx);

	return 0;
}
