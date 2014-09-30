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

#include "compositor/module.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <xf86drm.h>


/**
 * Internal compositor context
 *
 * This context holds the internal state of the compositor.
 */
static struct ws_compositor_context{
    struct ws_framebuffer_device fb;
    struct ws_monitor* conns; //<! A linked list of ws_monitors
} comp_ctx;

static int
find_connector_with_crtc(
    int crtc
) {
    for(struct ws_monitor* iter = comp_ctx.conns; iter; iter = iter->next) {
        if (iter->crtc == (uint32_t)crtc) {
            return -1;
        }
    }
    return 0;
}

static int
find_crtc(
    drmModeRes* res,
    drmModeConnector* conn,
    struct ws_monitor* connector
) {
    drmModeEncoder* enc;
    int32_t crtc;

    // We check if we already have found a suitable encoder
    if (conn->encoder_id) {
        enc = drmModeGetEncoder(comp_ctx.fb.fd, conn->encoder_id);
    } else {
        enc = NULL;
    }

    // If we do have an encoder, we check that noone else uses this crtc
    if (enc) {
        if (enc->crtc_id) {
            crtc = enc->crtc_id;

            int ret = find_connector_with_crtc(crtc);

            if (ret >= 0) {
                drmModeFreeEncoder(enc);
                connector->crtc = crtc;
                return 0;
            }
        }

        drmModeFreeEncoder(enc);
    }
    // There is no encoder+crtc pair! We go through /all/ the encoders now
    for (int i = 0; i < conn->count_encoders; ++i) {
        enc = drmModeGetEncoder(comp_ctx.fb.fd, conn->encoders[i]);

        if (!enc) {
            // TODO: Log Error!
            continue;
        }

        for( int j = 0; j < res->count_crtcs; ++j) {
            // Check if this encoding supports with all the crtcs
            if(!(enc->possible_crtcs & (1 << j))) {
                continue;
            }

            // Check noone else uses it!
            crtc = res->crtcs[j];
            int ret = find_connector_with_crtc(crtc);

            // Looks like we found one! Return!
            if (ret >= 0) {
                drmModeFreeEncoder(enc);
                connector->crtc = crtc;
                return 0;
            }
        }

        drmModeFreeEncoder(enc);
    }

    // TODO: Log Error! No CRTC FOUND!!!
    return -ENOENT;
}

/**
 * Get all connectors on the given framebuffer device
 *
 * This writes directly into the given compositor context
 */
static int
populate_connectors(
) {
    drmModeRes* res;
    drmModeConnector* conn;
    struct ws_monitor** connector = &comp_ctx.conns;

    res = drmModeGetResources(comp_ctx.fb.fd);
    if (!res) {
        // TODO: Log Error
        return -ENOENT;
    }

    // Let's go through all connectors (outputs)
    int i = res->count_connectors;
    while(i--) {
        conn = drmModeGetConnector(comp_ctx.fb.fd, res->connectors[i]);
        if (!conn) {
            // TODO: Log Error
            continue;
        }
        if (*connector) {
            (*connector)->next = calloc(1, sizeof(**connector));
            *connector = (*connector)->next;
        } else {
            *connector = calloc(1, sizeof(**connector));
        }
        (*connector)->conn = conn->connector_id;

        if (conn->connection != DRM_MODE_CONNECTED) {
            // TODO: Log Unused
            (*connector)->connected = 0;
            continue;
        }

        if (conn->count_modes == 0) {
            // TODO: Log No Valid Modes
            (*connector)->connected = 0;
            continue;
        }

        // TODO: Do not just take the biggest mode available
        memcpy(&(*connector)->mode, &conn->modes[0],
                sizeof((*connector)->mode));

        (*connector)->width = conn->modes[0].hdisplay;
        (*connector)->height = conn->modes[0].vdisplay;

        int ret = find_crtc(res, conn, *connector);
        if (ret < 0) {
            // TODO: Log error about not finding crtcs
            (*connector)->connected = 0;
            continue;
        }
        (*connector)->connected = 1;
    }
    return 0;
}


/**
 * Get a Frambuffer Device provided by DRM using the given path
 *
 * This is the graphic card device that is given by the kernel. It loads it
 * and populates the given framebuffer with the information found.
 */
static int
get_framebuffer_device(
    const char* path
) {
    int fd = open(path, O_RDWR | O_CLOEXEC);
    if (fd < 0)
    {
        // TODO: Log Error!
        return -ENOENT;
    }

    uint64_t has_dumb;
    if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || !has_dumb) {
        //TODO: Log Error!
        close(fd);
        return -EOPNOTSUPP;
    }
    comp_ctx.fb.path = path;
    comp_ctx.fb.fd = fd;
    return 0;
}


/*
 *
 * Interface implementation
 *
 */

int
ws_compositor_init(void) {
    static bool is_init = false;
    if (is_init) {
        return 0;
    }

    get_framebuffer_device("/dev/dri/card0");

    populate_connectors();

    //!< @todo: create a framebuffer for each connector

    //!< @todo: prelimary: preload a PNG from a hardcoded path

    //!< @todo: prelimary: blit the preloaded PNG on each of the frame buffers

    is_init = true;
    return 0;
}

void
ws_compositor_deinit(void) {

    if (comp_ctx.fb.fd >= 0) {
        close(comp_ctx.fb.fd);
    }

    struct ws_monitor* it = comp_ctx.conns;

    while(it) {
        struct ws_monitor* tmp = it->next;
        free(it);
        it = tmp;
    }

    //!< @todo: free all of the framebuffers

    //!< @todo: prelimary: free the preloaded PNG
}

