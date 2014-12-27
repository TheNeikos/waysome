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

#include "compositor/gl_program.h"

/**
 * Static structs
 */

ws_object_type_id WS_OBJECT_TYPE_ID_GL_PROGRAM= {
    .supertype  = &WS_OBJECT_TYPE_ID_OBJECT,
    .typestr    = "ws_gl_program",

    .deinit_callback    = NULL,
    .hash_callback      = NULL,
    .cmp_callback       = NULL,

    .attribute_table    = NULL,
    .function_table     = NULL,
};

/*
 *
 * Implementations
 *
 */

struct ws_gl_program*
ws_gl_program_new_from_files(
    char* vertex,
    char* fragment
) {
    //!< @todo implement
    return NULL;
}

int
ws_gl_program_init(
    struct ws_gl_program* program
) {
    //!< @todo implement
    return -1;
}

bool
ws_gl_program_check_linking(
    struct ws_gl_program* program,
    char* vertex_error,
    char* fragment_error,
    char* program_error
) {
    //!< @todo implement
    return false;
}

