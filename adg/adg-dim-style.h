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


#ifndef __ADG_DIM_STYLE_H__
#define __ADG_DIM_STYLE_H__

#include <adg/adg-style.h>
#include <adg/adg-font-style.h>
#include <adg/adg-line-style.h>
#include <adg/adg-arrow-style.h>
#include <adg/adg-pair.h>
#include <adg/adg-enums.h>


G_BEGIN_DECLS

#define ADG_TYPE_DIM_STYLE             (adg_dim_style_get_type())
#define ADG_DIM_STYLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), ADG_TYPE_DIM_STYLE, AdgDimStyle))
#define ADG_DIM_STYLE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), ADG_TYPE_DIM_STYLE, AdgDimStyleClass))
#define ADG_IS_DIM_STYLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), ADG_TYPE_DIM_STYLE))
#define ADG_IS_DIM_STYLE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), ADG_TYPE_DIM_STYLE))
#define ADG_DIM_STYLE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), ADG_TYPE_DIM_STYLE, AdgDimStyleClass))
#define ADG_SLOT_DIM_STYLE             (_adg_dim_style_get_slot())

typedef struct _AdgDimStyle        AdgDimStyle;
typedef struct _AdgDimStyleClass   AdgDimStyleClass;

struct _AdgDimStyle {
    /*< private >*/
    AdgStyle             parent;
    gpointer             data;
};

struct _AdgDimStyleClass {
    /*< private >*/
    AdgStyleClass        parent_class;
};


GType           adg_dim_style_get_type          (void) G_GNUC_CONST;
AdgStyleSlot    _adg_dim_style_get_slot         (void) G_GNUC_CONST;
AdgStyle *      adg_dim_style_new               (void);

AdgStyle *      adg_dim_style_get_value_style   (AdgDimStyle    *dim_style);
void            adg_dim_style_set_value_style   (AdgDimStyle    *dim_style,
                                                 AdgFontStyle   *style);
AdgStyle *      adg_dim_style_get_tolerance_style
                                                (AdgDimStyle    *dim_style);
void            adg_dim_style_set_tolerance_style
                                                (AdgDimStyle    *dim_style,
                                                 AdgFontStyle   *style);
AdgStyle *      adg_dim_style_get_note_style    (AdgDimStyle    *dim_style);
void            adg_dim_style_set_note_style    (AdgDimStyle    *dim_style,
                                                 AdgFontStyle   *style);
AdgStyle *      adg_dim_style_get_line_style    (AdgDimStyle    *dim_style);
void            adg_dim_style_set_line_style    (AdgDimStyle    *dim_style,
                                                 AdgLineStyle   *style);
AdgStyle *      adg_dim_style_get_arrow_style   (AdgDimStyle    *dim_style);
void            adg_dim_style_set_arrow_style   (AdgDimStyle    *dim_style,
                                                 AdgArrowStyle  *style);
gdouble         adg_dim_style_get_from_offset   (AdgDimStyle    *dim_style);
void            adg_dim_style_set_from_offset   (AdgDimStyle    *dim_style,
                                                 gdouble         offset);
gdouble         adg_dim_style_get_to_offset     (AdgDimStyle    *dim_style);
void            adg_dim_style_set_to_offset     (AdgDimStyle    *dim_style,
                                                 gdouble         offset);
gdouble         adg_dim_style_beyond            (AdgDimStyle    *dim_style);
gdouble         adg_dim_style_get_beyond        (AdgDimStyle    *dim_style);
void            adg_dim_style_set_beyond        (AdgDimStyle    *dim_style,
                                                 gdouble         length);
gdouble         adg_dim_style_get_baseline_spacing
                                                (AdgDimStyle    *dim_style);
void            adg_dim_style_set_baseline_spacing
                                                (AdgDimStyle    *dim_style,
                                                 gdouble         spacing);
gdouble         adg_dim_style_get_tolerance_spacing
                                                (AdgDimStyle    *dim_style);
void            adg_dim_style_set_tolerance_spacing
                                                (AdgDimStyle    *dim_style,
                                                 gdouble         spacing);
const AdgPair * adg_dim_style_get_quote_shift   (AdgDimStyle    *dim_style);
void            adg_dim_style_set_quote_shift   (AdgDimStyle    *dim_style,
                                                 const AdgPair  *shift);
const AdgPair * adg_dim_style_get_tolerance_shift
                                                (AdgDimStyle    *dim_style);
void            adg_dim_style_set_tolerance_shift
                                                (AdgDimStyle    *dim_style,
                                                 const AdgPair  *shift);
const AdgPair * adg_dim_style_get_note_shift    (AdgDimStyle    *dim_style);
void            adg_dim_style_set_note_shift    (AdgDimStyle    *dim_style,
                                                 const AdgPair  *shift);
const gchar *   adg_dim_style_get_number_format (AdgDimStyle    *dim_style);
void            adg_dim_style_set_number_format (AdgDimStyle    *dim_style,
                                                 const gchar    *format);
const gchar *   adg_dim_style_get_number_tag    (AdgDimStyle    *dim_style);
void            adg_dim_style_set_number_tag    (AdgDimStyle    *dim_style,
                                                 const gchar    *tag);

G_END_DECLS


#endif /* __ADG_DIM_STYLE_H__ */
