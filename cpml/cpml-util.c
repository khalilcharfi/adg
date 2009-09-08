/* CPML - Cairo Path Manipulation Library
 * Copyright (C) 2008, 2009  Nicola Fontana <ntd at entidi.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */


/**
 * SECTION:util
 * @Section_Id:utilities
 * @title: Utilities
 * @short_description: Assorted macros and functions
 *
 * Collection of macros and functions that do not fit inside any other topic.
 **/


#include "cpml-util.h"
#include <math.h>


/**
 * CPML_GNUC_CONST:
 *
 * To be appended at the end of a function to notice the compiler (gcc)
 * that the return value is constant.
 *
 * The coded is an adaptation of the #G_GNUC_CONST macro defined by
 * glib-2.18.3 in <filename>glib/gmacros.h</filename>.
 **/

/**
 * CAIRO_HAS_ARC_SUPPORT:
 *
 * Defined to 1 if cairo has arc support. Actually (cairo-1.8.8) cairo
 * does not have arc support so it is expected this define will be always
 * undefined.
 **/

/**
 * CAIRO_PATH_ARC_TO:
 *
 * The code of an arc-to primitive. This is expected to be defined by
 * cairo whenever (and if) cairo will support arc primitives. Actually
 * it resolves to an harcoded %100 constant.
 *
 * Check out the #CpmlArc section for further information.
 **/


/**
 * cpml_angle:
 * @angle: an angle in radians
 *
 * Normalizes @angle, that is returns the equivalent radians value
 * between the range %M_PI (inclusive) and %-M_PI (exclusive).
 *
 * Returns: an equivalent value in radians
 **/
double
cpml_angle(double angle)
{
    while (angle > M_PI)
        angle -= M_PI * 2;

    while (angle <= -M_PI)
        angle += M_PI * 2;

    return angle;
}
