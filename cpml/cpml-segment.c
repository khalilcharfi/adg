/* CPML - Cairo Path Manipulation Library
 * Copyright (C) 2008, Nicola Fontana <ntd at entidi.it>
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


#include "cpml-segment.h"
#include "cpml-pair.h"
#include "cpml-macros.h"
#include "cpml-alloca.h"

#include <stdio.h>
#include <string.h>

static cairo_bool_t     segment_normalize       (CpmlSegment       *segment);
static void             line_offset             (CpmlPair          *p,
                                                 CpmlVector        *vector,
                                                 double             offset);
static void             curve_offset            (CpmlPair          *p,
                                                 CpmlVector        *vector,
                                                 double             offset);
static void             join_primitives         (cairo_path_data_t *last_data,
                                                 const CpmlVector  *last_vector,
                                                 const CpmlPair    *new_point,
                                                 const CpmlVector  *new_vector);


/**
 * cpml_segment_init:
 * @segment: a #CpmlSegment
 * @src: the source cairo_path_t
 *
 * Builds a CpmlSegment from a cairo_path_t structure. This operation
 * involves stripping the leading %MOVE_TO primitives and setting the
 * internal segment structure accordling. A pointer to the source
 * path segment is kept.
 *
 * Return value: 1 on success, 0 on errors
 **/
cairo_bool_t
cpml_segment_init(CpmlSegment *segment, cairo_path_t *src)
{
    /* The cairo path should be defined and in perfect state */
    if (src == NULL || src->num_data == 0 ||
        src->status != CAIRO_STATUS_SUCCESS)
        return 0;

    segment->original = src;
    memcpy(&segment->path, src, sizeof(cairo_path_t));

    return segment_normalize(segment);
}

/**
 * cpml_segment_copy:
 * @segment: a #CpmlSegment
 * @src: the source segment to copy
 *
 * Makes a shallow copy of @src into @segment.
 *
 * Return value: @segment or %NULL on errors
 **/
CpmlSegment *
cpml_segment_copy(CpmlSegment *segment, const CpmlSegment *src)
{
    if (segment == NULL || src == NULL)
        return NULL;

    return memcpy(segment, src, sizeof(CpmlSegment));
}

/**
 * cpml_segment_dump:
 * @segment: a #CpmlSegment
 *
 * Dumps the specified @segment to stdout. Useful for debugging purposes.
 **/
void
cpml_segment_dump(const CpmlSegment *segment)
{
    const cairo_path_t *path;
    cairo_path_data_t *data;
    int n_data, n_point;

    if (segment == NULL) {
        printf("Trying to dump a NULL segment!\n");
        return;
    }

    path = &segment->path;
    for (n_data = 0; n_data < path->num_data; ++n_data) {
	data = path->data + n_data;

	switch (data->header.type) {
	case CAIRO_PATH_MOVE_TO:
	    printf("Move to ");
	    break;
	case CAIRO_PATH_LINE_TO:
	    printf("Line to ");
	    break;
	case CAIRO_PATH_CURVE_TO:
	    printf("Curve to ");
	    break;
	case CAIRO_PATH_CLOSE_PATH:
	    printf("Path close");
	    break;
	default:
	    printf("Unknown entity (%d)", data->header.type);
	    break;
	}

	for (n_point = 1; n_point < data->header.length; ++n_point)
	    printf("(%lf, %lf) ", data[n_point].point.x,
                   data[n_point].point.y);

	n_data += n_point - 1;
	printf("\n");
    }
}

/**
 * cpml_segment_reset:
 * @segment: a #CpmlSegment
 *
 * Modifies @segment to point to the first segment of the original path.
 **/
void
cpml_segment_reset(CpmlSegment *segment)
{
    memcpy(&segment->path, segment->original, sizeof(cairo_path_t));
    segment_normalize(segment);
}

/**
 * cpml_segment_next:
 * @segment: a #CpmlSegment
 *
 * Modifies @segment to point to the next segment of the original path.
 *
 * Return value: 1 on success, 0 if no next segment found or errors
 **/
cairo_bool_t
cpml_segment_next(CpmlSegment *segment)
{
    int num_data = segment->path.num_data;
    int offset = segment->path.data - segment->original->data;

    segment->path.num_data = segment->original->num_data - num_data - offset;
    segment->path.data += num_data;

    return segment_normalize(segment);
}

/**
 * cpml_segment_reverse:
 * @segment: a #CpmlSegment
 *
 * Reverses @segment in-place. The resulting rendering will be the same,
 * but with the primitives generated in reverse order.
 **/
void
cpml_segment_reverse(CpmlSegment *segment)
{
    cairo_path_data_t *data, *dst_data;
    size_t data_size;
    double end_x, end_y;
    int num_data, n_data;
    int num_points, n_point;
    const cairo_path_data_t *src_data;

    num_data = segment->path.num_data;
    data_size = sizeof(cairo_path_data_t) * num_data;
    data = cpml_alloca(data_size);
    end_x = segment->path.data[1].point.x;
    end_y = segment->path.data[1].point.y;

    for (n_data = 2; n_data < num_data; ++n_data) {
        src_data = segment->path.data + n_data;
	num_points = src_data->header.length;

        dst_data = data + num_data - n_data - num_points + 2;
        dst_data->header.type = src_data->header.type;
        dst_data->header.length = num_points;

	for (n_point = 1; n_point < num_points; ++n_point) {
            dst_data[num_points - n_point].point.x = end_x;
            dst_data[num_points - n_point].point.y = end_y;
            end_x = src_data[n_point].point.x;
            end_y = src_data[n_point].point.y;
        }

	n_data += n_point - 1;
    }

    data[0].header.type = CAIRO_PATH_MOVE_TO;
    data[0].header.length = 2;
    data[1].point.x = end_x;
    data[1].point.y = end_y;
    memcpy(segment->path.data, data, data_size);
}

/**
 * cpml_segment_transform:
 * @segment: a #CpmlSegment
 * @matrix: the matrix to be applied
 *
 * Applies @matrix on all the points of @segment.
 **/
void
cpml_segment_transform(CpmlSegment *segment, const cairo_matrix_t *matrix)
{
    cairo_path_data_t *data;
    int n_data, num_data;
    int n_point, num_points;

    data = segment->path.data;
    num_data = segment->path.num_data;

    for (n_data = 0; n_data < num_data; n_data += num_points) {
        num_points = data->header.length;
        ++data;
        for (n_point = 1; n_point < num_points; ++n_point) {
            cairo_matrix_transform_point(matrix, &data->point.x, &data->point.y);
            ++data;
        }
    }
}

/**
 * cpml_segment_offset:
 * @segment: a #CpmlSegment
 * @offset: the offset distance
 *
 * Offsets a segment of the specified amount, that is builds a "parallel"
 * segment at the @offset distance from the original one and returns the
 * result by replacing the original @segment.
 *
 * Return value: 1 on success, 0 on errors
 *
 * @todo: closed path are not yet managed; the solution is not so obvious.
 **/
cairo_bool_t
cpml_segment_offset(CpmlSegment *segment, double offset)
{
    int num_data, n_data;
    int num_points, n_point;
    cairo_path_data_t *data;
    cairo_path_data_t *last_data;
    CpmlVector v_old, v_new;
    CpmlPair p_old;
    CpmlPair p[4];

    num_data = segment->path.num_data;
    last_data = NULL;
    p_old.x = 0;
    p_old.y = 0;

    for (n_data = 0; n_data < num_data; n_data += data->header.length) {
        data = segment->path.data + n_data;
        num_points = data->header.length - 1;

        /* Fill the p[] vector */
        cpml_pair_copy(&p[0], &p_old);
        for (n_point = 1; n_point <= num_points; ++ n_point) {
            p[n_point].x = data[n_point].point.x;
            p[n_point].y = data[n_point].point.y;
        }

        /* Save the last direction vector in v_old */
        cpml_pair_copy(&v_old, &v_new);

        switch (data->header.type) {

        case CAIRO_PATH_MOVE_TO:
            v_new.x = 0;
            v_new.y = 0;
            break;

        case CAIRO_PATH_LINE_TO:
            line_offset(p, &v_new, offset);
            break;

        case CAIRO_PATH_CURVE_TO:
            curve_offset(p, &v_new, offset);
            break;

        case CAIRO_PATH_CLOSE_PATH:
            return 1;
        }

        join_primitives(last_data, &v_old, &p[0], &v_new);

        /* Save the end point of the original primitive in p_old */
        last_data = data + num_points;
        p_old.x = last_data->point.x;
        p_old.y = last_data->point.y;

        /* Store the results got from the p[] vector in the cairo path */
        for (n_point = 1; n_point <= num_points; ++ n_point) {
            data[n_point].point.x = p[n_point].x;
            data[n_point].point.y = p[n_point].y;
        }
    }

    return 1;
}

/**
 * segment_normalize:
 * @segment: a #CpmlSegment
 *
 * Strips the leading CAIRO_PATH_MOVE_TO primitives, updating the CpmlSegment
 * structure accordling. One, and only once, MOVE_TO primitive is left.
 *
 * Return value: 1 on success, 0 on no leading MOVE_TOs or on errors
 **/
static cairo_bool_t
segment_normalize(CpmlSegment *segment)
{
    cairo_path_data_t *data;

    if (segment == NULL || segment->path.num_data <= 0)
        return 0;

    data = segment->path.data;
    if (data->header.type != CAIRO_PATH_MOVE_TO) {
        segment->path.status = CAIRO_STATUS_INVALID_PATH_DATA;
        return 0;
    }

    while (segment->path.num_data >= 0) {
        data += 2;
        if (data->header.type != CAIRO_PATH_MOVE_TO)
            return 1;

        segment->path.data = data;
        segment->path.num_data -= 2;
    }

    return 0;
}

/**
 * line_offset:
 * @p: an array of two #CpmlPair structs
 * @vector: the ending direction vector of the resulting primitive
 * @offset: distance for the computed parallel line
 *
 * Given a line segment starting from @p[0] and ending in @p[1],
 * computes the parallel line distant @offset from the original one
 * and returns the result in the @p array.
 *
 * The direction vector of the new line segment is saved in @vector
 * with a somewhat arbitrary magnitude.
 **/
static void
line_offset(CpmlPair *p, CpmlVector *vector, double offset)
{
    CpmlVector normal;

    cpml_pair_sub(cpml_pair_copy(vector, &p[1]), &p[0]);

    cpml_vector_from_pair(&normal, vector, offset);
    cpml_vector_normal(&normal);

    cpml_pair_add(&p[0], &normal);
    cpml_pair_add(&p[1], &normal);
}

/**
 * curve_offset:
 * @p:      (inout): an array of four #CpmlPair structs
 * @vector: (out):   the ending direction vector of the resulting primitive
 * @offset: (in):    distance for the computed parallel line
 *
 * Given a cubic Bézier segment starting from @p[0] and ending
 * in p[3], with control points in @p[1] and @p[2], this function
 * finds the approximated Bézier curve parallel to the given one
 * at distance @offset (an offset curve).
 *
 * The four points needed to build the new curve are returned
 * in the @p vector. The angular coefficient of the new curve
 * in @p[3] is returned as @vector with @offset magnitude.
 *
 * To solve the offset problem, a custom algorithm is used. First, the
 * resulting curve MUST have the same slope at the start and end point.
 * These constraints are not sufficient to resolve the system, so I let
 * the curve pass thought a given point (pm, known and got from the
 * original curve) at a given time (m, now hardcoded to 0.5).
 *
 * Firstly, I define some useful variables:
 *
 * v0 = unitvector(p[1]-p[0]) * offset;
 * v3 = unitvector(p[3]-p[2]) * offset;
 * p0 = p[0] + normal v0;
 * p3 = p[3] + normal v3.
 *
 * Now I want the curve to have the specified slopes at the start
 * and end point. Forcing the same slope at the start point means:
 *
 * p1 = p0 + k1 v0.
 *
 * where k1 is an arbitrary factor. Decomposing for x and y components and
 * getting rid of k1:
 *
 * p1.x - p0.x = k1 v0.x;
 * p1.y - p0.y = k1 v0.y.
 *
 * (p1.x - p0.x) v0.y = (p1.y - p0.y) v0.x.
 *
 * Doing the same for the end point gives:
 * p2 = p3 + k2 v3.
 *
 * (p3.x - p2.x) v3.y = (p3.y - p2.y) v3.x.
 *
 * Now I interpolate the curve by forcing it to pass throught pm
 * when "time" is m, where 0 < m < 1. The cubic Bézier function is:
 *
 * C(t) = (1-t)³p0 + 3t(1-t)²p1 + 3t²(1-t)p2 + t³p3.
 *
 * and forcing t=m and C(t) = pm:
 *
 * pm = (1-m)³p0 + 3m(1-m)²p1 + 3m²(1-m)p2 + m³p3.
 * (1-m) p1 + m p2 = (pm - (1-m)³p0 - m³p3) / (3m (1-m)).
 *
 * Either p0 and p3 of the curve are known, so I can get rid of
 * the constant part:
 *
 * pk = (pm - (1-m)³p0 - m³p3) / (3m (1-m));
 * (1-m) p1 + m p2 = pk.
 *
 * So the final system is:
 *
 * (1-m) p1.x + m p2.x = pk->x;
 * (1-m) p1.y + m p2.y = pk->y;
 * (p1.x - p0.x) v0.y = (p1.y - p0.y) v0.x.
 * (p3.x - p2.x) v3.y = (p3.y - p2.y) v3.x.
 *
 * Given the above system, I get the following pool of equations:
 *
 * p1.x = (pk.x - m p2.x)/(1-m);
 * p1.y = (pk.y - m p2.y)/(1-m).
 *
 * p2.x = (pk.x - (1-m) p1.x)/m;
 * p2.y = (pk.y - (1-m) p1.y)/m.
 *
 * p1.x = p0.x + (p1.y - p0.y) v0.x/v0.y;
 * p1.y = p0.y + (p1.x - p0.x) v0.y/v0.x.
 *
 * p2.x = p3.x - (p3.y - p2.y) v3.x/v3.y;
 * p2.y = p3.y - (p3.x - p2.x) v3.y/v3.x.
 *
 * The algorithm takes from this pool what is needed in any specific case.
 *
 * Now the hard part: finding the first solution. Solving this system
 * is a nightmare because of divisions by 0 exceptions.
 *
 * After a lot of tedious calculations, it came out when the
 * v3.x*v0.y == v0.x*v3.y condition is met, that is when v0 and v1
 * are parallel and in the same direction, this algorithm is not valid:
 * this condition should be checked before anything else.
 *
 * If v3.x*v0.y == v0.x*v3.y, an alternative approach is provided:
 * the control points are found by offseting the control poligon
 * (as done by Tiller and Hanson) but using a 4/3 factor on the p1-p2
 * line to compensate the distance between the curve and the p1-p2 line
 * (the top of the curve is at exactly 3/4 of the poligon height
 * when v1 and v2 are parallel and in the same direction).
 *
 * So if v3.x*v0.y == v0.x*v3.y and if m is 0.5 (the mid point of the
 * curve) so that vm is the vector of @offset magnitude at m, I have:
 *
 * p1 = p0 + (p[1]-p[0]) + 4/3 vm;
 * p2 = p3 + (p[2]-p[3]) - 4/3 vm.
 *
 * In the other cases, I get the first coordinate by looking for a
 * 0 vector component. If no vector component is 0 any division is allowed
 * so, taking some equation from the pool, the following system is used:
 *
 * p1.x = (pk.x - m p2.x)/(1-m);
 * p1.x = p0.x + (p1.y-p0.y) v0.x/v0.y;
 * p2.x = p3.x - (p3.y - p2.y) v3.x/v3.y;
 * p2.y = (pk.y - (1-m) p1.y)/m.
 *
 * And now I resolve to get the p1.y value:
 *
 * (pk.x - m p2.x)/(1-m) = p0.x + (p1.y-p0.y) v0.x/v0.y;
 * p2.x = p3.x + ((pk.y - (1-m) p1.y)/m - p3.y) v3.x/v3.y.
 *
 * p2.x = pk.x/m - (1-m)/m (p0.x - v0.x/v0.y p0.y)
 *        - p1.y (1-m)/m v0.x/v0.y;
 * p2.x = p3.x + (pk.y/m - p3.y) v3.x/v3.y
 *        - p1.y (1-m)/m v3.x/v3.y.
 *
 * pk.x/m - (1-m)/m (p0.x - v0.x/v0.y p0.y)
 * - p1.y (1-m)/m v0.x/v0.y =
 *     p3.x + (pk.y/m - p3.y) v3.x/v3.y
 *     - p1.y (1-m)/m v3.x/v3.y.
 *
 * pk.x/m - (1-m)/m (p0.x - p0.y v0.x/v0.y)
 * - p1.y (1-m)/m v0.x/v0.y =
 *     p3.x + (pk.y/m - p3.y) v3.x/v3.y
 *     - p1.y (1-m)/m v3.x/v3.y.
 *
 * p1.y (v3.x/v3.y - v0.x/v0.y) = (p0.x - p0.y v0.x/v0.y)
 *     + (m (p3.x + (pk.y/m - p3.y) v3.x/v3.y) - pk.x) / (1-m).
 *
 * And this is the final equation:
 *
 * p1.y = ((p0.x - p0.y v0.x/v0.y)
 *        + (m (p3.x + (pk.y/m - p3.y) v3.x/v3.y) - pk.x) / (1-m))
 *        / (v3.x/v3.y - v0.x/v0.y).
 **/
static void
curve_offset(CpmlPair *p, CpmlVector *vector, double offset)
{
    double m, mm;
    CpmlVector v0, v3, vm;
    CpmlPair p0, p1, p2, p3, pm, pk;

    /* v0 = vector p[1]-p[0] of @offset magnitude */
    cpml_pair_sub(cpml_pair_copy(&v0, &p[1]), &p[0]);
    cpml_vector_from_pair(&v0, &v0, offset);

    /* v3 = vector p[3]-p[2] of @offset magnitude */
    cpml_pair_sub(cpml_pair_copy(&v3, &p[3]), &p[2]);
    cpml_vector_from_pair(&v3, &v3, offset);

    /* p0 = p[0] + normal of v0 (exact final value of p0) */
    cpml_vector_normal(cpml_pair_copy(&p0, &v0));
    cpml_pair_add(&p0, &p[0]);

    /* p3 = p[3] + normal of v3 (exact final value of p3) */
    cpml_vector_normal(cpml_pair_copy(&p3, &v3));
    cpml_pair_add(&p3, &p[3]);

    /* By default, interpolate the new curve by offseting the mid point.
     * @todo: use a better candidate. */
    m = 0.5;
    mm = 1.-m;

    /* pm = point in C(m) offseted the requested @offset distance */
    cpml_vector_at_curve(&vm, &p[0], &p[1], &p[2], &p[3], m, offset);
    cpml_vector_normal(&vm);
    cpml_pair_at_curve(&pm, &p[0], &p[1], &p[2], &p[3], m);
    cpml_pair_add(&pm, &vm);

    /* pk = (pm - (1-m)³p0 - m³p3) / (3m (1-m)) */
    pk.x = (pm.x - mm*mm*mm*p0.x - m*m*m*p3.x) / (m*3*mm);
    pk.y = (pm.y - mm*mm*mm*p0.y - m*m*m*p3.y) / (m*3*mm);

    if (v3.x*v0.y == v0.x*v3.y) {
        /* Same slope at the start and end point (parallel p0-p1 and p3-p2) */
        p1.x = p0.x + (p[1].x - p[0].x) + vm.x * 4/3;
        p1.y = p0.y + (p[1].y - p[0].y) + vm.y * 4/3;
        p2.x = p3.x + (p[2].x - p[3].x) + vm.x * 4/3;
        p2.y = p3.y + (p[2].y - p[3].y) + vm.y * 4/3;

    /* Probably the following can be condensed */
    } else if (v0.x == 0) {
        p1.x = p0.x;
        p2.x = (pk.x - mm*p1.x)/m;
        if (v3.x == 0)
            p2.y = p3.y + (pm.y - p3.y) * 4/3;
        else if (v3.y == 0)
            p2.y = p3.y;
        else
            p2.y = p3.y - (p3.x - p2.x) * v3.y/v3.x; 
        p1.y = (pk.y - m*p2.y)/mm;
    } else if (v0.y == 0) {
        p1.y = p0.y;
        p2.y = (pk.y - mm*p1.y)/m;
        if (v3.y == 0)
            p2.x = p3.x + (pm.x - p3.x) * 4/3;
        else if (v3.x == 0)
            p2.x = p3.x;
        else
            p2.x = p3.x - (p3.y - p2.y) * v3.x/v3.y; 
        p1.x = (pk.x - m*p2.x)/mm;
    } else if (v3.x == 0) {
        p2.x = p3.x;
        p1.x = (pk.x - m*p2.x)/mm;
        p1.y = p0.y;
        if (v0.y != 0 && v0.x != 0)
            p1.y += (p1.x - p0.x) * v0.y/v0.x;
        p2.y = (pk.y - mm*p1.y)/m;
    } else if (v3.y == 0) {
        p2.y = p3.y;
        p1.y = (pk.y - m*p2.y)/mm;
        p1.x = p0.x;
        if (v0.x != 0 && v0.y != 0)
            p1.x += (p1.y - p0.y) * v0.x/v0.y;
        p2.x = (pk.x - mm*p1.x)/m;
    } else if (v3.x*v0.y == v0.x*v3.y) {
        p1.x = p1.x + (pm.x - p1.x) * 4/3;
        p1.y = p1.y + (pm.y - p1.y) * 4/3;
        p2.x = p3.x + (pm.x - p3.x) * 4/3;
        p2.y = p3.y + (pm.y - p3.y) * 4/3;
    } else {
        p1.y = m/mm*v0.y*(v3.y*(p3.x-pk.x/m) + (pk.y/m-p3.y)*v3.x)
            / (v3.x*v0.y - v0.x*v3.y);
        p1.x = p0.x + (p1.y - p0.y) * v0.x/v0.y;
        p2.y = (pk.y - mm*p1.y)/m;
        p2.x = p3.x - (p3.y - p2.y) * v3.x/v3.y; 
    }

    /* Return the new curve in the original array */
    cpml_pair_copy(&p[0], &p0);
    cpml_pair_copy(&p[1], &p1);
    cpml_pair_copy(&p[2], &p2);
    cpml_pair_copy(&p[3], &p3);

    /* Return the ending vector */
    cpml_pair_copy(vector, &v3);
}

/**
 * join_primitives:
 * @last_data: the previous primitive end data point (if any)
 * @last_vector: @last_data direction vector
 * @new_point: the new primitive starting point
 * @new_vector: @new_point direction vector
 *
 * Joins two primitive modifying the end point of the first one (stored
 * as cairo path data in @last_data).
 *
 * @todo: this approach is quite naive when curves are involved.
 **/
static void
join_primitives(cairo_path_data_t *last_data, const CpmlVector *last_vector,
                const CpmlPair *new_point, const CpmlVector *new_vector)
{
    CpmlPair last_point;

    if (last_data == NULL)
        return;

    last_point.x = last_data->point.x;
    last_point.y = last_data->point.y;

    if (cpml_pair_intersection_pv_pv(&last_point,
                                     &last_point, last_vector,
                                     new_point, new_vector)) {
        last_data->point.x = last_point.x;
        last_data->point.y = last_point.y;
    } else {
        last_data->point.x = new_point->x;
        last_data->point.y = new_point->y;
    }
}
