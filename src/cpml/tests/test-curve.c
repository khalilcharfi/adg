/* ADG - Automatic Drawing Generation
 * Copyright (C) 2007-2015  Nicola Fontana <ntd at entidi.it>
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


#include <adg-test.h>
#include <cpml.h>


static cairo_path_data_t curve_data[] = {
    { .header = { CPML_MOVE, 2 }},
    { .point = { 1, 1 }},

    { .header = { CPML_CURVE, 4 }},
    { .point = { 1, 3 }},
    { .point = { 3, 3 }},
    { .point = { 3, 5 }}
};

CpmlPrimitive curve = {
    NULL,
    &curve_data[1],
    &curve_data[2]
};


static void
_cpml_test_sanity_pair_at_time(gint i)
{
    CpmlPair pair;

    /* Check that invalid arguments crashes the process */
    switch (i) {
    case 1:
        cpml_curve_put_pair_at_time(NULL, 0, &pair);
        break;
    case 2:
        cpml_curve_put_pair_at_time(&curve, 0, NULL);
        break;
    default:
        g_test_trap_assert_failed();
        break;
    }
}

static void
_cpml_test_sanity_vector_at_time(gint i)
{
    CpmlVector vector;

    /* Check that invalid arguments crashes the process */
    switch (i) {
    case 1:
        cpml_curve_put_vector_at_time(NULL, 0, &vector);
        break;
    case 2:
        cpml_curve_put_vector_at_time(&curve, 0, NULL);
        break;
    default:
        g_test_trap_assert_failed();
        break;
    }
}

static void
_cpml_test_sanity_offset_at_time(gint i)
{
    CpmlPair pair;

    /* Check that invalid arguments crashes the process */
    switch (i) {
    case 1:
        cpml_curve_put_offset_at_time(NULL, 0, 0, &pair);
        break;
    case 2:
        cpml_curve_put_offset_at_time(&curve, 0, 0, NULL);
        break;
    default:
        g_test_trap_assert_failed();
        break;
    }
}

static void
_cpml_test_offset_algorithm(void)
{
    g_assert_cmpint(cpml_curve_offset_algorithm(CPML_CURVE_OFFSET_ALGORITHM_GEOMETRICAL), ==, CPML_CURVE_OFFSET_ALGORITHM_HANDCRAFT);
    g_assert_cmpint(cpml_curve_offset_algorithm(CPML_CURVE_OFFSET_ALGORITHM_NONE), ==, CPML_CURVE_OFFSET_ALGORITHM_GEOMETRICAL);
    g_assert_cmpint(cpml_curve_offset_algorithm(CPML_CURVE_OFFSET_ALGORITHM_BAIOCA), ==, CPML_CURVE_OFFSET_ALGORITHM_GEOMETRICAL);
    g_assert_cmpint(cpml_curve_offset_algorithm(CPML_CURVE_OFFSET_ALGORITHM_DEFAULT), ==, CPML_CURVE_OFFSET_ALGORITHM_BAIOCA);
    g_assert_cmpint(cpml_curve_offset_algorithm(CPML_CURVE_OFFSET_ALGORITHM_GEOMETRICAL), ==, CPML_CURVE_OFFSET_ALGORITHM_HANDCRAFT);
    g_assert_cmpint(cpml_curve_offset_algorithm(CPML_CURVE_OFFSET_ALGORITHM_HANDCRAFT), ==, CPML_CURVE_OFFSET_ALGORITHM_GEOMETRICAL);
    g_assert_cmpint(cpml_curve_offset_algorithm(CPML_CURVE_OFFSET_ALGORITHM_NONE), ==, CPML_CURVE_OFFSET_ALGORITHM_HANDCRAFT);
}

static void
_cpml_test_pair_at_time(void)
{
    CpmlPair pair;

    cpml_curve_put_pair_at_time(&curve, 0, &pair);
    g_assert_cmpfloat(pair.x, ==, 1);
    g_assert_cmpfloat(pair.y, ==, 1);

    cpml_curve_put_pair_at_time(&curve, 0.5, &pair);
    g_assert_cmpfloat(pair.x, ==, 2);
    g_assert_cmpfloat(pair.y, ==, 3);

    cpml_curve_put_pair_at_time(&curve, 1, &pair);
    g_assert_cmpfloat(pair.x, ==, 3);
    g_assert_cmpfloat(pair.y, ==, 5);

    /* t is not bound to domain 0..1 */
    cpml_curve_put_pair_at_time(&curve, -1, &pair);
    g_assert_cmpfloat(pair.x, ==, 11);
    g_assert_cmpfloat(pair.y, ==, -15);

    cpml_curve_put_pair_at_time(&curve, 2, &pair);
    g_assert_cmpfloat(pair.x, ==, -7);
    g_assert_cmpfloat(pair.y, ==, 21);
}

static void
_cpml_test_vector_at_time(void)
{
    CpmlVector vector;

    cpml_curve_put_vector_at_time(&curve, 0, &vector);
    g_assert_cmpfloat(vector.x, ==, 0);
    g_assert_cmpfloat(vector.y, ==, 6);

    cpml_curve_put_vector_at_time(&curve, 0.5, &vector);
    g_assert_cmpfloat(vector.x, ==, 3);
    g_assert_cmpfloat(vector.y, ==, 3);

    cpml_curve_put_vector_at_time(&curve, 1, &vector);
    g_assert_cmpfloat(vector.x, ==, 0);
    g_assert_cmpfloat(vector.y, ==, 6);

    /* t is not bound to domain 0..1 */
    cpml_curve_put_vector_at_time(&curve, -1, &vector);
    g_assert_cmpfloat(vector.x, ==, -24);
    g_assert_cmpfloat(vector.y, ==, 30);

    cpml_curve_put_vector_at_time(&curve, 2, &vector);
    g_assert_cmpfloat(vector.x, ==, -24);
    g_assert_cmpfloat(vector.y, ==, 30);
}

static void
_cpml_test_offset_at_time(void)
{
    CpmlPair pair;

    cpml_curve_put_offset_at_time(&curve, 0, 1, &pair);
    g_assert_cmpfloat(pair.x, ==, 0);
    g_assert_cmpfloat(pair.y, ==, 1);

    cpml_curve_put_offset_at_time(&curve, 1, 1, &pair);
    g_assert_cmpfloat(pair.x, ==, 2);
    g_assert_cmpfloat(pair.y, ==, 5);

    cpml_curve_put_offset_at_time(&curve, 0, 2, &pair);
    g_assert_cmpfloat(pair.x, ==, -1);
    g_assert_cmpfloat(pair.y, ==, 1);

    cpml_curve_put_offset_at_time(&curve, 0.5, 0, &pair);
    g_assert_cmpfloat(pair.x, ==, 2);
    g_assert_cmpfloat(pair.y, ==, 3);

    cpml_curve_put_offset_at_time(&curve, 0.5, G_SQRT2, &pair);
    /* Check only the first 5 digits to avoid rounding errors */
    g_assert_cmpint((pair.x + 0.00005) * 10000, ==, 10000);
    g_assert_cmpint((pair.y + 0.00005) * 10000, ==, 40000);
}


int
main(int argc, char *argv[])
{
    adg_test_init(&argc, &argv);

    adg_test_add_traps("/cpml/curve/sanity/pair-at-time", _cpml_test_sanity_pair_at_time, 2);
    adg_test_add_traps("/cpml/curve/sanity/vector-at-time", _cpml_test_sanity_vector_at_time, 2);
    adg_test_add_traps("/cpml/curve/sanity/offset-at-time", _cpml_test_sanity_offset_at_time, 2);

    g_test_add_func("/cpml/curve/method/offset-algorithm", _cpml_test_offset_algorithm);
    g_test_add_func("/cpml/curve/method/pair-at-time", _cpml_test_pair_at_time);
    g_test_add_func("/cpml/curve/method/vector-at-time", _cpml_test_vector_at_time);
    g_test_add_func("/cpml/curve/method/offset-at-time", _cpml_test_offset_at_time);

    return g_test_run();
}
