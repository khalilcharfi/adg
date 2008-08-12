/* ADG - Automatic Drawing Generation
 * Copyright (C) 2007-2008, Nicola Fontana <ntd at entidi.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */


#ifndef __ADG_PAIR_H__
#define __ADG_PAIR_H__

#include <cpml/cpml.h>
#include <glib-object.h>


G_BEGIN_DECLS

#define ADG_TYPE_PAIR  (adg_pair_get_type ())


typedef CpmlPair AdgPair;


GType           adg_pair_get_type       (void) G_GNUC_CONST;
AdgPair *       adg_pair_dup            (const AdgPair  *pair);

G_END_DECLS


#endif /* __ADG_PAIR_H__ */
