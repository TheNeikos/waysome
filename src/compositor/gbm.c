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

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#define EGL_EGLEXT_PROTOTYPES 1
#include <EGL/eglext.h>
#include <gbm.h>
#include <xf86drmMode.h>

#include "compositor/gbm.h"
#include "compositor/internal_context.h"
#include "compositor/monitor.h"
#include "logger/module.h"
#include "objects/object.h"

/*
 *
 *  Forward declarations
 *
 */

static bool
gbm_surface_deinit(
    struct ws_object* self
);

/*
 *
 * Interface Implementation
 *
 */

ws_object_type_id WS_OBJECT_TYPE_ID_GBM_SURFACE = {
        .supertype  = (ws_object_type_id*) &WS_OBJECT_TYPE_ID_RAW_BUFFER,
        .typestr    = "ws_frame_buffer",

        .hash_callback = NULL,

        .deinit_callback = gbm_surface_deinit,
        .cmp_callback = NULL,

        .attribute_table = NULL,
        .function_table = NULL,
};

struct ws_gbm_surface*
ws_gbm_surface_new(
    struct ws_framebuffer_device* fb_dev,
    struct ws_monitor* monitor,
    int width,
    int height
) {
    struct ws_gbm_surface* tmp = calloc(1, sizeof(*tmp));
    if (!tmp) {
        return NULL;
    }

    ws_object_init(&tmp->obj);
    tmp->obj.settings |= WS_OBJECT_HEAPALLOCED;
    tmp->obj.id = (ws_object_type_id *) &WS_OBJECT_TYPE_ID_GBM_SURFACE;
    tmp->fb_dev = fb_dev;

    struct gbm_device* gbm = ws_framebuffer_device_get_gbm_dev(tmp->fb_dev);

    if (!gbm) {
        goto cleanup_tmp;
    }

    tmp->surf = gbm_surface_create(gbm, width, height,  GBM_BO_FORMAT_ARGB8888,
                                   GBM_BO_USE_RENDERING);

    if (!tmp->surf) {
        goto cleanup_tmp;
    }

    tmp->width = width;
    tmp->height = height;
    tmp->cur_fb = 0;

    return tmp;
cleanup_tmp:
    free(tmp);
    return NULL;
}

void
ws_gbm_surface_lock(
    struct ws_gbm_surface* surf,
    struct ws_monitor* monitor
) {

    surf->fb[surf->cur_fb].gbm_fb = gbm_surface_lock_front_buffer(surf->surf);

    struct gbm_bo* bo = surf->fb[surf->cur_fb].gbm_fb;
    surf->fb[surf->cur_fb].handle = gbm_bo_get_handle(bo).u32;
    surf->fb[surf->cur_fb].stride = gbm_bo_get_stride(bo);

    drmModeAddFB(surf->fb_dev->fd,
            monitor->current_mode->mode.hdisplay,
            monitor->current_mode->mode.vdisplay,
            24, 32, surf->fb[surf->cur_fb].stride,
            surf->fb[surf->cur_fb].handle, &surf->fb[surf->cur_fb].fb);
}

void
ws_gbm_surface_flip(
    struct ws_gbm_surface* surf,
    struct ws_monitor* monitor
) {
    drmModePageFlip(ws_comp_ctx.fb->fd, monitor->crtc,
                    surf->fb[surf->cur_fb].fb, DRM_MODE_PAGE_FLIP_EVENT,
                    monitor);
}

void
ws_gbm_surface_release(
    struct ws_gbm_surface* surf
) {
    gbm_surface_release_buffer(surf->surf,
                               surf->fb[surf->cur_fb ^ 1].gbm_fb);
    drmModeRmFB(surf->fb_dev->fd, surf->fb[surf->cur_fb ^ 1].fb);
    surf->cur_fb ^= 1;
}

/*
 *
 * Internal implementation
 *
 */

static bool
gbm_surface_deinit(
    struct ws_object* obj
) {
    struct ws_gbm_surface* self = (struct ws_gbm_surface*) obj;
    if (self->surf) {
        gbm_surface_destroy(self->surf);
    }

    return true;
}


