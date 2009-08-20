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


#ifndef __ADG_UTIL_H__
#define __ADG_UTIL_H__

#include <glib.h>
#include <cairo.h>
#include <math.h>

#define ADG_DIR_RIGHT           0.
#define ADG_DIR_DOWN            M_PI_2
#define ADG_DIR_LEFT            M_PI
#define ADG_DIR_UP              (M_PI_2 * 3.)

#define ADG_FORWARD_DECL(id)    typedef struct _##id id
#define ADG_ISSET(flags,mask)   (((flags) & (mask)) != 0 ? TRUE : FALSE)
#define ADG_SET(flags,mask)     G_STMT_START{ (flags) |= (mask); }G_STMT_END
#define ADG_UNSET(flags,mask)   G_STMT_START{ (flags) &= ~(mask); }G_STMT_END


#define ADG_CHECKPOINT_WITH_MESSAGE(message) \
                                g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, \
                                       "file `%s' at line %d: %s", \
                                       __FILE__, __LINE__, #message)
#define ADG_CHECKPOINT()        ADG_CHECKPOINT_WITH_MESSAGE ("check point")

#define ADG_STUB()              ADG_CHECKPOINT_WITH_MESSAGE("stub")


G_BEGIN_DECLS

int             adg_strcmp              (const char     *s1,
                                         const gchar    *s2);

G_END_DECLS


#endif /* __ADG_UTIL_H__ */
