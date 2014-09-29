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

#ifndef __WS_COMPOSITOR_MODULE_H__
#define __WS_COMPOSITOR_MODULE_H__

#include "EGL/egl.h"
#include "wayland-server.h"

struct egl_display {
    EGLDisplay display;
    EGLint major;
    EGLint minor;
};

struct waysome_compositor {
    struct egl_display egl_disp;
};

void
compositor_init(
    void
);

void
compositor_create_surface(
    struct wl_client* client,
    struct wl_resource* res,
    uint32_t id
);

void
compositor_create_region(
    struct wl_client* client,
    struct wl_resource* res,
    uint32_t id
);

#endif // __WS_COMPOSITOR_MODULE_H__
