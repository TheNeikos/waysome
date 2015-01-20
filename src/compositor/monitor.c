/*
 * waysome - wayland based window manager
 *
 * Copyright in alphabetical order:
 *
 * Copyright (C) 2014-2015 Julian Ganz
 * Copyright (C) 2014-2015 Manuel Messner
 * Copyright (C) 2014-2015 Marcel Müller
 * Copyright (C) 2014-2015 Matthias Beyer
 * Copyright (C) 2014-2015 Nadja Sommerfeld
 *
 * This file is part of waysome.
 *
 * waysome is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * waysome is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with waysome. If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <wayland-server.h>
#include <wayland-server-protocol.h>
#include <xf86drm.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#define EGL_EGLEXT_PROTOTYPES 1
#include <EGL/eglext.h>
#include <EGL/eglmesaext.h>


#include "compositor/buffer/image.h"
#include "compositor/framebuffer_device.h"
#include "compositor/internal_context.h"
#include "compositor/monitor.h"
#include "compositor/wayland/abstract_shell_surface.h"
#include "compositor/wayland/surface.h"
#include "logger/module.h"
#include "objects/object.h"
#include "util/wayland.h"

static struct ws_logger_context log_ctx = { .prefix = "[Compositor/Monitor] " };

/*
 *
 *  Forward declarations
 *
 */

static bool
monitor_deinit(
    struct ws_object* self
);

static size_t
monitor_hash(
    struct ws_object* self
);

static int
monitor_cmp(
    struct ws_object const* obj1,
    struct ws_object const* obj2
);

static void
bind_output(
    struct wl_client* client,
    void* data,
    uint32_t version,
    uint32_t id
);

static int
publish_modes(
    void* _data,
    void const* _mode
);

static int
redraw_surfaces(
    void* dummy,
    void const* surf_
);

static void
monitor_event(
    EV_P_
    ev_io* w,
    int revents
);

static void
handle_page_flip(
    int fd,
    unsigned int sequence,
    unsigned int tv_sec,
    unsigned int tv_usec,
    void* monitor
);

/*
 *
 * type variable
 *
 */

ws_object_type_id WS_OBJECT_TYPE_ID_MONITOR = {
    .supertype = &WS_OBJECT_TYPE_ID_OBJECT,
    .typestr = "ws_monitor",
    .deinit_callback = monitor_deinit,
    .hash_callback = monitor_hash,
    .cmp_callback = monitor_cmp,

    .attribute_table = NULL,
    .function_table = NULL,
};

/*
 *
 * Interface Implementation
 *
 */

struct ws_monitor*
ws_monitor_new(
    void
) {
    struct ws_monitor* tmp = calloc(1, sizeof(*tmp));
    ws_object_init(&tmp->obj);
    tmp->obj.id = &WS_OBJECT_TYPE_ID_MONITOR;
    tmp->obj.settings |= WS_OBJECT_HEAPALLOCED;

    // initialize members
    if (ws_set_init(&tmp->surfaces) < 0) {
        goto cleanup_alloc;
    }

    if (ws_set_init(&tmp->modes) < 0) {
        goto cleanup_alloc;
    }


    return tmp;

cleanup_alloc:
    free(tmp);
    return tmp;
}

struct ws_set*
ws_monitor_surfaces(
    struct ws_monitor* self
) {
    return &self->surfaces;
}

void
ws_monitor_publish(
    struct ws_monitor* self
) {
    struct wl_display* display = ws_wayland_acquire_display();
    if (!display) {
        return;
    }

    self->global = wl_global_create(display, &wl_output_interface, 2,
                    self, &bind_output);

    ws_wayland_release_display();

}

void
ws_monitor_populate_fb(
    struct ws_monitor* self
) {

    if (!self->connected) {
        ws_log(&log_ctx, LOG_DEBUG,
                "Did not create FB for self %d.", self->crtc);
        return;
    }

    if (!self->current_mode) {
        ws_log(&log_ctx, LOG_ERR,
                "No mode set, can't create Framebuffer");
        return;
    }

    self->gbm_surf = ws_gbm_surface_new(
            self->fb_dev, self,
            self->current_mode->mode.hdisplay,
            self->current_mode->mode.vdisplay
    );
    if (!self->gbm_surf) {
        ws_log(&log_ctx, LOG_CRIT, "Could not create GBM Surface");
    }

    struct ev_loop* loop = EV_DEFAULT;
    ev_io_init(&self->event_watcher, monitor_event, self->fb_dev->fd, EV_READ);
    ev_io_start(loop, &self->event_watcher);

    self->saved_crtc = drmModeGetCrtc(ws_comp_ctx.fb->fd, self->crtc);

    struct ws_framebuffer_device* fb_dev = ws_comp_ctx.fb;

    EGLDisplay disp = ws_framebuffer_device_get_egl_display(fb_dev);

    struct wl_display* wl_disp = ws_wayland_acquire_display();

    int ret = eglBindWaylandDisplayWL(disp, wl_disp);

    ws_wayland_release_display();

    if (eglGetError() != EGL_SUCCESS || !ret) {
        ws_log(&log_ctx, LOG_CRIT, "Could not bind wl display to egl");
        return;
    }

    struct gbm_surface* surf = self->gbm_surf->surf;

    self->egl_surf = eglCreatePlatformWindowSurfaceEXT(disp,
                                                       fb_dev->egl_conf,
                                                       surf,
                                                       NULL);

    int err;
    if ((err = eglGetError()) != EGL_SUCCESS || self->egl_surf == NULL) {
        ws_log(&log_ctx, LOG_ERR, "Could not create window surface %x", err);
        return;
    }

    ret = eglMakeCurrent(disp, self->egl_surf,
                             self->egl_surf, fb_dev->egl_ctx);

    if (!ret) {
        err = eglGetError();
        ws_log(&log_ctx, LOG_ERR, "Could not make surface current %x",
               err);
        return;
    }


    // glViewport(0, 0, self->current_mode->mode.hdisplay,
    //                  self->current_mode->mode.vdisplay);

    // Shader sources
    const GLchar* vertexSource =
        "#version 100\n"
        "attribute vec2 position;"
        "attribute vec2 UV;"
        "uniform float size_x;"
        "uniform float size_y;"
        "varying vec2 uv;"
        "void main() {"
        "   float left = 0.0;"
        "   float right = size_y;"
        "   float lower = size_x;"
        "   float upper = 0.0;"
        "   float far = 1.0;"
        "   float near = -1.0;"
        "   mat4 proj = mat4( 2.0 / (right - left), 0.0, 0.0, -(right + left)/(right - left),"
        "                     0.0, 2.0 / (upper - lower), 0.0,  -(upper + lower)/(upper - lower),"
        "                     0.0, 0.0,         -2.0 / (far - near), -(far + near)/(far - near),"
        "                     0.0, 0.0,          0.0,  1.0);"
        "   gl_Position = vec4(position, 1.0, 1.0) * proj;"
        "   uv = UV;"
        "}";
    const GLchar* fragmentSource =
        "#version 100\n"
        "precision mediump float;"
        "varying vec2 uv;"
        "uniform sampler2D tex;"
        "void main() {"
        "   gl_FragColor = vec4(texture2D(tex, uv).rgb, 1);"
        "}";

    // Create and compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    char info[500];
    int size;
    glGetShaderInfoLog(vertexShader, 500, &size, info);

    if (size > 0) {
        ws_log(&log_ctx, LOG_ERR, "Shader %u: %s", vertexShader, info);
    }

    // Create and compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderInfoLog(fragmentShader, 500, &size, info);

    if (size > 0) {
        ws_log(&log_ctx, LOG_ERR, "Shader %u: %s", fragmentShader, info);
    }

    // Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindAttribLocation(shaderProgram, 0, "position");
    glBindAttribLocation(shaderProgram, 1, "UV");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    glGetProgramInfoLog(shaderProgram, 500, &size, info);

    if (size > 0) {
        ws_log(&log_ctx, LOG_ERR, "Program error %u: %s", shaderProgram, info);
    }

    ws_log(&log_ctx, LOG_DEBUG, "Using shader program %d", shaderProgram);

    // Set the blend function for alpha blending
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    // Enable alpha blending
    glEnable(GL_BLEND);
    //!< @todo, make this settable through a transaction
    glClearColor(0.2f, 0.2f, 0.3f, 1.0f);

    // Tell EGL how big the display is!
    glViewport(0, 0,
            self->current_mode->mode.hdisplay,
            self->current_mode->mode.vdisplay);


    //!< Kick off the draw loop
    ws_monitor_redraw(self);
    return;
}

void
ws_monitor_set_mode_with_id(
    struct ws_monitor* self,
    int id
) {
    struct ws_monitor_mode mode;
    memset(&mode, 0, sizeof(mode));
    mode.obj.id = &WS_OBJECT_TYPE_ID_MONITOR_MODE;
    mode.id = id;
    self->current_mode =
        (struct ws_monitor_mode*) ws_set_get(
                &self->modes,
                (struct ws_object*)&mode);

    if (!self->current_mode) {
        return;
    }

    if (!self->resource) {
        ws_log(&log_ctx, LOG_DEBUG, "Did not publish mode.");
        return;
    }
    ws_log(&log_ctx, LOG_DEBUG, "Published a mode.");

    // Let's tell wayland that this is the current mode!
    wl_output_send_mode(self->resource, WL_OUTPUT_MODE_CURRENT,
            self->current_mode->mode.hdisplay,
            self->current_mode->mode.vdisplay,
            // The buffer and wayland differ on which unit to use
            self->current_mode->mode.vrefresh * 1000);
}

struct ws_monitor_mode*
ws_monitor_copy_mode(
    struct ws_monitor* self,
    struct _drmModeModeInfo const* src
) {
    struct ws_monitor_mode* mode = ws_monitor_mode_new();
    memcpy(&mode->mode, src, sizeof(*src));
    mode->id = self->mode_count++;
    ws_set_insert(&self->modes, &mode->obj);
    return mode;
}

struct ws_monitor_mode*
ws_monitor_add_mode(
    struct ws_monitor* self,
    int width,
    int height
) {
    //!< We exit rather than letting the program finish as it will hang the
    // graphic card
    ws_log(&log_ctx, LOG_CRIT, "Looks like we're in a doozy! This is undefined"
            " behaviour and shouldn't be called. 'ws_monitor_add_mode'");
    exit(1);
    struct ws_monitor_mode* mode = ws_monitor_mode_new();
    if (!mode) {
        ws_log(&log_ctx, LOG_ERR, "Could not create mode.");
        return NULL;
    }
    mode->id = self->mode_count++;
    mode->mode.hdisplay = width;
    mode->mode.vdisplay = height;
    mode->mode.clock = 155;
    //!< @todo: Add more information like vsync etc...
    ws_set_insert(&self->modes, &mode->obj);
    return mode;
}

int
ws_monitor_redraw(
    void const* self_ //!< The monitor object
) {
    struct ws_monitor* self = (struct ws_monitor*) self_;

    if (!self->gbm_surf) {
        return 0;
    }

    static int test = 0;
    if (!test) {
        eglSwapBuffers(ws_comp_ctx.fb->egl_disp, self->egl_surf);
        ws_gbm_surface_lock(self->gbm_surf, self);

        short cur_fb = self->gbm_surf->cur_fb;
        drmModeSetCrtc(ws_comp_ctx.fb->fd, self->crtc,
                self->gbm_surf->fb[cur_fb].handle, 0, 0,
                &self->conn, 1, &self->current_mode->mode);

        ws_gbm_surface_release(self->gbm_surf);
        test = 1;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    GLint shaderProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgram);

    glUniform1f(glGetUniformLocation(shaderProgram, "size_y"),
                 self->current_mode->mode.hdisplay);
    glUniform1f(glGetUniformLocation(shaderProgram, "size_x"),
                 self->current_mode->mode.vdisplay);

    glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        ws_log(&log_ctx, LOG_CRIT, "Framebuffer is not complete!");
        return 1;
    }

    ws_set_select(&self->surfaces, NULL, NULL, redraw_surfaces, NULL);

    eglSwapBuffers(ws_comp_ctx.fb->egl_disp, self->egl_surf);

    ws_gbm_surface_lock(self->gbm_surf, self);

    ws_gbm_surface_flip(self->gbm_surf, self);

    return 0;
}

/*
 *
 * Internal implementation
 *
 */

static void
bind_output(
    struct wl_client* client,
    void* data,
    uint32_t version,
    uint32_t id
) {

    struct ws_monitor* monitor = data;

    if (version >= 2) {
        // We only support from 2 and ongoing, so let's say that!
        version = 2;
    }

    monitor->resource = wl_resource_create(client, &wl_output_interface,
            version, id);

    if (!monitor->resource) {
        ws_log(&log_ctx, LOG_ERR, "Wayland couldn't create object");
        return;
    }

    // We don't set an implementation, instead we just set the data
    wl_resource_set_implementation(monitor->resource, NULL, data, NULL);

    // The origin of this monitor, 0,0 in this case
    wl_output_send_geometry(monitor->resource, 0, 0, monitor->phys_width,
            // 0 is the subpixel type, and the two strings are monitor infos
            monitor->phys_height, 0, "unknown", "unknown",
            WL_OUTPUT_TRANSFORM_NORMAL);
    // We publish all the modes we have right now through wayland
    ws_set_select(&monitor->modes, NULL, NULL, publish_modes,
                  monitor->resource);

    // We tell wayland that this output is done!
    wl_output_send_done(monitor->resource);
}


static int
publish_modes(
    void* _data,
    void const* _mode
) {
    struct ws_monitor_mode* mode = (struct ws_monitor_mode*) _mode;
    // The 0 here means that this isn't a preferred nor the current display
    wl_output_send_mode((struct wl_resource*) _data, 0,
            mode->mode.hdisplay, mode->mode.vdisplay,
            // The buffer and wayland differ on which unit to use
            mode->mode.vrefresh * 1000);
    return 0;
}

static bool
monitor_deinit(
    struct ws_object* obj
) {
    struct ws_monitor* self = (struct ws_monitor*) obj;
    if (self->connected) {
        drmModeSetCrtc(self->fb_dev->fd,
                self->saved_crtc->crtc_id,
                self->saved_crtc->buffer_id,
                self->saved_crtc->x,
                self->saved_crtc->y,
                &self->conn,
                1,
                &self->saved_crtc->mode);
    }

    if (self->gbm_surf) {
        ws_object_unref(&self->gbm_surf->obj);
    }

    ws_object_deinit((struct ws_object*) &self->surfaces);
    return true;
}

static size_t
monitor_hash(
    struct ws_object* obj
) {
    struct ws_monitor* self = (struct ws_monitor*) obj;
    return SIZE_MAX / (self->crtc * self->fb_dev->fd + 1);
}

static int
monitor_cmp(
    struct ws_object const* obj1,
    struct ws_object const* obj2
) {
    struct ws_monitor* mon1 = (struct ws_monitor*) obj1;
    struct ws_monitor* mon2 = (struct ws_monitor*) obj2;

    if (mon1->id != mon2->id) {
        return (mon1->id > mon2->id) - (mon1->id < mon2->id);
    }

    if (mon1->fb_dev->fd != mon2->fb_dev->fd) {
        return (mon1->fb_dev->fd > mon2->fb_dev->fd) -
            (mon1->fb_dev->fd < mon2->fb_dev->fd);
    }
    return 0;
}

static int
redraw_surfaces(
    void* dummy,
    void const* surf_
) {
    struct ws_abstract_shell_surface* surf;
    surf = (struct ws_abstract_shell_surface*) surf_;

    if (!surf->surface) {
        return 0;
    }

    ws_surface_redraw(surf->surface);
    return 0;
}


static void
monitor_event(
    EV_P_
    ev_io* w,
    int revents
) {
    struct ws_monitor* self = wl_container_of(w, self, event_watcher);

    drmEventContext ev = {
        .version = DRM_EVENT_CONTEXT_VERSION,
        .page_flip_handler = handle_page_flip,
        .vblank_handler = NULL
    };

    ws_log(&log_ctx, LOG_DEBUG, "LIBEV handled drm event");

    drmHandleEvent(self->fb_dev->fd, &ev);
}

static void
handle_page_flip(
    int fd,
    unsigned int sequence,
    unsigned int tv_sec,
    unsigned int tv_usec,
    void* monitor
) {
    struct ws_monitor* self = (struct ws_monitor*) monitor;
    ws_log(&log_ctx, LOG_DEBUG, "Flippin' the surface!");
    ws_gbm_surface_release(self->gbm_surf);
    ws_monitor_redraw(self);
}
