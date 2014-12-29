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

#include <malloc.h>
#include <time.h>

// wayland-server.h has to be included before wayland-server-protocol.h
#include <wayland-server.h>
#include <wayland-server-protocol.h>

#include "compositor/internal_context.h"
#include "compositor/monitor.h"
#include "compositor/texture.h"
#include "compositor/wayland/client.h"
#include "compositor/wayland/region.h"
#include "compositor/wayland/surface.h"
#include "objects/set.h"
#include "util/wayland.h"

/**
 * Version of the wayland surface interface we're implementing
 */
#define WAYLAND_SURFACE_VERSION (1)

/*
 *
 * Forward declarations
 *
 */

static struct ws_logger_context log_ctx = { .prefix = "[Compositor/Surface] " };

/**
 * Destroy a surface
 *
 * See wayland server library documentation for details
 */
static void
surface_destroy_cb(
    struct wl_client* client, //!< client requesting the action
    struct wl_resource* resource //!< the resource affected by the action
);

/**
 * Attach a buffer to the surface
 *
 * See wayland server library documentation for details
 */
static void
surface_attach_cb(
    struct wl_client* client, //!< client requesting the action
    struct wl_resource* resource, //!< the resource affected by the action
    struct wl_resource* buffer, //!< buffer to attach to the surface
    int32_t x, //!< change in position to display, relative to the buffer: x
    int32_t y //!< change in position to display, relative to the buffer: y
);

/**
 * Mark an area as damaged
 *
 * See wayland server library documentation for details
 */
static void
surface_damage_cb(
    struct wl_client* client, //!< client requesting the action
    struct wl_resource* resource, //!< the resource affected by the action
    int32_t x, //!< x-coordinate of upper left corner of damaged area
    int32_t y, //!< y-coordinate of upper left corner of damaged area
    int32_t width, //!< width of damaged area
    int32_t height //!< height of damaged area
);

/**
 * Request a one-time notification on when to update the output
 *
 * See wayland server library documentation for details
 */
static void
surface_frame_cb(
    struct wl_client* client, //!< client requesting the action
    struct wl_resource* resource, //!< the resource affected by the action
    uint32_t callback //!< callback to call when it's time for a new frame
);

/**
 * Mark a region as being opaque
 *
 * See wayland server library documentation for details
 */
static void
surface_set_opaque_region_cb(
    struct wl_client* client, //!< client requesting the action
    struct wl_resource* resource, //!< the resource affected by the action
    struct wl_resource* region //!< region to set
);

/**
 * Mark a region as being an input region
 *
 * See wayland server library documentation for details
 */
static void
surface_set_input_region_cb(
    struct wl_client* client, //!< client requesting the action
    struct wl_resource* resource, //!< the resource affected by the action
    struct wl_resource* region //!< region to set
);

/**
 * Commit current state (perform a flip)
 *
 * See wayland server library documentation for details
 */
static void
surface_commit_cb(
    struct wl_client* client, //!< client requesting the action
    struct wl_resource* resource //!< the resource affected by the action
);

/**
 * Set a buffer transformation
 *
 * See wayland server library documentation for details
 */
static void
surface_set_buffer_transform_cb(
    struct wl_client* client, //!< client requesting the action
    struct wl_resource* resource, //!< the resource affected by the action
    int32_t transform //!< transformation to set
);

/**
 * Set a buffer scale
 *
 * See wayland server library documentation for details
 */
static void
surface_set_buffer_scale_cb(
    struct wl_client* client, //!< client requesting the action
    struct wl_resource* resource, //!< the resource affected by the action
    int32_t scale //!< scale to set
);


/**
 * Destroy the user data associated with a surface
 *
 * This method implements the destruction of the user data associated with a
 * surface.
 * This invalidates the object and performs an unref on it.
 *
 * @warning only call on resources which hold surfaces as constructed by
 *          ws_surface_new()
 */
static void
resource_destroy(
    struct wl_resource* resource
);


/*
 *
 * Internal constants
 *
 */

/**
 * Surface interface definition
 *
 * This interface definition holds all the methods for the surface type.
 */
static struct wl_surface_interface interface = {
    .destroy                = surface_destroy_cb,
    .attach                 = surface_attach_cb,
    .damage                 = surface_damage_cb,
    .frame                  = surface_frame_cb,
    .set_opaque_region      = surface_set_opaque_region_cb,
    .set_input_region       = surface_set_input_region_cb,
    .commit                 = surface_commit_cb,
    .set_buffer_transform   = surface_set_buffer_transform_cb,
    .set_buffer_scale       = surface_set_buffer_scale_cb,
};


/*
 *
 * Interface implementation
 *
 */

ws_object_type_id WS_OBJECT_TYPE_ID_SURFACE = {
    .supertype  = &WS_OBJECT_TYPE_ID_WAYLAND_OBJ,
    .typestr    = "ws_surface",

    .deinit_callback    = NULL,
    .hash_callback      = NULL,
    .cmp_callback       = NULL,

    .attribute_table    = NULL,
    .function_table = NULL,
};

struct ws_surface*
ws_surface_new(
    struct wl_client* client,
    uint32_t serial
) {
    struct ws_surface* self = calloc(1, sizeof(struct ws_surface));
    if (!self) {
        return NULL;
    }

    // try to set up the resource
    struct wl_resource* resource;
    resource = ws_wayland_client_create_resource(client, &wl_surface_interface,
                                  WAYLAND_SURFACE_VERSION, serial);
    if (!resource) {
        goto cleanup_surface;
    }

    // set the implementation
    wl_resource_set_implementation(resource, &interface,
                                   ws_object_getref(&self->wl_obj.obj),
                                   resource_destroy);

    // finish the initialisation
    ws_wayland_obj_init(&self->wl_obj, resource);
    self->wl_obj.obj.id = &WS_OBJECT_TYPE_ID_SURFACE;

    // initialize the members
    ws_wayland_buffer_init(&self->img_buf, NULL);

    ws_texture_init(&self->texture);

    glGenBuffers(1, &self->vbo);

    return self;
cleanup_surface:
    free(self);
    return NULL;
}

struct ws_surface*
ws_surface_from_resource(
    struct wl_resource* resource
) {
    if (!resource) {
        return NULL;
    }

    // check whether the resource is indeed a surface
    if (!wl_resource_instance_of(resource, &wl_surface_interface, &interface)) {
        return NULL;
    }

    return (struct ws_surface*) wl_resource_get_user_data(resource);
}


/*
 *
 * Internal implementation
 *
 */

static void
surface_destroy_cb(
    struct wl_client* client,
    struct wl_resource* resource
) {
    struct ws_surface* self;
    self = (struct ws_surface*) wl_resource_get_user_data(resource);

    // deinitialize the main image buffer
    ws_object_deinit(&self->img_buf.wl_obj.obj);
}

static void
surface_attach_cb(
    struct wl_client* client,
    struct wl_resource* resource,
    struct wl_resource* buffer,
    int32_t x,
    int32_t y
) {
    struct ws_surface* self;
    self = (struct ws_surface*) wl_resource_get_user_data(resource);

    ws_wayland_buffer_set_resource(&self->img_buf, buffer);

    self->offset_x = x;
    self->offset_y = y;
}

static void
surface_damage_cb(
    struct wl_client* client,
    struct wl_resource* resource,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height
) {
    //!< @todo: implement
}

static void
surface_frame_cb(
    struct wl_client* client,
    struct wl_resource* resource,
    uint32_t callback
) {
    //!< @todo: implement

    struct ws_surface* surface = ws_surface_from_resource(resource);
    surface->frame_callback = ws_wayland_client_create_resource(client,
                                           &wl_callback_interface, 1, callback);
    if (!surface->frame_callback) {
        return;
    }
    wl_resource_set_implementation(surface->frame_callback, NULL, NULL, NULL);
}

static void
surface_set_opaque_region_cb(
    struct wl_client* client,
    struct wl_resource* resource,
    struct wl_resource* region
) {
    //!< @todo: implement
}

static void
surface_set_input_region_cb(
    struct wl_client* client,
    struct wl_resource* resource,
    struct wl_resource* region
) {
    struct ws_surface* surface = ws_surface_from_resource(resource);

    if (!surface) {
        return;
    }
    if (surface->input_region) {
        ws_object_unref(&surface->input_region->wl_obj.obj);
    }
    surface->input_region = getref(ws_region_from_resource(region));
}

static void
surface_commit_cb(
    struct wl_client* client,
    struct wl_resource* resource
) {
    struct ws_surface* s = wl_resource_get_user_data(resource);

    if (s->role == &wl_pointer_interface) {
        return;
    }

    struct ws_buffer* buffer = ws_wayland_buffer_get_buffer(&s->img_buf);
    ws_buffer_transfer2texture(buffer, &s->texture);
    s->width = ws_buffer_width(buffer);
    s->height = ws_buffer_height(buffer);

    //!< @todo remove this

    if (s->parent) {
        s->parent->width = s->width;
        s->parent->height = s->height;
    }

    ws_log(&log_ctx, LOG_DEBUG, "Commited surface with size: %dx%d", s->width,
            s->height);

    if (s->frame_callback) {
        wl_callback_send_done(s->frame_callback,
                                clock() / (CLOCKS_PER_SEC/1000));
        wl_resource_destroy(s->frame_callback);
        s->frame_callback = NULL;
    }

    ws_wayland_buffer_release(&s->img_buf);

}

static void
surface_set_buffer_transform_cb(
    struct wl_client* client,
    struct wl_resource* resource,
    int32_t transform
) {
    //!< @todo: implement
}

static void
surface_set_buffer_scale_cb(
    struct wl_client* client,
    struct wl_resource* resource,
    int32_t transform
) {
    //!< @todo: implement
}

static int
sf_remove_surface(
    void* _surface,
    void const* mon
) {
    struct ws_surface* surface = (struct ws_surface*) _surface;
    ws_object_unref(&surface->wl_obj.obj);

    return 0;
}

static void
resource_destroy(
    struct wl_resource* resource
) {
    struct ws_surface* surface;
    surface = (struct ws_surface*) wl_resource_get_user_data(resource);
    // we don't need a null-check since we rely on the resource to ref a surface

    // Right now *every* monitor has this surface! So let's change that
    ws_set_select(&ws_comp_ctx.monitors, NULL, NULL,
                  sf_remove_surface, surface);

    struct ws_cursor* cursor = ws_cursor_get();

    if (cursor->active_surface == surface->parent) {
        ws_cursor_set_image(cursor, NULL);
    }

    // invalidate
    ws_object_lock_write(&surface->wl_obj.obj);
    surface->wl_obj.resource = NULL;
    ws_object_unlock(&surface->wl_obj.obj);
    ws_object_unref(&surface->wl_obj.obj);
}

int
ws_surface_set_role(
    struct ws_surface* self,
    struct wl_interface const* role
) {
    int ret = -1;
    ws_object_lock_write(&self->wl_obj.obj);

    if (!self || !role || (self->role && (self->role != role))) {
        goto unlock;
    }

    self->role = role;
    ret = 0;
unlock:
    ws_object_unlock(&self->wl_obj.obj);
    return ret;
}

void
ws_surface_redraw(
    struct ws_surface* s //!< The surface
) {
    ws_log(&log_ctx, LOG_DEBUG, "Redrawing!");
    GLushort indices[4] = {0, 1, 2, 3};

    // Format is x, y,  u, v
    int x =  s->parent->x + s->offset_x;
    int y =  s->parent->y + s->offset_y;
    GLfloat v[] = {
        x,            y,              0, 0,
        x           , y + s->height,  0, 1,
        x + s->width, y,              1, 0,
        x + s->width, y + s->height,  1, 1
    };

    ws_log(&log_ctx, LOG_DEBUG, "Drawing surface with size: %dx%d", s->width,
            s->height);

    ws_texture_bind(&s->texture, GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);

    glBindBuffer(GL_ARRAY_BUFFER, s->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_DYNAMIC_DRAW);

    const GLint stride = 4 * sizeof(GLfloat);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
            (void*) (2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, indices);
}

