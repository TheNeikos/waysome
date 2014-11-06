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

#include <stddef.h>
#include <string.h>

#include "serialize/json/keys.h"
#include "serialize/json/states.h"
#include "serialize/json/string_jump_state.h"

/**
 * Map for mapping the current state with a string on the next state
 */
static const struct {
    enum json_backend_state current;
    enum json_backend_state next;
    const char* str;
} MAP[] = {
    { .current = STATE_MSG, .next = STATE_UID,      .str = UID          },
    { .current = STATE_MSG, .next = STATE_TYPE,     .str = TYPE         },
    { .current = STATE_MSG, .next = STATE_COMMANDS, .str = COMMANDS     },
    { .current = STATE_MSG, .next = STATE_FLAGS,    .str = FLAGS        },
    {
        .current = STATE_FLAGS_MAP,
        .next = STATE_FLAGS_EXEC,
        .str = FLAG_EXEC
    },
    {
        .current = STATE_FLAGS_MAP,
        .next = STATE_FLAGS_REGISTER,
        .str = FLAG_REGISTER
    },
    {
        .current = STATE_MSG,
        .next = STATE_EVENT_NAME,
        .str = EVENT_NAME
    },
    {
        .current = STATE_MSG,
        .next = STATE_EVENT_VALUE,
        .str = EVENT_VALUE,
    },

    { .str = NULL },
};

/*
 *
 * Interface implementation
 *
 */

enum json_backend_state
get_next_state_for_string(
    enum json_backend_state current,
    const unsigned char * str
) {
    for (size_t i = 0; MAP[i].str; i++) {
        if (MAP[i].current == current) {
            if (0 == strncmp(MAP[i].str, (char*) str, strlen(MAP[i].str))) {
                return MAP[i].next;
            }
        }
    }

    return STATE_INVALID;
}
