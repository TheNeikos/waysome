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

/**
 * @addtogroup compositor "Compositor"
 *
 * @{
 */

/**
 * @addtogroup compositor_buffer "Compositor GBM Buffer"
 *
 * @{
 */

#ifndef __WS_OBJECTS_FRAME_GBM_SURFACE_H__
#define __WS_OBJECTS_FRAME_GBM_SURFACE_H__

#include <pthread.h>
#include <stdbool.h>
#include <ev.h>

#include "compositor/monitor.h"
#include "objects/object.h"

/**
 * ws_gbm type definition
 *
 * This holds the gbm_surface and handles locking/releasing the display
 *
 * It can also be bound to a monitor.
 *
 */
struct ws_gbm_surface {
    struct ws_object obj; //!< @protected Base class.
    struct ws_framebuffer_device* fb_dev; //!< @public Framebuffer Device
    struct gbm_surface* surf; //!< @private GBM Surface
    uint32_t width; //!< @public width of the gbm_surface
    uint32_t height; //!< @public height of the gbm_surface
    struct {
        struct gbm_bo* gbm_fb; //!< @public the bo buffer
        uint32_t handle;
        uint32_t stride;
        uint32_t fb;
    } fb[2];
    short cur_fb;
    bool pflip_pending; //!< @public page flip pending?
};

/**
 * Variable which holds the type information about the ws_frame_buffer type
 */
extern ws_object_type_id WS_OBJECT_TYPE_ID_gbm_surface;

/**
 * Get a gbm buffer from the given framebuffer device
 *
 * @memberof ws_gbm_surface
 *
 * @return the gbm buffer on success, NULL on failure
 */
struct ws_gbm_surface*
ws_gbm_surface_new(
    struct ws_framebuffer_device* fb_dev,
    struct ws_monitor* monitor,
    int width,
    int height
);

/**
 * Flips the gbm buffer to the front
 */
void
ws_gbm_surface_flip(
    struct ws_gbm_surface* surf,
    struct ws_monitor* monitor
);

/**
 * This redraws the buffer to the given monitor!
 */
void
ws_gbm_surface_lock(
    struct ws_gbm_surface* surf, //!< The surface to be bound
    struct ws_monitor* monitor//!< The monitor to which it should be drawn
);

/**
 * Releases the buffers bound by it
 */
void
ws_gbm_surface_release(
    struct ws_gbm_surface* surf //!< The surface on which to release the buffers
);

#endif // __WS_OBJECTS_FRAME_GBM_SURFACE_H__

/**
 * @}
 */

/**
 * @}
 */

