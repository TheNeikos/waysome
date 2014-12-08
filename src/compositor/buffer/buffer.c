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

#include <errno.h>
#include <string.h>

#include "util/arithmetical.h"

#include "compositor/buffer/buffer.h"
#include "compositor/internal_context.h"

ws_buffer_type_id WS_OBJECT_TYPE_ID_BUFFER = {
    .type = {
        .supertype  = &WS_OBJECT_TYPE_ID_OBJECT,
        .typestr    = "ws_buffer",

        .hash_callback = NULL,

        .deinit_callback = NULL,
        .cmp_callback = NULL,

        .attribute_table = NULL,
        .function_table = NULL,
    },
    .get_data = NULL,
    .get_width = NULL,
    .get_height = NULL,
    .get_stride = NULL,
    .get_format = NULL,
    .begin_access = NULL,
    .end_access = NULL,
};

int
ws_buffer_init(
    struct ws_buffer* self
) {
    if (!self) {
        return -EINVAL;
    }

    ws_object_init(&self->obj);
    self->obj.id = &WS_OBJECT_TYPE_ID_BUFFER.type;

    return 0;
}


void*
ws_buffer_data(
    struct ws_buffer const* self
) {
    ws_buffer_type_id* type = (ws_buffer_type_id*) self->obj.id;

    // search for an implementation in the base classes
    while (!type->get_data) {
        // we hit the basic, abstract buffer type, which does nothing
        if (type == &WS_OBJECT_TYPE_ID_BUFFER) {
            return NULL;
        }
        type = (ws_buffer_type_id*) type->type.supertype;
    }
    return type->get_data(self);
}

int32_t
ws_buffer_width(
    struct ws_buffer const* self
) {
    ws_buffer_type_id* type = (ws_buffer_type_id*) self->obj.id;

    // search for an implementation in the base classes
    while (!type->get_width) {
        // we hit the basic, abstract buffer type, which does nothing
        if (type == &WS_OBJECT_TYPE_ID_BUFFER) {
            return 0;
        }
        type = (ws_buffer_type_id*) type->type.supertype;
    }
    return type->get_width(self);
}

int32_t
ws_buffer_height(
    struct ws_buffer const* self
) {
    ws_buffer_type_id* type = (ws_buffer_type_id*) self->obj.id;

    // search for an implementation in the base classes
    while (!type->get_height) {
        // we hit the basic, abstract buffer type, which does nothing
        if (type == &WS_OBJECT_TYPE_ID_BUFFER) {
            return 0;
        }
        type = (ws_buffer_type_id*) type->type.supertype;
    }
    return type->get_height(self);
}

int32_t
ws_buffer_stride(
    struct ws_buffer const* self
) {
    ws_buffer_type_id* type = (ws_buffer_type_id*) self->obj.id;

    // search for an implementation in the base classes
    while (!type->get_stride) {
        // we hit the basic, abstract buffer type, which does nothing
        if (type == &WS_OBJECT_TYPE_ID_BUFFER) {
            return 0;
        }
        type = (ws_buffer_type_id*) type->type.supertype;
    }
    return type->get_stride(self);
}

uint32_t
ws_buffer_format(
    struct ws_buffer const* self
) {
    ws_buffer_type_id* type = (ws_buffer_type_id*) self->obj.id;

    // search for an implementation in the base classes
    while (!type->get_format) {
        // we hit the basic, abstract buffer type, which does nothing
        if (type == &WS_OBJECT_TYPE_ID_BUFFER) {
            return 0; //!< @todo: return invalid format
        }
        type = (ws_buffer_type_id*) type->type.supertype;
    }
    return type->get_format(self);
}

uint32_t
ws_buffer_bpp(
    struct ws_buffer const* self
) {
    ws_buffer_type_id* type = (ws_buffer_type_id*) self->obj.id;

    // search for an implementation in the base classes
    while (!type->get_bpp) {
        // we hit the basic, abstract buffer type, which does nothing
        if (type == &WS_OBJECT_TYPE_ID_BUFFER) {
            return 0; //!< @todo: return invalid format
        }
        type = (ws_buffer_type_id*) type->type.supertype;
    }
    return type->get_bpp(self);
}

void
ws_buffer_begin_access(
    struct ws_buffer* self
) {
    ws_buffer_type_id* type = (ws_buffer_type_id*) self->obj.id;

    // search for an implementation in the base classes
    while (!type->begin_access) {
        // we hit the basic, abstract buffer type, which does nothing
        if (type == &WS_OBJECT_TYPE_ID_BUFFER) {
            return;
        }
        type = (ws_buffer_type_id*) type->type.supertype;
    }
    type->begin_access(self);
}

void
ws_buffer_end_access(
    struct ws_buffer* self
) {
    ws_buffer_type_id* type = (ws_buffer_type_id*) self->obj.id;

    // search for an implementation in the base classes
    while (!type->end_access) {
        // we hit the basic, abstract buffer type, which does nothing
        if (type == &WS_OBJECT_TYPE_ID_BUFFER) {
            return;
        }
        type = (ws_buffer_type_id*) type->type.supertype;
    }
    type->end_access(self);
}

void
ws_buffer_blit(
    struct ws_buffer* dest,
    struct ws_buffer const* src
) {
    void* buf_dst = ws_buffer_data(dest);
    void* buf_src = ws_buffer_data(src);
    if (!(buf_dst && buf_src)) {
        return;
    }

    //!< @todo use byte-size instead of stride
    ws_log(&log_ctx, LOG_DEBUG, "Blitting image with dim: %dx%d with bpp:%d",
            ws_buffer_width(src),
            ws_buffer_height(src),
            ws_buffer_bpp(src));
    int min_x = MIN(ws_buffer_width(dest) * ws_buffer_bpp(dest),
            ws_buffer_width(src) * ws_buffer_bpp(src));
    int min_y = MIN(ws_buffer_height(dest), ws_buffer_height(src));

    int stride_dst = ws_buffer_stride(dest);
    int stride_src = ws_buffer_stride(src);
    for (int y = 0; y < min_y; ++y) {
        memcpy(((char*) buf_dst) + (y * stride_dst),
                ((char*) buf_src) + (y * stride_src),
                min_x
        );
    }
}

void
ws_buffer_blit_at(
    struct ws_buffer* dest,
    struct ws_buffer const* src,
    int const x,
    int const y
) {
    void* buf_dst = ws_buffer_data(dest);
    void* buf_src = ws_buffer_data(src);
    if (!(buf_dst && buf_src)) {
        return;
    }

    //!< @todo use byte-size instead of stride
    ws_log(&log_ctx, LOG_DEBUG, "Blitting image with dim: %dx%d at (%d, %d)",
            ws_buffer_width(src),
            ws_buffer_height(src),
            x, y);

    // The maximal width can copy. We substract the y position to account for
    // displacement /into/ the destination buffer.
    int max_width =  MIN((ws_buffer_width(dest) - y) * ws_buffer_bpp(dest),
                         ws_buffer_width(src)  * ws_buffer_bpp(src));

    // How many rows can we copy at max?
    int max_height = MIN((ws_buffer_height(dest) - x), ws_buffer_height(src));

    int stride_dest = ws_buffer_stride(dest);
    int stride_src = ws_buffer_stride(src);

    //
    //  We go into the buffer at the x and y position
    //     <--------Stride------->
    //     +---------------------+
    //     |        ^            |
    //     |        x            |
    //     |        v            |
    //     |<   y  >+---+        |
    //     |        |###|        |
    //     |        |###|        |
    //     |        +---+        |
    //     +---------------------+
    //  This is done by adding the corret offset. Since a stride is the full
    //  length of a row we just multiply the x value with it.
    //  BPP = Bytes per Pixel. This way we then go to the left in the buffer.
    //  From then on we just
    buf_dst = ((char*) buf_dst) + (x * stride_dest) + (y * ws_buffer_bpp(dest));

    // vert = vertical position
    for (int vert = 0; vert < max_height; ++vert) {
        // We copy a whole row from one buffer into the other
        memcpy(((char*) buf_dst) + (vert * stride_dest),
               ((char*) buf_src) + (vert * stride_src),
               max_width);
    }

}
