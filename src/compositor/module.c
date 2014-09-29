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

#include <stdio.h>
#include <stdlib.h>

#include "compositor/module.h"

#include "wayland-server.h"


static void
initialize_egl(
    struct waysome_compositor* cs
) {
    // We get the default display
    cs->egl_disp.display = eglGetDisplay(NULL);
    eglInitialize(cs->egl_disp.display, &cs->egl_disp.major,
            &cs->egl_disp.minor);

    printf("Major: %d, Minor: %d", cs->egl_disp.major, cs->egl_disp.minor);
}

void
compositor_init(
    void
) {
    struct wl_display* display = wl_display_create();

    struct waysome_compositor* compositor = calloc(1, sizeof(*compositor));
    initialize_egl(compositor);

}
