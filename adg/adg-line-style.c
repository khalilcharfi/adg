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


/**
 * SECTION:adglinestyle
 * @title: AdgLineStyle
 * @short_description: line style related stuff
 *
 * Contains parameters on how to draw lines such as width, cap mode, join mode
 * and dash composition, if used.
 */

#include "adg-line-style.h"
#include "adg-line-style-private.h"
#include "adg-type-builtins.h"
#include "adg-intl.h"
#include "adg-util.h"

#define PARENT_CLASS ((AdgStyleClass *) adg_line_style_parent_class)


enum
{
  PROP_0,
  PROP_WIDTH,
  PROP_CAP,
  PROP_JOIN,
  PROP_MITER_LIMIT,
  PROP_ANTIALIAS,
  PROP_DASH
};


static void	get_property		(GObject	*object,
					 guint		 prop_id,
					 GValue		*value,
					 GParamSpec	*pspec);
static void	set_property		(GObject	*object,
					 guint		 prop_id,
					 const GValue	*value,
					 GParamSpec	*pspec);


G_DEFINE_TYPE (AdgLineStyle, adg_line_style, ADG_TYPE_STYLE)


static void
adg_line_style_class_init (AdgLineStyleClass *klass)
{
  GObjectClass *gobject_class;
  GParamSpec   *param;

  gobject_class = (GObjectClass *) klass;

  g_type_class_add_private (klass, sizeof (AdgLineStylePrivate));

  gobject_class->get_property = get_property;
  gobject_class->set_property = set_property;

  param = g_param_spec_double ("width",
			       P_("Line Width"),
			       P_("The line thickness in device unit"),
			       0., G_MAXDOUBLE, 2.,
			       G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_WIDTH, param);

  param = g_param_spec_int ("cap",
			    P_("Line Cap"),
			    P_("The line cap mode"),
			    G_MININT, G_MAXINT, CAIRO_LINE_CAP_ROUND,
			    G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_CAP, param);

  param = g_param_spec_int ("join",
			    P_("Line Join"),
			    P_("The line join mode"),
			    G_MININT, G_MAXINT, CAIRO_LINE_JOIN_ROUND,
			    G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_JOIN, param);

  param = g_param_spec_double ("miter-limit",
			       P_("Miter Limit"),
			       P_("Whether the lines should be joined with a bevel instead of a miter"),
			       0., G_MAXDOUBLE, 10.,
			       G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_MITER_LIMIT, param);

  param = g_param_spec_int ("antialias",
			    P_("Antialiasing Mode"),
			    P_("Type of antialiasing to do when rendering lines"),
			    G_MININT, G_MAXINT, CAIRO_ANTIALIAS_DEFAULT,
			    G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_ANTIALIAS, param);

  /* TODO: PROP_DASH (PROP_DASHES, PROP_NUM_DASHES, PROP_DASH_OFFSET) */
}

static void
adg_line_style_init (AdgLineStyle *line_style)
{
  AdgLineStylePrivate *priv = G_TYPE_INSTANCE_GET_PRIVATE (line_style,
							   ADG_TYPE_LINE_STYLE,
							   AdgLineStylePrivate);

  priv->width = ADG_NAN;
  priv->cap = CAIRO_LINE_CAP_BUTT;
  priv->join = CAIRO_LINE_JOIN_MITER;
  priv->miter_limit = ADG_NAN;
  priv->dashes = NULL;
  priv->num_dashes = 0;
  priv->dash_offset = 0.;

  line_style->priv = priv;
}

static void
get_property (GObject    *object,
	      guint       prop_id,
	      GValue     *value,
	      GParamSpec *pspec)
{
  AdgLineStyle *line_style = (AdgLineStyle *) object;

  switch (prop_id)
    {
    case PROP_WIDTH:
      g_value_set_double (value, line_style->priv->width);
      break;
    case PROP_CAP:
      g_value_set_int (value, line_style->priv->cap);
      break;
    case PROP_JOIN:
      g_value_set_int (value, line_style->priv->join);
      break;
    case PROP_MITER_LIMIT:
      g_value_set_double (value, line_style->priv->miter_limit);
      break;
    case PROP_ANTIALIAS:
      g_value_set_int (value, line_style->priv->antialias);
      break;
    case PROP_DASH:
      /* TODO */
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
set_property (GObject      *object,
	      guint         prop_id,
	      const GValue *value,
	      GParamSpec   *pspec)
{
  AdgLineStyle *line_style = (AdgLineStyle *) object;

  switch (prop_id)
    {
    case PROP_WIDTH:
      line_style->priv->width = g_value_get_double (value);
      break;
    case PROP_CAP:
      line_style->priv->cap = g_value_get_int (value);
      break;
    case PROP_JOIN:
      line_style->priv->join = g_value_get_int (value);
      break;
    case PROP_MITER_LIMIT:
      line_style->priv->miter_limit = g_value_get_double (value);
      break;
    case PROP_ANTIALIAS:
      line_style->priv->antialias = g_value_get_int (value);
      break;
    case PROP_DASH:
      /* TODO */
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


AdgStyle *
adg_line_style_from_id (AdgLineStyleId id)
{
  static AdgStyle **builtins = NULL;

  if G_UNLIKELY (builtins == NULL)
    {
      builtins = g_new (AdgStyle *, ADG_LINE_STYLE_LAST);

      builtins[ADG_LINE_STYLE_DRAW] = g_object_new (ADG_TYPE_LINE_STYLE, NULL);
      builtins[ADG_LINE_STYLE_CENTER] = g_object_new (ADG_TYPE_LINE_STYLE,
						      "g", 1.,
						      "width", 0.75,
						      NULL);
      builtins[ADG_LINE_STYLE_HIDDEN] = g_object_new (ADG_TYPE_LINE_STYLE,
						      "a", 0.5,
						      "width", 0.75,
						      NULL);
      builtins[ADG_LINE_STYLE_XATCH] = g_object_new (ADG_TYPE_LINE_STYLE,
						     "b", 1.,
						     "width", 1.25,
						     NULL);
      builtins[ADG_LINE_STYLE_DIM] = g_object_new (ADG_TYPE_LINE_STYLE,
						   "r", 1.,
						   "width", 0.75,
						   NULL);
    }

  g_return_val_if_fail (id < ADG_LINE_STYLE_LAST, NULL);
  return builtins[id];
}

void
adg_line_style_apply (const AdgLineStyle *line_style,
                      cairo_t            *cr)
{
  g_return_if_fail (line_style != NULL);
  g_return_if_fail (cr != NULL);

  if (!adg_isnan (line_style->priv->width))
    {
      double device_width = line_style->priv->width;
      cairo_device_to_user_distance (cr, &device_width, &device_width);
      cairo_set_line_width (cr, device_width);
    }

  cairo_set_line_cap (cr, line_style->priv->cap);
  cairo_set_line_join (cr, line_style->priv->join);

  if (!adg_isnan (line_style->priv->miter_limit))
    cairo_set_miter_limit (cr, line_style->priv->miter_limit);

  cairo_set_antialias (cr, line_style->priv->antialias);

  if (line_style->priv->num_dashes > 0)
    {
      g_return_if_fail (line_style->priv->dashes != NULL);

      cairo_set_dash (cr, line_style->priv->dashes, line_style->priv->num_dashes,
		      line_style->priv->dash_offset);
    }
}
