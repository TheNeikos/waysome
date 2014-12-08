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

#ifndef __WS_WL_COMPOSITOR_H__
#define __WS_WL_COMPOSITOR_H__

struct ws_compositing_event; //defined in compositing_event.h

/**
 * Initialize the wayland side of the compositor
 *
 * @return 0 if the initialisation was successful, a negative error code
 *         otherwise.
 */
int
ws_wayland_compositor_init(void);

/**
 *  This empties the event queue of compositing events, applying all
 *  transformations and flipping the buffers.
 */
void
ws_wayland_compositor_flush(void);

/**
 * This adds an event to the list of transformations to be done until the next
 * flush.
 */
void
ws_wayland_compositor_add_event(
    struct ws_compositing_event* event
);

#endif // __WS_WL_SURFACE_H__

