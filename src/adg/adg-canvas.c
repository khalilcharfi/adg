/* ADG - Automatic Drawing Generation
 * Copyright (C) 2007,2008,2009,2010  Nicola Fontana <ntd at entidi.it>
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
 * SECTION:adg-canvas
 * @short_description: The drawing container
 *
 * This object is the toplevel entity of an ADG drawing. It can be
 * bound to a GTK+ widget, such as #AdgGtkArea, or manually rendered
 * to a custom surface.
 **/

/**
 * AdgCanvas:
 *
 * All fields are private and should not be used directly.
 * Use its public methods instead.
 **/


#include "adg-internal.h"
#include "adg-canvas.h"
#include "adg-canvas-private.h"
#include "adg-dress-builtins.h"
#include "adg-color-style.h"

#define PARENT_ENTITY_CLASS  ((AdgEntityClass *) adg_canvas_parent_class)

enum {
    PROP_0,
    PROP_BACKGROUND_DRESS,
    PROP_TOP_MARGIN,
    PROP_RIGHT_MARGIN,
    PROP_BOTTOM_MARGIN,
    PROP_LEFT_MARGIN
};


static void             _adg_get_property       (GObject        *object,
                                                 guint           param_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec);
static void             _adg_set_property       (GObject        *object,
                                                 guint           param_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec);
static void             _adg_arrange            (AdgEntity      *entity);
static void             _adg_render             (AdgEntity      *entity,
                                                 cairo_t        *cr);


G_DEFINE_TYPE(AdgCanvas, adg_canvas, ADG_TYPE_CONTAINER);


static void
adg_canvas_class_init(AdgCanvasClass *klass)
{
    GObjectClass *gobject_class;
    AdgEntityClass *entity_class;
    GParamSpec *param;

    gobject_class = (GObjectClass *) klass;
    entity_class = (AdgEntityClass *) klass;

    g_type_class_add_private(klass, sizeof(AdgCanvasPrivate));

    gobject_class->get_property = _adg_get_property;
    gobject_class->set_property = _adg_set_property;

    entity_class->arrange = _adg_arrange;
    entity_class->render = _adg_render;

    param = adg_param_spec_dress("background-dress",
                                 P_("Background Dress"),
                                 P_("The color dress to use for the canvas background"),
                                 ADG_DRESS_COLOR_BACKGROUND,
                                 G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_BACKGROUND_DRESS, param);

    param = g_param_spec_double("top-margin",
                                P_("Top Margin"),
                                P_("The margin (in identity space) to leave empty above the drawing"),
                                G_MINDOUBLE, G_MAXDOUBLE, 15,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_TOP_MARGIN, param);

    param = g_param_spec_double("right-margin",
                                P_("Right Margin"),
                                P_("The margin (in identity space) to leave empty at the right of the drawing"),
                                G_MINDOUBLE, G_MAXDOUBLE, 15,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_RIGHT_MARGIN, param);

    param = g_param_spec_double("bottom-margin",
                                P_("Bottom Margin"),
                                P_("The margin (in identity space) to leave empty below the drawing"),
                                G_MINDOUBLE, G_MAXDOUBLE, 15,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_BOTTOM_MARGIN, param);

    param = g_param_spec_double("left-margin",
                                P_("Left Margin"),
                                P_("The margin (in identity space) to leave empty at the left of the drawing"),
                                G_MINDOUBLE, G_MAXDOUBLE, 15,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_LEFT_MARGIN, param);
}

static void
adg_canvas_init(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data = G_TYPE_INSTANCE_GET_PRIVATE(canvas,
                                                         ADG_TYPE_CANVAS,
                                                         AdgCanvasPrivate);

    data->background_dress = ADG_DRESS_COLOR_BACKGROUND;
    data->top_margin = 15;
    data->right_margin = 15;
    data->bottom_margin = 15;
    data->left_margin = 15;

    canvas->data = data;
}

static void
_adg_get_property(GObject *object, guint prop_id,
                  GValue *value, GParamSpec *pspec)
{
    AdgCanvasPrivate *data = ((AdgCanvas *) object)->data;

    switch (prop_id) {
    case PROP_BACKGROUND_DRESS:
        g_value_set_int(value, data->background_dress);
        break;
    case PROP_TOP_MARGIN:
        g_value_set_double(value, data->top_margin);
        break;
    case PROP_RIGHT_MARGIN:
        g_value_set_double(value, data->right_margin);
        break;
    case PROP_BOTTOM_MARGIN:
        g_value_set_double(value, data->bottom_margin);
        break;
    case PROP_LEFT_MARGIN:
        g_value_set_double(value, data->left_margin);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
_adg_set_property(GObject *object, guint prop_id,
                  const GValue *value, GParamSpec *pspec)
{
    AdgCanvas *canvas;
    AdgCanvasPrivate *data;

    canvas = (AdgCanvas *) object;
    data = canvas->data;

    switch (prop_id) {
    case PROP_BACKGROUND_DRESS:
        data->background_dress = g_value_get_int(value);
        break;
    case PROP_TOP_MARGIN:
        data->top_margin = g_value_get_double(value);
        break;
    case PROP_RIGHT_MARGIN:
        data->right_margin = g_value_get_double(value);
        break;
    case PROP_BOTTOM_MARGIN:
        data->bottom_margin = g_value_get_double(value);
        break;
    case PROP_LEFT_MARGIN:
        data->left_margin = g_value_get_double(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}


/**
 * adg_canvas_new:
 *
 * Creates a new empty canvas object.
 *
 * Returns: the canvas
 **/
AdgCanvas *
adg_canvas_new(void)
{
    return g_object_new(ADG_TYPE_CANVAS, NULL);
}

/**
 * adg_canvas_set_background_dress:
 * @canvas: an #AdgCanvas
 * @dress: the new #AdgDress to use
 *
 * Sets a new background dress for rendering @canvas: the new
 * dress must be a color dress.
 **/
void
adg_canvas_set_background_dress(AdgCanvas *canvas, AdgDress dress)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas, "background-dress", dress, NULL);
}

/**
 * adg_canvas_get_background_dress:
 * @canvas: an #AdgCanvas
 *
 * Gets the background dress to be used in rendering @canvas.
 *
 * Returns: the current background dress
 **/
AdgDress
adg_canvas_get_background_dress(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data;

    g_return_val_if_fail(ADG_IS_CANVAS(canvas), ADG_DRESS_UNDEFINED);

    data = canvas->data;

    return data->background_dress;
}

/**
 * adg_canvas_set_top_margin:
 * @canvas: an #AdgCanvas
 * @value: the new margin, in identity space
 *
 * Changes the top margin of @canvas by setting #AdgCanvas:top-margin
 * to @value. Negative values are allowed.
 **/
void
adg_canvas_set_top_margin(AdgCanvas *canvas, gdouble value)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas, "top-margin", value, NULL);
}

/**
 * adg_canvas_get_top_margin:
 * @canvas: an #AdgCanvas
 *
 * Gets the top margin (in identity space) of @canvas.
 *
 * Returns: the requested margin or %0 on error
 **/
gdouble
adg_canvas_get_top_margin(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data;

    g_return_val_if_fail(ADG_IS_CANVAS(canvas), 0.);

    data = canvas->data;
    return data->top_margin;
}

/**
 * adg_canvas_set_right_margin:
 * @canvas: an #AdgCanvas
 * @value: the new margin, in identity space
 *
 * Changes the right margin of @canvas by setting #AdgCanvas:right-margin
 * to @value. Negative values are allowed.
 **/
void
adg_canvas_set_right_margin(AdgCanvas *canvas, gdouble value)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas, "right-margin", value, NULL);
}

/**
 * adg_canvas_get_right_margin:
 * @canvas: an #AdgCanvas
 *
 * Gets the right margin (in identity space) of @canvas.
 *
 * Returns: the requested margin or %0 on error
 **/
gdouble
adg_canvas_get_right_margin(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data;

    g_return_val_if_fail(ADG_IS_CANVAS(canvas), 0.);

    data = canvas->data;
    return data->right_margin;
}


/**
 * adg_canvas_set_bottom_margin:
 * @canvas: an #AdgCanvas
 * @value: the new margin, in identity space
 *
 * Changes the bottom margin of @canvas by setting #AdgCanvas:bottom-margin
 * to @value. Negative values are allowed.
 **/
void
adg_canvas_set_bottom_margin(AdgCanvas *canvas, gdouble value)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas, "bottom-margin", value, NULL);
}

/**
 * adg_canvas_get_bottom_margin:
 * @canvas: an #AdgCanvas
 *
 * Gets the bottom margin (in identity space) of @canvas.
 *
 * Returns: the requested margin or %0 on error
 **/
gdouble
adg_canvas_get_bottom_margin(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data;

    g_return_val_if_fail(ADG_IS_CANVAS(canvas), 0.);

    data = canvas->data;
    return data->bottom_margin;
}

/**
 * adg_canvas_set_left_margin:
 * @canvas: an #AdgCanvas
 * @value: the new margin, in identity space
 *
 * Changes the left margin of @canvas by setting #AdgCanvas:left-margin
 * to @value. Negative values are allowed.
 **/
void
adg_canvas_set_left_margin(AdgCanvas *canvas, gdouble value)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas, "left-margin", value, NULL);
}

/**
 * adg_canvas_get_left_margin:
 * @canvas: an #AdgCanvas
 *
 * Gets the left margin (in identity space) of @canvas.
 *
 * Returns: the requested margin or %0 on error
 **/
gdouble
adg_canvas_get_left_margin(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data;

    g_return_val_if_fail(ADG_IS_CANVAS(canvas), 0.);

    data = canvas->data;
    return data->left_margin;
}


/**
 * adg_canvas_set_margins:
 * @canvas: an #AdgCanvas
 * @top: top margin, in identity space
 * @right: right margin, in identity space
 * @bottom: bottom margin, in identity space
 * @left: left margin, in identity space
 *
 * Convenient function to set all the margins at once.
 **/
void
adg_canvas_set_margins(AdgCanvas *canvas, gdouble top, gdouble right,
                          gdouble bottom, gdouble left)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas,
                 "top-margin", top,
                 "right-margin", right,
                 "bottom-margin", bottom,
                 "left-margin", left,
                 NULL);
}


static void
_adg_arrange(AdgEntity *entity)
{
    CpmlExtents extents;

    if (PARENT_ENTITY_CLASS->arrange)
        PARENT_ENTITY_CLASS->arrange(entity);

    cpml_extents_copy(&extents, adg_entity_get_extents(entity));

    if (extents.is_defined) {
        AdgCanvasPrivate *data = ((AdgCanvas *) entity)->data;

        extents.org.x -= data->left_margin;
        extents.org.y -= data->top_margin;
        extents.size.x += data->left_margin + data->right_margin;
        extents.size.y += data->top_margin + data->bottom_margin;

        adg_entity_set_extents(entity, &extents);
    }
}

static void
_adg_render(AdgEntity *entity, cairo_t *cr)
{
    AdgCanvasPrivate *data = ((AdgCanvas *) entity)->data;

    adg_entity_apply_dress(entity, data->background_dress, cr);
    cairo_paint(cr);

    if (PARENT_ENTITY_CLASS->render)
        PARENT_ENTITY_CLASS->render(entity, cr);
}
