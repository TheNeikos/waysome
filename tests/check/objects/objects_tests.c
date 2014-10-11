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
 * @addtogroup tests "Testing"
 *
 * @{
 */

/**
 * @addtogroup tests_objects "Testing: Objects"
 *
 * @{
 */

#include <check.h>
#include "tests.h"

#include "objects/object.h"

START_TEST (test_object_init) {
    struct ws_object o;
    memset(&o, 0, sizeof(o));

    ck_assert(ws_object_init(&o));
    ck_assert(ws_object_deinit(&o));
}
END_TEST

START_TEST (test_object_alloc) {
    ck_assert(ws_object_new(1) == NULL);

    struct ws_object* o = ws_object_new(sizeof(*o));
    ck_assert(o != NULL);

    ws_object_deinit(o);
}
END_TEST

static Suite*
objects_suite(void)
{
    Suite* s    = suite_create("Objects");
    TCase* tc   = tcase_create("main case");

    suite_add_tcase(s, tc);
    // tcase_add_checked_fixture(tc, setup, cleanup); // Not used yet

    tcase_add_test(tc, test_object_init);
    tcase_add_test(tc, test_object_alloc);

    return s;
}

WS_TESTS_CHECK_MAIN(objects_suite);

/**
 * @}
 */

/**
 * @}
 */
