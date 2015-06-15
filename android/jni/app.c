#include <jni.h>
#include <errno.h>
#include <math.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <dlfcn.h>

#include <openhmd.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

struct engine {
	struct android_app* app;

	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	int32_t width;
	int32_t height;

	int32_t touchX;
	int32_t touchY;
};

/**
 * Initialize an EGL context for the current display.
 * TODO tidy this up, currently it's mostly Google example code
 */
int init_display(struct engine* engine) {

	// Setup OpenGL ES 2
	// http://stackoverflow.com/questions/11478957/how-do-i-create-an-opengl-es-2-context-in-a-native-activity

	const EGLint attribs[] = {
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, //important
			EGL_BLUE_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_RED_SIZE, 8,
			EGL_NONE
	};

	EGLint attribList[] =
	{
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE
	};

	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	/* Here, the application chooses the configuration it desires. In this
	 * sample, we have a very simplified selection process, where we pick
	 * the first EGLConfig that matches our criteria */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	 * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	 * As soon as we picked a EGLConfig, we can safely reconfigure the
	 * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);

	context = eglCreateContext(display, config, NULL, attribList);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return -1;
	}

	// Grab the width and height of the surface
	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	engine->display = display;
	engine->context = context;
	engine->surface = surface;
	engine->width = w;
	engine->height = h;

	// Initialize GL state.
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, w, h);

	return 0;
}

/**
 * Just the current frame in the display.
 */
void draw_frame(struct engine* engine) {
	// No display.
	if (engine->display == NULL) {
		return;
	}

	glClearColor(255,0,0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
void terminate_display(struct engine* engine) {
	if (engine->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (engine->context != EGL_NO_CONTEXT) {
			eglDestroyContext(engine->display, engine->context);
		}
		if (engine->surface != EGL_NO_SURFACE) {
			eglDestroySurface(engine->display, engine->surface);
		}
		eglTerminate(engine->display);
	}
	engine->display = EGL_NO_DISPLAY;
	engine->context = EGL_NO_CONTEXT;
	engine->surface = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
int32_t handle_input(struct android_app* app, AInputEvent* event) {
	struct engine* engine = (struct engine*)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		engine->touchX = AMotionEvent_getX(event, 0);
		engine->touchY = AMotionEvent_getY(event, 0);
		LOGI("x %d\ty %d\n",engine->touchX,engine->touchY);
		return 1;
	}
	return 0;
}

/**
 * Process the next main command.
 */
void handle_cmd(struct android_app* app, int32_t cmd) {
	struct engine* engine = (struct engine*)app->userData;
	switch (cmd) {
	case APP_CMD_SAVE_STATE:
		break;
	case APP_CMD_INIT_WINDOW:
		// The window is being shown, get it ready.
		if (engine->app->window != NULL) {
			init_display(engine);
			draw_frame(engine);
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// The window is being hidden or closed, clean it up.
		terminate_display(engine);
		break;
	case APP_CMD_LOST_FOCUS:
		draw_frame(engine);
		break;
	}
}

bool load_library(struct android_app* state, const char* name)
{
	char buffer[512];
	snprintf(buffer, sizeof(buffer), "%s/../lib/%s", state->activity->internalDataPath, name);
	LOGI("loading library from file: %s", buffer);
	void* handle = dlopen(buffer, RTLD_NOW | RTLD_GLOBAL);
	return handle != NULL;
}

void ls(const char* dir)
{
	DIR *dp;
	struct dirent *ep;     
	dp = opendir(dir);

	if (dp != NULL)
	{
		while((ep = readdir(dp))){
			LOGI("file: %s\n", ep->d_name);
		}

		closedir(dp);
	}

	else {
		LOGW("Couldn't open the directory");
	}
}

void openhmd_android_init(struct android_app* state);

/**
 * Main entry point, handles events
 */
void android_main(struct android_app* state)
{
	app_dummy();
	
	struct engine engine;

	memset(&engine, 0, sizeof(engine));
	state->userData = &engine;
	state->onAppCmd = handle_cmd;
	state->onInputEvent = handle_input;
	engine.app = state;
	
	ASensorManager* sensorManager = ASensorManager_getInstance();
	const ASensor* acc = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	ASensorEventQueue* eventQueue = ASensorManager_createEventQueue(sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);

	ohmd_context* ctx = ohmd_ctx_create();

	// Probe for devices
	int num_devices = ohmd_ctx_probe(ctx);
	if(num_devices < 0){
		LOGI("failed to probe devices: %s\n", ohmd_ctx_get_error(ctx));
		return;
	}

	printf("num devices: %d\n\n", num_devices);

	// Print device information
	for(int i = 0; i < num_devices; i++){
		LOGI("device %d\n", i);
		LOGI("  vendor:  %s\n", ohmd_list_gets(ctx, i, OHMD_VENDOR));
		LOGI("  product: %s\n", ohmd_list_gets(ctx, i, OHMD_PRODUCT));
		LOGI("  path:    %s\n\n", ohmd_list_gets(ctx, i, OHMD_PATH));
	}

	// Open default device (0)
	ohmd_device* hmd = ohmd_list_open_device(ctx, 0);
	
	if(!hmd){
		LOGI("failed to open device: %s\n", ohmd_ctx_get_error(ctx));
		return;
	}

	// Read all pending events.
	while(1){
		int ident;
		int events;
		struct android_poll_source* source;

		while((ident=ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0){
			// If a sensor has data, process it now.
			if (ident == LOOPER_ID_USER) {
				LOGI("sensor has data");

				if (acc != NULL) {
					ASensorEvent event;

					while (ASensorEventQueue_getEvents(eventQueue, &event, 1) > 0) {
						float sensor[10] = {};

						// TODO dt - time since last update
						sensor[0] = .01f;
						
						// TODO gyro vec3
						sensor[1] = 0.0f;
						sensor[2] = 0.0f;
						sensor[3] = 0.0f;

						// accelerator vec3
						sensor[4] = event.acceleration.x;
						sensor[5] = event.acceleration.y;
						sensor[6] = event.acceleration.z;
						
						// TODO magnetometer vec3
						sensor[7] = event.acceleration.x;
						sensor[8] = event.acceleration.y;
						sensor[9] = event.acceleration.z;

						ohmd_device_setf(hmd, OHMD_EXTERNAL_SENSOR_FUSION, sensor); 
						
						LOGI("RAW accelerometer: x=%f y=%f z=%f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
					}
				}
			}

			// Process this event.
			if (source != NULL) {
				source->process(state, source);
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0) {
				terminate_display(&engine);
				return;
			}
		}
		
		ohmd_ctx_update(ctx);

		float rot[4] = {};
		ohmd_device_getf(hmd, OHMD_ROTATION_QUAT, rot);
		//LOGI("rotation quat: %f, %f, %f, %f", rot[0], rot[1], rot[2], rot[3]);

		// Draw the current frame
		draw_frame(&engine);
	}
	

	ohmd_ctx_destroy(ctx);
}
