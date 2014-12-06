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

#ifndef __WS_WL_SURFACE_H__
#define __WS_WL_SURFACE_H__

#include "compositor/wayland/buffer.h"
#include "objects/wayland_obj.h"


// forward declarations
struct wl_client;


/**
 * Waysome's implementation of wl_surface
 *
 * This struct represents a surface
 *
 * @extends ws_wayland_obj
 */
struct ws_surface {
    struct ws_wayland_obj wl_obj; //!< @protected Base class.
    struct ws_wayland_buffer img_buf; //!< @protected image buffer
    struct ws_region* input_region; //!< @protected input region
    struct wl_resource* frame_callback; //!< @protected frame callback
    struct wl_interface const* role; //!< @protected role of this surface
    int32_t x; //!< @public x position of this surface
    int32_t y; //!< @public y position of this surface
    int32_t width; //!< @public width of the surface
    int32_t height; //!< @public height of the surface
    struct {
        struct ws_wayland_buffer commit_buf; //!< @private buffer for commit
        int32_t flags; //!< @private bit flags about the current commit status
        int32_t x; //!< @public possible x position of this surface
        int32_t y; //!< @public possible y position of this surface
    } commit; //!< @private members about the current commit
};

/**
 * Variable which holds type information about the wl_surface type
 */
extern ws_object_type_id WS_OBJECT_TYPE_ID_SURFACE;

/**
 * Create a new surface
 *
 * @memberof ws_surface
 *
 * create a new surface without any bufers attached to it
 */
struct ws_surface*
ws_surface_new(
    struct wl_client* client, //!< client requesting the surface creation
    uint32_t serial //!< id of the newly created surface
);

/**
 * Get a surface from a resource
 *
 * Extracts the surface from a resource.
 *
 * @memberof ws_surface
 *
 * @return the surface or NULL, if the resource is not a wl_surface.
 */
struct ws_surface*
ws_surface_from_resource(
    struct wl_resource* resource
);

/**
 * Set the role of a surface
 *
 * By the wayland specification a surface can only have one type of role at
 * a time and that until it gets deleted.
 *
 * @memberof ws_surface
 *
 * @return 0 on success and a negative value on failure
 */
int
ws_surface_set_role(
    struct ws_surface* self, //!< the surface
    struct wl_interface const* role //!< our role
);

/**
 * Regenerate the internal state of the surface
 *
 * This is to be called after any of the following properties change:
 *      1. width
 *      2. height
 *      3. z-order (TBD)
 *
 * @warning This can be an expensive process and shouldn't be called too often
 */
void
ws_surface_regenerate_state(
    struct ws_surface* self //!< the surface
);

#endif // __WS_WL_SURFACE_H__

