/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2003 Peter Mattis and Spencer Kimball
 *
 * gimpbrush_pdb.c
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

/* NOTE: This file is auto-generated by pdbgen.pl */

#include "config.h"

#include "stamp-pdbgen.h"

#include "gimp.h"


/**
 * SECTION: gimpbrush
 * @title: gimpbrush
 * @short_description: Installable object used by painting and stroking tools.
 *
 * Installable object used by painting and stroking tools.
 **/


/**
 * gimp_brush_new:
 * @name: The requested name of the new brush.
 *
 * Create a new generated brush having default parameters.
 *
 * Creates a new, parametric brush.
 *
 * Returns: (transfer none): The brush.
 *
 * Since: 2.2
 **/
GimpBrush *
gimp_brush_new (const gchar *name)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  GimpBrush *brush = NULL;

  args = gimp_value_array_new_from_types (NULL,
                                          G_TYPE_STRING, name,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-new",
                                               args);
  gimp_value_array_unref (args);

  if (GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS)
    brush = GIMP_VALUES_GET_BRUSH (return_vals, 1);

  gimp_value_array_unref (return_vals);

  return brush;
}

/**
 * gimp_brush_get_by_name:
 * @name: The name of the brush.
 *
 * Returns the brush with the given name.
 *
 * Return an existing brush having the given name. Returns %NULL when
 * no brush exists of that name.
 *
 * Returns: (nullable) (transfer none): The brush.
 *
 * Since: 3.0
 **/
GimpBrush *
gimp_brush_get_by_name (const gchar *name)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  GimpBrush *brush = NULL;

  args = gimp_value_array_new_from_types (NULL,
                                          G_TYPE_STRING, name,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-get-by-name",
                                               args);
  gimp_value_array_unref (args);

  if (GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS)
    brush = GIMP_VALUES_GET_BRUSH (return_vals, 1);

  gimp_value_array_unref (return_vals);

  return brush;
}

/**
 * gimp_brush_is_generated:
 * @brush: The brush.
 *
 * Whether the brush is generated (parametric versus raster).
 *
 * Returns TRUE when brush is parametric.
 *
 * Returns: TRUE if the brush is generated.
 *
 * Since: 2.4
 **/
gboolean
gimp_brush_is_generated (GimpBrush *brush)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gboolean generated = FALSE;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-is-generated",
                                               args);
  gimp_value_array_unref (args);

  if (GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS)
    generated = GIMP_VALUES_GET_BOOLEAN (return_vals, 1);

  gimp_value_array_unref (return_vals);

  return generated;
}

/**
 * gimp_brush_get_info:
 * @brush: The brush.
 * @width: (out): The brush width.
 * @height: (out): The brush height.
 * @mask_bpp: (out): The brush mask bpp.
 * @color_bpp: (out): The brush color bpp.
 *
 * Gets information about the brush.
 *
 * Gets information about the brush: brush extents (width and height),
 * color depth and mask depth (bpp). The color bpp is zero when the
 * brush is parametric versus raster.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.2
 **/
gboolean
gimp_brush_get_info (GimpBrush *brush,
                     gint      *width,
                     gint      *height,
                     gint      *mask_bpp,
                     gint      *color_bpp)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gboolean success = TRUE;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-get-info",
                                               args);
  gimp_value_array_unref (args);

  *width = 0;
  *height = 0;
  *mask_bpp = 0;
  *color_bpp = 0;

  success = GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS;

  if (success)
    {
      *width = GIMP_VALUES_GET_INT (return_vals, 1);
      *height = GIMP_VALUES_GET_INT (return_vals, 2);
      *mask_bpp = GIMP_VALUES_GET_INT (return_vals, 3);
      *color_bpp = GIMP_VALUES_GET_INT (return_vals, 4);
    }

  gimp_value_array_unref (return_vals);

  return success;
}

/**
 * _gimp_brush_get_pixels:
 * @brush: The brush.
 * @width: (out): The brush width.
 * @height: (out): The brush height.
 * @mask_bpp: (out): The brush mask bpp.
 * @mask_bytes: (out) (transfer full): The brush mask data.
 * @color_bpp: (out): The brush color bpp.
 * @color_bytes: (out) (transfer full): The brush color data.
 *
 * Gets information about the brush.
 *
 * Gets information about the brush: the brush extents (width and
 * height) and its pixels data. The color bpp is zero and pixels empty
 * when the brush is parametric versus raster.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.2
 **/
gboolean
_gimp_brush_get_pixels (GimpBrush  *brush,
                        gint       *width,
                        gint       *height,
                        gint       *mask_bpp,
                        GBytes    **mask_bytes,
                        gint       *color_bpp,
                        GBytes    **color_bytes)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gboolean success = TRUE;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-get-pixels",
                                               args);
  gimp_value_array_unref (args);

  *width = 0;
  *height = 0;
  *mask_bpp = 0;
  *mask_bytes = NULL;
  *color_bpp = 0;
  *color_bytes = NULL;

  success = GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS;

  if (success)
    {
      *width = GIMP_VALUES_GET_INT (return_vals, 1);
      *height = GIMP_VALUES_GET_INT (return_vals, 2);
      *mask_bpp = GIMP_VALUES_GET_INT (return_vals, 3);
      *mask_bytes = GIMP_VALUES_DUP_BYTES (return_vals, 4);
      *color_bpp = GIMP_VALUES_GET_INT (return_vals, 5);
      *color_bytes = GIMP_VALUES_DUP_BYTES (return_vals, 6);
    }

  gimp_value_array_unref (return_vals);

  return success;
}

/**
 * gimp_brush_get_spacing:
 * @brush: The brush.
 *
 * Gets the brush spacing, the stamping frequency.
 *
 * Returns the spacing setting for the brush. Spacing is an integer
 * between 0 and 1000 which represents a percentage of the maximum of
 * the width and height of the mask. Both parametric and raster brushes
 * have a spacing.
 *
 * Returns: The brush spacing.
 *
 * Since: 2.2
 **/
gint
gimp_brush_get_spacing (GimpBrush *brush)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gint spacing = 0;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-get-spacing",
                                               args);
  gimp_value_array_unref (args);

  if (GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS)
    spacing = GIMP_VALUES_GET_INT (return_vals, 1);

  gimp_value_array_unref (return_vals);

  return spacing;
}

/**
 * gimp_brush_set_spacing:
 * @brush: The brush.
 * @spacing: The brush spacing.
 *
 * Sets the brush spacing.
 *
 * Set the spacing for the brush. The spacing must be an integer
 * between 0 and 1000. Both parametric and raster brushes have a
 * spacing. Returns an error when the brush is not editable. Create a
 * new or copied brush or to get an editable brush.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.4
 **/
gboolean
gimp_brush_set_spacing (GimpBrush *brush,
                        gint       spacing)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gboolean success = TRUE;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          G_TYPE_INT, spacing,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-set-spacing",
                                               args);
  gimp_value_array_unref (args);

  success = GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS;

  gimp_value_array_unref (return_vals);

  return success;
}

/**
 * gimp_brush_get_shape:
 * @brush: The brush.
 * @shape: (out): The brush shape.
 *
 * Gets the shape of a generated brush.
 *
 * Gets the shape of a generated brush. Returns an error when called
 * for a non-parametric brush. The choices for shape are Circle
 * (GIMP_BRUSH_GENERATED_CIRCLE), Square (GIMP_BRUSH_GENERATED_SQUARE),
 * and Diamond (GIMP_BRUSH_GENERATED_DIAMOND). Other shapes might be
 * added in the future.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.4
 **/
gboolean
gimp_brush_get_shape (GimpBrush               *brush,
                      GimpBrushGeneratedShape *shape)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gboolean success = TRUE;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-get-shape",
                                               args);
  gimp_value_array_unref (args);

  *shape = 0;

  success = GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS;

  if (success)
    *shape = GIMP_VALUES_GET_ENUM (return_vals, 1);

  gimp_value_array_unref (return_vals);

  return success;
}

/**
 * gimp_brush_set_shape:
 * @brush: The brush.
 * @shape_in: The brush shape.
 * @shape_out: (out): The brush shape actually assigned.
 *
 * Sets the shape of a generated brush.
 *
 * Sets the shape of a generated brush. Returns an error when brush is
 * non-parametric or not editable. The choices for shape are Circle
 * (GIMP_BRUSH_GENERATED_CIRCLE), Square (GIMP_BRUSH_GENERATED_SQUARE),
 * and Diamond (GIMP_BRUSH_GENERATED_DIAMOND).
 *
 * Returns: TRUE on success.
 *
 * Since: 2.4
 **/
gboolean
gimp_brush_set_shape (GimpBrush               *brush,
                      GimpBrushGeneratedShape  shape_in,
                      GimpBrushGeneratedShape *shape_out)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gboolean success = TRUE;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          GIMP_TYPE_BRUSH_GENERATED_SHAPE, shape_in,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-set-shape",
                                               args);
  gimp_value_array_unref (args);

  *shape_out = 0;

  success = GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS;

  if (success)
    *shape_out = GIMP_VALUES_GET_ENUM (return_vals, 1);

  gimp_value_array_unref (return_vals);

  return success;
}

/**
 * gimp_brush_get_radius:
 * @brush: The brush.
 * @radius: (out): The radius of the brush in pixels.
 *
 * Gets the radius of a generated brush.
 *
 * Gets the radius of a generated brush. Returns an error when called
 * for a non-parametric brush.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.4
 **/
gboolean
gimp_brush_get_radius (GimpBrush *brush,
                       gdouble   *radius)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gboolean success = TRUE;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-get-radius",
                                               args);
  gimp_value_array_unref (args);

  *radius = 0.0;

  success = GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS;

  if (success)
    *radius = GIMP_VALUES_GET_DOUBLE (return_vals, 1);

  gimp_value_array_unref (return_vals);

  return success;
}

/**
 * gimp_brush_set_radius:
 * @brush: The brush.
 * @radius_in: The desired brush radius in pixel.
 * @radius_out: (out): The brush radius actually assigned.
 *
 * Sets the radius of a generated brush.
 *
 * Sets the radius for a generated brush. Clamps radius to [0.1,
 * 4000.0]. Returns the clamped value. Returns an error when brush is
 * non-parametric or not editable.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.4
 **/
gboolean
gimp_brush_set_radius (GimpBrush *brush,
                       gdouble    radius_in,
                       gdouble   *radius_out)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gboolean success = TRUE;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          G_TYPE_DOUBLE, radius_in,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-set-radius",
                                               args);
  gimp_value_array_unref (args);

  *radius_out = 0.0;

  success = GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS;

  if (success)
    *radius_out = GIMP_VALUES_GET_DOUBLE (return_vals, 1);

  gimp_value_array_unref (return_vals);

  return success;
}

/**
 * gimp_brush_get_spikes:
 * @brush: The brush.
 * @spikes: (out): The number of spikes on the brush.
 *
 * Gets the number of spikes for a generated brush.
 *
 * Gets the number of spikes for a generated brush. Returns an error
 * when called for a non-parametric brush.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.4
 **/
gboolean
gimp_brush_get_spikes (GimpBrush *brush,
                       gint      *spikes)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gboolean success = TRUE;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-get-spikes",
                                               args);
  gimp_value_array_unref (args);

  *spikes = 0;

  success = GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS;

  if (success)
    *spikes = GIMP_VALUES_GET_INT (return_vals, 1);

  gimp_value_array_unref (return_vals);

  return success;
}

/**
 * gimp_brush_set_spikes:
 * @brush: The brush.
 * @spikes_in: The desired number of spikes.
 * @spikes_out: (out): The number of spikes actually assigned.
 *
 * Sets the number of spikes for a generated brush.
 *
 * Sets the number of spikes for a generated brush. Clamps spikes to
 * [2,20]. Returns the clamped value. Returns an error when brush is
 * non-parametric or not editable.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.4
 **/
gboolean
gimp_brush_set_spikes (GimpBrush *brush,
                       gint       spikes_in,
                       gint      *spikes_out)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gboolean success = TRUE;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          G_TYPE_INT, spikes_in,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-set-spikes",
                                               args);
  gimp_value_array_unref (args);

  *spikes_out = 0;

  success = GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS;

  if (success)
    *spikes_out = GIMP_VALUES_GET_INT (return_vals, 1);

  gimp_value_array_unref (return_vals);

  return success;
}

/**
 * gimp_brush_get_hardness:
 * @brush: The brush.
 * @hardness: (out): The hardness of the brush.
 *
 * Gets the hardness of a generated brush.
 *
 * Gets the hardness of a generated brush. The hardness of a brush is
 * the amount its intensity fades at the outside edge, as a double
 * between 0.0 and 1.0. Returns an error when called for a
 * non-parametric brush.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.4
 **/
gboolean
gimp_brush_get_hardness (GimpBrush *brush,
                         gdouble   *hardness)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gboolean success = TRUE;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-get-hardness",
                                               args);
  gimp_value_array_unref (args);

  *hardness = 0.0;

  success = GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS;

  if (success)
    *hardness = GIMP_VALUES_GET_DOUBLE (return_vals, 1);

  gimp_value_array_unref (return_vals);

  return success;
}

/**
 * gimp_brush_set_hardness:
 * @brush: The brush.
 * @hardness_in: The desired brush hardness.
 * @hardness_out: (out): The brush hardness actually assigned.
 *
 * Sets the hardness of a generated brush.
 *
 * Sets the hardness for a generated brush. Clamps hardness to [0.0,
 * 1.0]. Returns the clamped value. Returns an error when brush is
 * non-parametric or not editable.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.4
 **/
gboolean
gimp_brush_set_hardness (GimpBrush *brush,
                         gdouble    hardness_in,
                         gdouble   *hardness_out)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gboolean success = TRUE;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          G_TYPE_DOUBLE, hardness_in,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-set-hardness",
                                               args);
  gimp_value_array_unref (args);

  *hardness_out = 0.0;

  success = GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS;

  if (success)
    *hardness_out = GIMP_VALUES_GET_DOUBLE (return_vals, 1);

  gimp_value_array_unref (return_vals);

  return success;
}

/**
 * gimp_brush_get_aspect_ratio:
 * @brush: The brush.
 * @aspect_ratio: (out): The aspect ratio of the brush.
 *
 * Gets the aspect ratio of a generated brush.
 *
 * Gets the aspect ratio of a generated brush. Returns an error when
 * called for a non-parametric brush. The aspect ratio is a double
 * between 0.0 and 1000.0.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.4
 **/
gboolean
gimp_brush_get_aspect_ratio (GimpBrush *brush,
                             gdouble   *aspect_ratio)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gboolean success = TRUE;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-get-aspect-ratio",
                                               args);
  gimp_value_array_unref (args);

  *aspect_ratio = 0.0;

  success = GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS;

  if (success)
    *aspect_ratio = GIMP_VALUES_GET_DOUBLE (return_vals, 1);

  gimp_value_array_unref (return_vals);

  return success;
}

/**
 * gimp_brush_set_aspect_ratio:
 * @brush: The brush.
 * @aspect_ratio_in: The desired brush aspect ratio.
 * @aspect_ratio_out: (out): The brush aspect ratio actually assigned.
 *
 * Sets the aspect ratio of a generated brush.
 *
 * Sets the aspect ratio for a generated brush. Clamps aspect ratio to
 * [0.0, 1000.0]. Returns the clamped value. Returns an error when
 * brush is non-parametric or not editable.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.4
 **/
gboolean
gimp_brush_set_aspect_ratio (GimpBrush *brush,
                             gdouble    aspect_ratio_in,
                             gdouble   *aspect_ratio_out)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gboolean success = TRUE;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          G_TYPE_DOUBLE, aspect_ratio_in,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-set-aspect-ratio",
                                               args);
  gimp_value_array_unref (args);

  *aspect_ratio_out = 0.0;

  success = GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS;

  if (success)
    *aspect_ratio_out = GIMP_VALUES_GET_DOUBLE (return_vals, 1);

  gimp_value_array_unref (return_vals);

  return success;
}

/**
 * gimp_brush_get_angle:
 * @brush: The brush.
 * @angle: (out): The rotation angle of the brush in degree.
 *
 * Gets the rotation angle of a generated brush.
 *
 * Gets the angle of rotation for a generated brush. Returns an error
 * when called for a non-parametric brush.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.4
 **/
gboolean
gimp_brush_get_angle (GimpBrush *brush,
                      gdouble   *angle)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gboolean success = TRUE;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-get-angle",
                                               args);
  gimp_value_array_unref (args);

  *angle = 0.0;

  success = GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS;

  if (success)
    *angle = GIMP_VALUES_GET_DOUBLE (return_vals, 1);

  gimp_value_array_unref (return_vals);

  return success;
}

/**
 * gimp_brush_set_angle:
 * @brush: The brush.
 * @angle_in: The desired brush rotation angle in degrees.
 * @angle_out: (out): The brush rotation angle actually assigned.
 *
 * Sets the rotation angle of a generated brush.
 *
 * Sets the rotation angle for a generated brush. Sets the angle modulo
 * 180, in the range [-180.0, 180.0]. Returns the clamped value.
 * Returns an error when brush is non-parametric or not editable.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.4
 **/
gboolean
gimp_brush_set_angle (GimpBrush *brush,
                      gdouble    angle_in,
                      gdouble   *angle_out)
{
  GimpValueArray *args;
  GimpValueArray *return_vals;
  gboolean success = TRUE;

  args = gimp_value_array_new_from_types (NULL,
                                          GIMP_TYPE_BRUSH, brush,
                                          G_TYPE_DOUBLE, angle_in,
                                          G_TYPE_NONE);

  return_vals = _gimp_pdb_run_procedure_array (gimp_get_pdb (),
                                               "gimp-brush-set-angle",
                                               args);
  gimp_value_array_unref (args);

  *angle_out = 0.0;

  success = GIMP_VALUES_GET_ENUM (return_vals, 0) == GIMP_PDB_SUCCESS;

  if (success)
    *angle_out = GIMP_VALUES_GET_DOUBLE (return_vals, 1);

  gimp_value_array_unref (return_vals);

  return success;
}
