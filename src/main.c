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
#include <ev.h>

static void
handle_sig(
    struct ev_loop* loop,
    struct ev_signal* w,
    int revent
) {
    // TODO Do proper logging here
    fprintf(stderr, "Caught signal %d\n", w->signum);
    ev_unloop(loop, EVUNLOOP_ALL);
}

int
main(
    int argc,
    char** argv
) {

    struct ev_loop* default_loop = ev_default_loop(EVFLAG_AUTO);

    struct ev_signal sigint_watcher;
    ev_signal_init(&sigint_watcher, handle_sig, SIGINT);
    ev_signal_start(default_loop, &sigint_watcher);

    struct ev_signal sigterm_watcher;
    ev_signal_init(&sigterm_watcher, handle_sig, SIGTERM);
    ev_signal_start(default_loop, &sigterm_watcher);

    ev_loop(default_loop, 0);

    return 0;
}
