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
 * @addtogroup compositor "Compositor"
 *
 * @{
 */

#ifndef __WS_OBJECTS_GL_PROGRAM_H__
#define __WS_OBJECTS_GL_PROGRAM_H__

#include "GLES2/gl2.h"

#include "objects/object.h"

/**
 * An object representing a GL Program.
 *
 * It stores the necessary ids of a given program and is used in the rendering
 * process.
 *
 * Different opengl shaders enable users to change a clients appearance, truly
 * being in power of their display.
 *
 */
struct ws_gl_program {
    struct ws_object obj; //!< @private the super type
    GLuint vertex_shader; //!< @private the vertex shader id
    GLuint fragment_shader; //!< @private the fragment shader id
    GLuint program; //!< @private the program id
    char* vertex_shader_path; //!< @private the possible vertex shader path
    char* fragment_shader_path; //!< @private the possible fragment shader path
    bool has_linked; //!< @private a boolean flag to check linking status
};

/**
 * Variable which holds type information about the wl_surface type
 */
extern ws_object_type_id WS_OBJECT_TYPE_ID_GL_PROGRAM;


/**
 * Create a new ws_gl_program with the given paths as sources
 *
 * @notice Be sure to check whether the given program has actually linked
 * or you will be met with a black screen on using it.
 *
 * @returns the new ws_gl_program or NULL on failure
 */
struct ws_gl_program*
ws_gl_program_new_from_files(
    char* vertex, //!< The source file for the vertex shader
    char* fragment //!< The source file for the fragment shader
);

/**
 * Inits the given ws_gl_program, creating the glProgram and trying to link it
 *
 * @returns 0 on success, a negative errno.h otherwise
 */
int
ws_gl_program_init(
    struct ws_gl_program* program //!< The program to initialize
);

/**
 * Check whether a given program has succesfully linked or not, and return
 * any errors. When given a NULL value the relevant status is not checked
 *
 * @notice If this function returns true the variables are not modified.
 * @notice The caller is reponsible for supplying enough space for the error
 *         message to be written to.
 *
 * @returns true if a program has linked correctly, false otherwise
 */
bool
ws_gl_program_check_linking(
    struct ws_gl_program* program, //!< The program to check
    char* vertex_error, //!< Set if the vertex shader has an error
    char* fragment_error, //!< Set if the fragment shader has an error
    char* program_error //!< Set if the program linking has an error
);

#endif // __WS_OBJECTS_GL_PROGRAM_H__

/**
 * @}
 */

