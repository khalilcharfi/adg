/* ADG - Automatic Drawing Generation
 * Copyright (C) 2007,2008,2009 Nicola Fontana <ntd at entidi.it>
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


#ifndef __ADG_ENUMS_H__
#define __ADG_ENUMS_H__

#include <glib-object.h>


G_BEGIN_DECLS

typedef enum {
    ADG_THREE_STATE_OFF,
    ADG_THREE_STATE_ON,
    ADG_THREE_STATE_UNKNOWN
} AdgThreeState;

typedef enum {
    ADG_TRANSFORM_NONE,
    ADG_TRANSFORM_BEFORE,
    ADG_TRANSFORM_AFTER,
    ADG_TRANSFORM_BEFORE_NORMALIZED,
    ADG_TRANSFORM_AFTER_NORMALIZED
} AdgTransformMode;

G_END_DECLS


#endif                          /* __ADG_ENUMS_H__ */
