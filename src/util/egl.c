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

#include "png.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <wayland-server.h>

#include "util/arithmetical.h"
#include "util/egl.h"


/*
 *
 * Internal constants
 *
 */

static struct ws_egl_fmt const mappings[] = {
    {
        .shm_fmt = WL_SHM_FORMAT_RGBA8888,
        .egl = { .fmt = GL_RGBA, .type = GL_UNSIGNED_BYTE },
        .png_fmt = PNG_FORMAT_RGBA,
        .bpp = 4
    },
    {
        .shm_fmt = WL_SHM_FORMAT_RGBX8888,
        .egl = { .fmt = GL_RGBA, .type = GL_UNSIGNED_BYTE },
        .png_fmt = PNG_FORMAT_RGBA,
        .bpp = 4
    },
    {
        .shm_fmt = WL_SHM_FORMAT_ARGB8888,
        .egl = { .fmt = 0, .type = 0 },
        .png_fmt = PNG_FORMAT_ARGB,
        .bpp = 4
    },
    {
        .shm_fmt = WL_SHM_FORMAT_XRGB8888,
        .egl = { .fmt = 0, .type = 0 },
        .png_fmt = PNG_FORMAT_ARGB,
        .bpp = 4
    }
};


/*
 *
 * Interface implementation
 *
 */

struct ws_egl_fmt const*
ws_egl_fmt_from_shm_fmt(
    enum wl_shm_format shm_fmt
) {
    struct ws_egl_fmt const* mapping = mappings + ARYLEN(mappings);

    while (mapping-- > mappings) {
        if (mapping->shm_fmt == shm_fmt) {
            return mapping;
        }
    }

    return NULL;
}

struct ws_egl_fmt const*
ws_egl_fmt_get_rgba() {
    return mappings;
}

struct ws_egl_fmt const*
ws_egl_fmt_get_argb()
{
    return ws_egl_fmt_from_shm_fmt(WL_SHM_FORMAT_ARGB8888);
}

int
ws_egl_fmt_advertise(
    struct wl_display* display
) {
    int retval = -1;

    struct ws_egl_fmt const* mapping = mappings + ARYLEN(mappings);

    while (mapping-- > mappings) {
        if (!mapping->egl.fmt || !mapping->egl.type) {
            continue;
        }

        if (wl_display_add_shm_format(display, mapping->shm_fmt)) {
            retval = 0;
        }
    }

    return retval;
}

