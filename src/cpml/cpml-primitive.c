/* CPML - Cairo Path Manipulation Library
 * Copyright (C) 2008,2009,2010  Nicola Fontana <ntd at entidi.it>
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
 * SECTION:cpml-primitive
 * @Section_Id:CpmlPrimitive
 * @title: CpmlPrimitive
 * @short_description: Basic component of segments
 *
 * A primitive is an atomic geometric element found inside #CpmlSegment.
 * The available primitives are the same defined by #cairo_path_data_type_t
 * with the additional #CPML_ARC type (check #CpmlPrimitiveType
 * for further information) and without #CPML_MOVE as it is not
 * considered a primitive and it is managed in different way: the move-to
 * primitives are only used to define the origin of a segment.
 **/

/**
 * CpmlPrimitiveType:
 *
 * This is another name for #cairo_path_data_type_t type. Although
 * phisically they are the same struct, #CpmlPrimitiveType conceptually
 * embodies an important difference: it can be used to specify the
 * special #CPML_ARC primitive. This is not a native cairo
 * primitive and having two different types is a good way to make clear
 * when a function expect or not embedded arc-to primitives.
 **/

/**
 * CpmlPrimitive:
 * @segment: the source #CpmlSegment
 * @org: a pointer to the first point of the primitive
 * @data: the array of the path data, prepended by the header
 *
 * As for #CpmlSegment, also the primitive is unobtrusive. This
 * means CpmlPrimitive does not include any coordinates but instead
 * keeps pointers to the original segment (and, by transition, to
 * the underlying #CpmlPath struct).
 **/

/**
 * CPML_MOVE:
 *
 * An operation that denotes a current point movement internally used to
 * keep track of the starting point of a primitive.
 * It is equivalent to the %CAIRO_PATH_MOVE_TO cairo constant.
 **/


#include "cpml-internal.h"
#include "cpml-extents.h"
#include "cpml-segment.h"
#include "cpml-primitive.h"
#include "cpml-primitive-private.h"
#include "cpml-line.h"
#include "cpml-arc.h"
#include "cpml-curve.h"
#include "cpml-close.h"
#include <string.h>
#include <stdio.h>


static const _CpmlPrimitiveClass *
                get_class_from_type     (CpmlPrimitiveType        type);
static const _CpmlPrimitiveClass *
                get_class               (const CpmlPrimitive     *primitive);
static void     dump_cairo_point        (const cairo_path_data_t *path_data);


/**
 * cpml_primitive_type_get_n_points:
 * @type: a primitive type
 *
 * Gets the number of points required to identify the @type primitive.
 *
 * Returns: the number of points or %0 on errors
 **/
size_t
cpml_primitive_type_get_n_points(CpmlPrimitiveType type)
{
    const _CpmlPrimitiveClass *class_data = get_class_from_type(type);

    if (class_data == NULL)
        return 0;

    return class_data->n_points;
}

/**
 * cpml_primitive_from_segment:
 * @primitive: the destination #CpmlPrimitive struct
 * @segment: the source segment
 *
 * Initializes @primitive to the first primitive of @segment.
 **/
void
cpml_primitive_from_segment(CpmlPrimitive *primitive, CpmlSegment *segment)
{
    primitive->segment = segment;

    /* The first element of a CpmlSegment is always a CPML_MOVE,
     * as ensured by cpml_segment_from_cairo() and by the browsing APIs,
     * so the origin is in the second data item */
    primitive->org = segment->data + 1;

    /* Also, the segment APIs ensure that @segment is prepended by
     * only one CPML_MOVE */
    primitive->data = segment->data + segment->data->header.length;
}

/**
 * cpml_primitive_copy:
 * @primitive: the destination #CpmlPrimitive
 * @src: the source #CpmlPrimitive
 *
 * Copies @src in @primitive. This is a shallow copy: the internal fields
 * of @primitive refer to the same memory as the original @src primitive.
 **/
void
cpml_primitive_copy(CpmlPrimitive *primitive, const CpmlPrimitive *src)
{
    memcpy(primitive, src, sizeof(CpmlPrimitive));
}

/**
 * cpml_primitive_reset:
 * @primitive: a #CpmlPrimitive
 *
 * Resets @primitive so it refers to the first primitive of the
 * source segment.
 **/
void
cpml_primitive_reset(CpmlPrimitive *primitive)
{
    cpml_primitive_from_segment(primitive, primitive->segment);
}

/**
 * cpml_primitive_next:
 * @primitive: a #CpmlPrimitive
 *
 *
 * Changes @primitive so it refers to the next primitive on the
 * source segment. If there are no more primitives, @primitive is
 * not changed and 0 is returned.
 *
 * Returns: 1 on success, 0 if no next primitive found or errors
 **/
cairo_bool_t
cpml_primitive_next(CpmlPrimitive *primitive)
{
    cairo_path_data_t *new_data;
    const cairo_path_data_t *end_data;

    new_data = primitive->data + primitive->data->header.length;
    end_data = primitive->segment->data + primitive->segment->num_data;

    if (new_data >= end_data)
        return 0;

    primitive->org = cpml_primitive_get_point(primitive, -1);
    primitive->data = new_data;

    return 1;
}

/**
 * cpml_primitive_get_n_points:
 * @primitive: a #CpmlPrimitive
 *
 * Gets the number of points required to identify @primitive.
 * It is similar to cpml_primitive_type_get_n_points() but using
 * a @primitive instance instead of a type.
 *
 * Returns: the number of points or %0 on errors
 **/
size_t
cpml_primitive_get_n_points(const CpmlPrimitive *primitive)
{
    return cpml_primitive_type_get_n_points(primitive->data->header.type);
}

/**
 * cpml_primitive_get_point:
 * @primitive: a #CpmlPrimitive
 * @n_point: the index of the point to retrieve
 *
 * Gets the specified @n_point from @primitive. The index starts
 * at 0: if @n_point is 0, the start point (the origin) is
 * returned, 1 for the second point and so on. If @n_point is
 * negative, it is considered as a negative index from the end,
 * so that -1 is the end point, -2 the point before the end point
 * and so on.
 *
 * #CPML_CLOSE is managed in a special way: if @n_point
 * is -1 or 1 and @primitive is a close-path, this function cycles
 * the source #CpmlSegment and returns the first point. This is
 * needed because requesting the end point (or the second point)
 * of a close path is a valid operation and must returns the start
 * of the segment.
 *
 * Returns: a pointer to the requested point (in cairo format)
 *          or %NULL if the point is outside the valid range
 **/
cairo_path_data_t *
cpml_primitive_get_point(const CpmlPrimitive *primitive, int n_point)
{
    size_t n_points;

    /* For a start point request, simply return the origin
     * without further checking */
    if (n_point == 0)
        return primitive->org;

    /* The CPML_CLOSE special case */
    if (primitive->data->header.type == CPML_CLOSE &&
        (n_point == 1 || n_point == -1))
        return &primitive->segment->data[1];

    n_points = cpml_primitive_get_n_points(primitive);
    if (n_points == 0)
        return NULL;

    /* If n_point is negative, consider it as a negative index from the end */
    if (n_point < 0)
        n_point = n_points + n_point;

    /* Out of range condition */
    if (n_point < 0 || n_point >= n_points)
        return NULL;

    return n_point == 0 ? primitive->org : &primitive->data[n_point];
}

/**
 * cpml_primitive_get_length:
 * @primitive: a #CpmlPrimitive
 *
 * Abstracts the length() family functions by providing a common
 * way to access the underlying primitive-specific implementation.
 * The function returns the length of @primitive.
 *
 * Returns: the requested length or 0 on errors
 **/
double
cpml_primitive_get_length(const CpmlPrimitive *primitive)
{
    const _CpmlPrimitiveClass *class_data = get_class(primitive);

    if (class_data == NULL || class_data->get_length == NULL)
        return 0;

    return class_data->get_length(primitive);
}

/**
 * cpml_primitive_put_extents:
 * @primitive: a #CpmlPrimitive
 * @extents: where to store the extents
 *
 * Abstracts the extents() family functions by providing a common
 * way to access the underlying primitive-specific implementation.
 *
 * This function stores in @extents the bounding box of @primitive.
 *
 * On errors, that is if the extents cannot be calculated for some
 * reason, this function does nothing.
 **/
void
cpml_primitive_put_extents(const CpmlPrimitive *primitive,
                           CpmlExtents *extents)
{
    const _CpmlPrimitiveClass *class_data = get_class(primitive);

    if (class_data == NULL || class_data->put_extents == NULL)
        return;

    class_data->put_extents(primitive, extents);
}

/**
 * cpml_primitive_put_pair_at:
 * @primitive: a #CpmlPrimitive
 * @pos:       the position value
 * @pair:      the destination #CpmlPair
 *
 * Abstracts the put_pair_at() family functions by providing a common
 * way to access the underlying primitive-specific implementation.
 *
 * It gets the coordinates of the point lying on @primitive
 * at position @pos. @pos is an homogeneous factor where 0 is the
 * start point, 1 the end point, 0.5 the mid point and so on.
 * @pos can be less than 0 or greater than %1, in which case the
 * coordinates of @pair are interpolated.
 *
 * On errors, that is if the coordinates cannot be calculated for
 * some reason, this function does nothing.
 **/
void
cpml_primitive_put_pair_at(const CpmlPrimitive *primitive, double pos,
                           CpmlPair *pair)
{
    const _CpmlPrimitiveClass *class_data = get_class(primitive);

    if (class_data == NULL || class_data->put_pair_at == NULL)
        return;

    class_data->put_pair_at(primitive, pos, pair);
}

/**
 * cpml_primitive_put_vector_at:
 * @primitive: a #CpmlPrimitive
 * @pos:       the position value
 * @vector:    the destination #CpmlVector
 *
 * Abstracts the put_vector_at() family functions by providing a common
 * way to access the underlying primitive-specific implementation.
 *
 * It gets the steepness of the point at position @pos on @primitive.
 * @pos is an homogeneous factor where 0 is the start point, 1 the
 * end point, 0.5 the mid point and so on.
 * @pos can be less than 0 or greater than %1, in which case the
 * coordinates of @pair are interpolated.
 *
 * On errors, that is if the steepness cannot be calculated for
 * some reason, this function does nothing.
 **/
void
cpml_primitive_put_vector_at(const CpmlPrimitive *primitive, double pos,
                             CpmlVector *vector)
{
    const _CpmlPrimitiveClass *class_data = get_class(primitive);

    if (class_data == NULL || class_data->put_vector_at == NULL)
        return;

    class_data->put_vector_at(primitive, pos, vector);
}

/**
 * cpml_primitive_get_closest_pos:
 * @primitive: a #CpmlPrimitive
 * @pair:      the coordinates of the subject point
 *
 * Returns the pos value of the point on @primitive nearest to @pair.
 * The returned value is always clamped between %0 and %1.
 *
 * Returns: the requested pos value between %0 and %1, or %-1 on errors
 **/
double
cpml_primitive_get_closest_pos(const CpmlPrimitive *primitive,
                               const CpmlPair *pair)
{
    const _CpmlPrimitiveClass *class_data = get_class(primitive);

    if (class_data == NULL || class_data->get_closest_pos == NULL)
        return -1;

    return class_data->get_closest_pos(primitive, pair);
}

/**
 * cpml_primitive_put_intersections:
 * @primitive:  the first #CpmlPrimitive
 * @primitive2: the second #CpmlPrimitive
 * @n_dest:     maximum number of intersections to return
 * @dest:       the destination buffer that can contain @n_dest #CpmlPair
 *
 * Finds the intersection points between the given primitives and
 * returns the result in @dest. The size of @dest should be enough
 * to store @n_dest #CpmlPair. The maximum number of intersections
 * is dependent on the type of the primitive involved in the
 * operation. If there are at least one Bézier curve involved, up to
 * %4 intersections could be returned. Otherwise, if there is an arc
 * the intersections will be %2 at maximum. For line primitives, there
 * is only %1 point (or %0 if the lines are parallel).
 *
 * <note>
 * <para>
 * The convention used by the CPML library is that a primitive should
 * implement only the intersection algorithms with lower degree
 * primitives. This is required to avoid code duplication: intersection
 * between arc and Bézier curves must be implemented by #CPML_CURVE and
 * intersection between lines and arcs must be implemented by #CPML_ARC.
 * cpml_primitive_put_intersections() will take care of swapping the
 * arguments if they are not properly ordered.
 * </para>
 * </note>
 *
 * Returns: the number of intersection points found or 0 if the
 *          primitives do not intersect or on errors
 **/
size_t
cpml_primitive_put_intersections(const CpmlPrimitive *primitive,
                                 const CpmlPrimitive *primitive2,
                                 size_t n_dest, CpmlPair *dest)
{
    const _CpmlPrimitiveClass *class_data;
    size_t n_points, n_points2;

    class_data = get_class(primitive);

    if (class_data == NULL || class_data->put_intersections == NULL)
        return 0;

    n_points = cpml_primitive_get_n_points(primitive);
    n_points2 = cpml_primitive_get_n_points(primitive2);

    if (n_points == 0 || n_points2 == 0)
        return 0;

    /* Primitives reordering: the first must be the more complex one */
    if (n_points < n_points2) {
        const CpmlPrimitive *old_primitive2 = primitive2;
        primitive2 = primitive;
        primitive = old_primitive2;
    }

    return class_data->put_intersections(primitive, primitive2, n_dest, dest);
}

/**
 * cpml_primitive_put_intersections_with_segment:
 * @primitive: a #CpmlPrimitive
 * @segment:   a #CpmlSegment
 * @n_dest:    maximum number of intersection pairs to return
 * @dest:      the destination buffer of #CpmlPair
 *
 * Computes the intersections between @segment and @primitive by
 * sequentially scanning the primitives in @segment and looking
 * for their intersections with @primitive.
 *
 * If the intersections are more than @n_dest, only the first
 * @n_dest pairs are stored.
 *
 * Returns: the number of intersections found
 **/
size_t
cpml_primitive_put_intersections_with_segment(const CpmlPrimitive *primitive,
                                              const CpmlSegment *segment,
                                              size_t n_dest, CpmlPair *dest)
{
    CpmlPrimitive portion;
    size_t found;

    cpml_primitive_from_segment(&portion, (CpmlSegment *) segment);
    found = 0;

    while (found < n_dest) {
        found += cpml_primitive_put_intersections(&portion, primitive,
                                                  n_dest-found, dest+found);
        if (!cpml_primitive_next(&portion))
            break;
    }

    return found;
}

/**
 * cpml_primitive_offset:
 * @primitive: a #CpmlPrimitive
 * @offset: distance for the computed offset primitive
 *
 * Given a primitive, computes the same (or approximated) parallel
 * primitive distant @offset from the original one and returns
 * the result by changing @primitive.
 *
 * On errors, that is if the offset primitive cannot be calculated
 * for some reason, this function does nothing.
 **/
void
cpml_primitive_offset(CpmlPrimitive *primitive, double offset)
{
    const _CpmlPrimitiveClass *class_data = get_class(primitive);

    if (class_data == NULL || class_data->offset == NULL)
        return;

    return class_data->offset(primitive, offset);
}

/**
 * cpml_primitive_join:
 * @primitive: the first #CpmlPrimitive
 * @primitive2: the second #CpmlPrimitive
 *
 * Joins two primitive modifying the end point of @primitive and the
 * start point of @primitive2 so that the resulting points will overlap.
 *
 * <important>
 * <title>TODO</title>
 * <itemizedlist>
 * <listitem>Actually, the join is done by extending the end vector
 *           of @primitive and the start vector of @primitive2 and
 *           interpolating the intersection: this means no primitive
 *           dependent code is needed. Anyway, it is likely to change
 *           in the future because this approach is quite naive when
 *           curves are involved.</listitem>
 * </itemizedlist>
 * </important>
 *
 * Returns: 1 on success, 0 if the end vector of @primitive
 *          and the start vector of @primitive2 are parallel
 **/
cairo_bool_t
cpml_primitive_join(CpmlPrimitive *primitive, CpmlPrimitive *primitive2)
{
    cairo_path_data_t *end1, *start2;
    CpmlPrimitive line1, line2;
    cairo_path_data_t data1[2], data2[2];
    CpmlPair joint;

    end1 = cpml_primitive_get_point(primitive, -1);
    start2 = cpml_primitive_get_point(primitive2, 0);

    /* Check if the primitives are yet connected */
    if (end1->point.x == start2->point.x && end1->point.y == start2->point.y)
        return 1;

    line1.org = cpml_primitive_get_point(primitive, -2);
    line1.data = data1;
    data1[0].header.type = CPML_LINE;
    data1[1] = *end1;

    line2.org = start2;
    line2.data = data2;
    data2[0].header.type = CPML_LINE;
    data2[1] = *cpml_primitive_get_point(primitive2, 1);

    if (!cpml_primitive_put_intersections(&line1, &line2, 1, &joint))
        return 0;

    cpml_pair_to_cairo(&joint, end1);
    cpml_pair_to_cairo(&joint, start2);

    return 1;
}

/**
 * cpml_primitive_to_cairo:
 * @primitive: a #CpmlPrimitive
 * @cr: the destination cairo context
 *
 * Renders a single @primitive to the @cr cairo context.
 * As a special case, if the primitive is a #CPML_CLOSE, an
 * equivalent line is rendered, because a close path left alone
 * is not renderable.
 *
 * Also a #CPML_ARC primitive is treated specially, as it is not
 * natively supported by cairo and has its own rendering API.
 **/
void
cpml_primitive_to_cairo(const CpmlPrimitive *primitive, cairo_t *cr)
{
    cairo_path_t path;
    cairo_path_data_t *path_data;

    cairo_move_to(cr, primitive->org->point.x, primitive->org->point.y);

    switch (primitive->data->header.type) {

    case CPML_CLOSE:
        path_data = cpml_primitive_get_point(primitive, -1);
        cairo_line_to(cr, path_data->point.x, path_data->point.y);
        break;

    case CPML_ARC:
        cpml_arc_to_cairo(primitive, cr);
        break;

    default:
        path.status = CAIRO_STATUS_SUCCESS;
        path.data = primitive->data;
        path.num_data = primitive->data->header.length;
        cairo_append_path(cr, &path);
        break;
    }
}

/**
 * cpml_primitive_dump:
 * @primitive: a #CpmlPrimitive
 * @org_also:  whether to output also the origin coordinates
 *
 * Dumps info on the specified @primitive to stdout: useful for
 * debugging purposes. If @org_also is 1, a #CPML_MOVE to the
 * origin is prepended to the data otherwise the
 * <structfield>org</structfield> field is not used.
 **/
void
cpml_primitive_dump(const CpmlPrimitive *primitive, cairo_bool_t org_also)
{
    const cairo_path_data_t *data;
    int type;
    const _CpmlPrimitiveClass *class_data;
    size_t n, n_points;

    data = primitive->data;
    type = data->header.type;
    class_data = get_class_from_type(type);

    if (class_data == NULL) {
        printf("Unknown primitive type (%d)\n", type);
        return;
    }

    /* Dump the origin, if requested */
    if (org_also) {
        printf("move to ");
        dump_cairo_point(primitive->org);
        printf("\n");
    }

    printf("%s ", class_data->name);

    n_points = cpml_primitive_get_n_points(primitive);
    for (n = 1; n < n_points; ++n)
        dump_cairo_point(cpml_primitive_get_point(primitive, n));

    printf("\n");
}


static const _CpmlPrimitiveClass *
get_class_from_type(CpmlPrimitiveType type)
{
    switch (type) {
    case CPML_LINE:
        return _cpml_line_get_class();
    case CPML_ARC:
        return _cpml_arc_get_class();
    case CPML_CURVE:
        return _cpml_curve_get_class();
    case CPML_CLOSE:
        return _cpml_close_get_class();
    default:
        break;
    }

    return NULL;
}

static const _CpmlPrimitiveClass *
get_class(const CpmlPrimitive *primitive)
{
    return get_class_from_type(primitive->data->header.type);
}

static void
dump_cairo_point(const cairo_path_data_t *path_data)
{
    printf("(%g %g) ", path_data->point.x, path_data->point.y);
}