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

/**
 * @addtogroup serializer_module "Serializer module"
 *
 * @{
 */

/**
 * @addtogroup serializer_module_json_backend "Serializer JSON backend"
 *
 * @{
 */

/**
 * @addtogroup serializer_module_json_backend_deser "JSON backend deserializer"
 *
 * Serializer module JSON backend private utilities
 *
 * @{
 */

#ifndef __WS_SERIALIZE_JSON_DESERIALIZER_STATE_H__
#define __WS_SERIALIZE_JSON_DESERIALIZER_STATE_H__

#include <stdbool.h>
#include <stdint.h>
#include <yajl/yajl_parse.h>

#include "command/command.h"
#include "command/statement.h"

#include "serialize/json/states.h"

#include "objects/message/transaction.h"

#include "values/union.h"
#include "values/string.h"

/**
 * Deserializer state object
 */
struct deserializer_state {
    yajl_handle handle;

    enum json_backend_state current_state; //!< @protected State identifier

    uintmax_t id;

    enum ws_transaction_flags flags; //!< @public flag cache
    struct ws_string* register_name; //!< @public name cache

    struct ws_statement* tmp_statement;

    uintmax_t nboxbrackets;
    uintmax_t ncurvedbrackets;

    struct ws_string* ev_name; //!< @public name cache for event name
    struct ws_value* ev_ctx; //!< @public event context

    bool has_event;
};

/**
 * Allocate a new desertializer state object
 *
 * @return new object of `struct serializer_yajl_state_deserializer` or NULL
 */
struct deserializer_state*
deserialize_state_new(yajl_callbacks* cbs, void* ctx);

#endif //__WS_SERIALIZE_JSON_DESERIALIZER_STATE_H__

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
