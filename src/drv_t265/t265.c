/* Intel Realsense T265 driver */

#include <librealsense2/rs.h>
#include <librealsense2/h/rs_pipeline.h>
#include <librealsense2/h/rs_option.h>
#include <librealsense2/h/rs_frame.h>
#include <stdio.h>
#include <string.h>
#include "../openhmdi.h"

struct RS_State
{
  rs2_context* ctx;
  rs2_pipeline* pipe;
  rs2_pipeline_profile* profile;
  rs2_config* config;
};

struct Pose_Data
{
  float rotation_x;
  float rotation_y;
  float rotation_z;
  float rotation_w;
  float translation_x;
  float translation_y;
  float translation_z;
};

typedef struct {
	ohmd_device base;
  struct RS_State handle;
} t265_priv;

int check_error(rs2_error* e)
{
  if (e)
  {
    fprintf(stderr, "rs_error was raised when calling %s(%s): \n",
    rs2_get_failed_function(e), rs2_get_failed_args(e));
    fprintf(stderr, "%s\n", rs2_get_error_message(e));
    return 1;
  }
  return 0;
}

int create_context(struct RS_State* rs_state)
{
  rs2_error* e = NULL;

  //fprintf(stderr, "creating context\n");

  rs_state->ctx = rs2_create_context(RS2_API_VERSION, &e);
  if (check_error(e) != 0)
  {
    rs_state->ctx = NULL;
    fprintf(stderr, "Failed creating rs context\n");
    return 1;
  }

  //fprintf(stderr, "context created\n");

  return 0;
}

int close_t265(struct RS_State* s)
{
  if (s == NULL)
  {
    fprintf(stderr, "Cannot clear state: given pointer is null\n");
    return 1;
  }

  if (s->config)
  {
    rs2_delete_config(s->config);
  }

  if (s->profile)
  {
    rs2_delete_pipeline_profile(s->profile);
  }

  if (s->pipe)
  {
    rs2_pipeline_stop(s->pipe, NULL);
    rs2_delete_pipeline(s->pipe);
  }

  if (s->ctx)
  {
    rs2_delete_context(s->ctx);
  }

  return 0;
}

int create_streams(struct RS_State* s)
{
  if (s == NULL)
  {
    fprintf(stderr, "Cannot init streaming: given pointer is null\n");
    return 1;
  }

  rs2_error* e = NULL;
  s->pipe = rs2_create_pipeline(s->ctx, &e);
  if (check_error(e) != 0)
  {
    s->pipe = NULL;
    return 1;
  }

  s->config = rs2_create_config(&e);
  check_error(e);


  rs2_config_enable_stream(s->config, RS2_STREAM_POSE, 0, 0, 0, RS2_FORMAT_6DOF, 200, &e);
  if (check_error(e) != 0)
  {
    fprintf(stderr, "Failed initting pose streaming\n");
    return 1;
  }
  //fprintf(stderr, "Pose stream created\n");

  return 0;
}

int start_stream(struct RS_State* s)
{
  rs2_error* e = NULL;

  if (s == NULL)
  {
    fprintf(stderr, "Cannot star t stream: given pointer is null\n");
    return 1;
  }

  s->profile = rs2_pipeline_start_with_config(s->pipe, s->config, &e);
  if (check_error(e) != 0) {
    fprintf(stderr, "Failed starting pipeline\n");
    return 1;
  }
  //fprintf(stderr, "pipeline started\n");

  return 0;
}

int start_sensor(struct RS_State* rs_state)
{
  if (rs_state->ctx == NULL)
  {
    if (create_context(rs_state) != 0)
    {
      fprintf(stderr, "Failed creating context when starting sensor\n");
      return 1;
    }
  }

  if (create_streams(rs_state) != 0)
  {
    fprintf(stderr, "Failed initting streams\n");
    return 1;
  }

  if (start_stream(rs_state) != 0)
  {
    fprintf(stderr, "Failed starting streams\n");
    return 1;
  }

  return 0;
}

int update(struct RS_State* rs_state, struct Pose_Data* _data)
{
  rs2_frame* frames;
  rs2_error* e = NULL;

  frames = rs2_pipeline_wait_for_frames(rs_state->pipe, RS2_DEFAULT_TIMEOUT, &e);
  if (check_error(e) != 0) {
    fprintf(stderr, "Failed waiting for frames\n");
    return 1;
  }

  int num_of_frames = rs2_embedded_frames_count(frames, &e);
  check_error(e);

  int i;
  for (i = 0; i < num_of_frames; i++)
  {
    rs2_frame* frame = rs2_extract_frame(frames, i, &e);
    check_error(e);

    if (rs2_is_frame_extendable_to(frame, RS2_EXTENSION_POSE_FRAME, &e) == 1)
    {
      if (check_error(e) != 0)
      {
        fprintf(stderr, "Failed waiting for frame after frame queue\n");
        rs2_release_frame(frame);
        rs2_release_frame(frames);
        return 1;
      }

      struct rs2_pose camera_pose;
      rs2_pose_frame_get_pose_data(frame, &camera_pose, &e);
      check_error(e);

      _data->rotation_x = camera_pose.rotation.x;
      _data->rotation_y = camera_pose.rotation.y;
      _data->rotation_z = camera_pose.rotation.z;
      _data->rotation_w = camera_pose.rotation.w;

      _data->translation_x = camera_pose.translation.x;
      _data->translation_y = camera_pose.translation.y;
      _data->translation_z = camera_pose.translation.z;

      /*
      printf("Rotation x %.3f\n", camera_pose->rotation.x);
      printf("Rotation y %.3f\n", camera_pose->rotation.y);
      printf("Rotation z %.3f\n", camera_pose->rotation.z);
      printf("Rotation w %.3f\n", camera_pose->rotation.w);
      */
      }
      rs2_release_frame(frame);
    }
    rs2_release_frame(frames);
    return 0;
}

static void update_device(ohmd_device* device)
{
  t265_priv* priv = (t265_priv*)device;

  struct Pose_Data pose_data;
  update(&priv->handle, &pose_data);
  priv->base.rotation.x = pose_data.rotation_x;
  priv->base.rotation.y = pose_data.rotation_y;
  priv->base.rotation.z = pose_data.rotation_z;
  priv->base.rotation.w = pose_data.rotation_w;

  priv->base.position.x = pose_data.translation_x;
  priv->base.position.y = pose_data.translation_y;
  priv->base.position.z = pose_data.translation_z;

  /*
  printf("Rotation x %.3f\n", priv->base.rotation.x);
  printf("Rotation y %.3f\n", priv->base.rotation.y);
  printf("Rotation z %.3f\n", priv->base.rotation.z);
  printf("Rotation w %.3f\n", priv->base.rotation.w);
  printf("Rotation quaternions\n");

  printf("Position x %.3f\n", priv->base.position.x);
  printf("Position y %.3f\n", priv->base.position.y);
  printf("Position z %.3f\n", priv->base.position.z);
  printf("Position meters\n");
  */
}

static int getf(ohmd_device* device, ohmd_float_value type, float* out)
{
	t265_priv* priv = (t265_priv*)device;

	switch(type){
    // HMD
    case OHMD_ROTATION_QUAT:
    *(quatf*)out = priv->base.rotation;
    break;

    case OHMD_POSITION_VECTOR:
    *(vec3f*)out = priv->base.position;
    break;

    case OHMD_DISTORTION_K:
    // TODO this should be set to the equivalent of no distortion
    memset(out, 0, sizeof(float) * 6);
    break;

    case OHMD_CONTROLS_STATE:
    out[0] = .1f;
    out[1] = 1.0f;
    break;

    default:
    ohmd_set_error(priv->base.ctx, "invalid type given to getf (%ud)", type);
    return OHMD_S_INVALID_PARAMETER;
    break;
  }
	return OHMD_S_OK;
}

static void close_device(ohmd_device* device)
{
  t265_priv* priv = (t265_priv*)device;
  close_t265(&priv->handle);
	LOGD("Closing t265 device");
	free(device);
}

static ohmd_device* open_device(ohmd_driver* driver, ohmd_device_desc* desc)
{
	t265_priv* priv = ohmd_alloc(driver->ctx, sizeof(t265_priv));
	if(!priv)
		return NULL;

  struct RS_State rs_state;
  memset(&rs_state, 0, sizeof(rs_state));
  priv->handle = rs_state;
  start_sensor(&priv->handle);

	// Set default device properties
	ohmd_set_default_device_properties(&priv->base.properties);

	// Set device properties (imitates the rift values)
	priv->base.properties.hsize = 0.149760f;
	priv->base.properties.vsize = 0.093600f;
	priv->base.properties.hres = 1920;
	priv->base.properties.vres = 1080;
	priv->base.properties.lens_sep = 0.063500f;
	priv->base.properties.lens_vpos = 0.046800f;
	priv->base.properties.fov = DEG_TO_RAD(125.5144f);
	priv->base.properties.ratio = (1280.0f / 800.0f) / 2.0f;

	// calculate projection eye projection matrices from the device properties
	ohmd_calc_default_proj_matrices(&priv->base.properties);

	// set up device callbacks
	priv->base.update = update_device;
	priv->base.close = close_device;
	priv->base.getf = getf;

	return (ohmd_device*)priv;
}

static void get_device_list(ohmd_driver* driver, ohmd_device_list* list)
{
	int id = 0;
	ohmd_device_desc* desc;

  rs2_error* e = NULL;
  rs2_context* ctx = rs2_create_context(RS2_API_VERSION, &e);
  check_error(e);

  rs2_device_list* device_list = rs2_query_devices(ctx, &e);
  check_error(e);
  //printf("device list created\n");

  int dev_count = rs2_get_device_count(device_list, &e);
  check_error(e);
  //printf("There are %d connected RealSense devices.\n", dev_count);

  while (id < dev_count)
  {
    printf("id: %d\n", id);
		desc = &list->devices[list->num_devices++];

		strcpy(desc->driver, "Intel Realsense T265 Driver");
		strcpy(desc->vendor, "Intel");
		strcpy(desc->product, "Intel Realsense T265 Device");

		strcpy(desc->path, "None");

		desc->driver_ptr = driver;

    //OHMD_DEVICE_FLAGS_POSITIONAL_TRACKING |
		desc->device_flags = OHMD_DEVICE_FLAGS_ROTATIONAL_TRACKING;
		desc->device_class = OHMD_DEVICE_CLASS_HMD;

		desc->id = id++;
  }
  rs2_delete_device_list(device_list);
  rs2_delete_context(ctx);
}

static void destroy_driver(ohmd_driver* drv)
{
	LOGD("shutting down driver");
	free(drv);
}

ohmd_driver* ohmd_create_t265_drv(ohmd_context* ctx)
{
	ohmd_driver* drv = ohmd_alloc(ctx, sizeof(ohmd_driver));
	if(!drv)
		return NULL;

	drv->get_device_list = get_device_list;
	drv->open_device = open_device;
	drv->destroy = destroy_driver;

	return drv;
}
