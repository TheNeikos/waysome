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

#ifndef __WAYSOME_ABSTRACT_SHELL_SURFACE_H__
#define __WAYSOME_ABSTRACT_SHELL_SURFACE_H__

#include "objects/wayland_obj.h"

// forward declarations
struct ws_surface; // definition in compositor/surface.h
struct wl_interface; // definition in wayland headers
struct ws_compositing_event; //defintion in compositor.h


/**
 * An abstract shell surface to have an unified way to access the surface.
 */
struct ws_abstract_shell_surface {
    struct ws_wayland_obj wl_obj; //!< @protected: The associated wayland object
    struct ws_surface* surface; //!< @public: The associated surface
    struct ws_monitor* monitor; //!< @private: The monitor the shell is on
    struct ws_image_buffer* cache_buffer; //!< @private: The temp buffer
    bool updated; //!< Flag for if the shell was updated
    bool visible; //!< Flag for if the shell is visible
    int height; //!< The height of the window
    int width; //!< The width of the window
    int x; //!< The x position of the window
    int y; //!< The y position of the window
};

/**
 * Variable which holds type information about the wl_surface type
 */
extern ws_object_type_id WS_OBJECT_TYPE_ID_ABSTRACT_SHELL_SURFACE;

/**
 * Create a new abstract shell surface
 *
 * @warning only to be used in derived types
 *
 * @return 0 on success, a negative error number otherwise
 */
int
ws_abstract_shell_surface_init(
    struct ws_abstract_shell_surface* self, //!< surface to initialize
    struct wl_resource* resource, //!< resource to set
    struct ws_surface* surface, //!< surface to attach
    struct wl_interface const* role, //!< role to give the surface
    struct ws_monitor* monitor //!< The monitor the surface is on
);

/**
 * Set the position of the surface
 *
 * @memberof ws_abstract_shell_surface
 *
 * @return 0 on success, a negative error number otherwise
 */
int
ws_abstract_shell_surface_set_pos(
    struct ws_abstract_shell_surface* self, //!< surface to modify
    int32_t x, //!< x position
    int32_t y //!< y position
)
__ws_nonnull__(1)
;

/**
 * Set the width of a ws_abstract_shell_surface
 *
 * @memberof ws_abstract_shell_surface
 *
 * @return zero on success, else negative errno.h number
 */
int
ws_abstract_shell_surface_set_width(
    struct ws_abstract_shell_surface* self, //!< surface to modify
    int32_t width //!< The new width
);

/**
 * Set the height of a ws_abstract_shell_surface
 *
 * @memberof ws_abstract_shell_surface
 *
 * @return zero on success, else negative errno.h number
 */
int
ws_abstract_shell_surface_set_height(
    struct ws_abstract_shell_surface* self, //!< surface to modify
    int32_t height //!< The new height
);

/**
 * Set the width and height of a ws_abstract_shell_surface
 *
 * @memberof ws_abstract_shell_surface
 *
 * @return zero on success, else negative errno.h number
 */
int
ws_abstract_shell_surface_set_width_and_height(
    struct ws_abstract_shell_surface* self, //!< surface to modify
    int32_t width, //!< The new width
    int32_t height //!< The new height
);

/**
 * Redraws the shell on it's given monitor
 */
void
ws_abstract_shell_surface_redraw(
    struct ws_compositing_event* event, //!< The event that caused this redraw
    void* dummy //!< Additional data given to this function (unused here)
);

/**
 * Does the actual composition, copying all the buffers into the given position
 */
void
ws_abstract_shell_surface_composite(
    struct ws_abstract_shell_surface* surface, //!< surface to draw
    struct ws_monitor* monitor //!< The monitor to composite for
);

/**
 * Does the actual composition, copying all the buffers into the given position
 */
void
ws_abstract_shell_surface_update(
    struct ws_abstract_shell_surface* self //!< surface to update
);

#endif // __WAYSOME_ABSTRACT_SHELL_SURFACE_H__
