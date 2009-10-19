/*
 * GStreamer
 * Copyright (C) 2008 Filippo Argiolas <filippo.argiolas@gmail.com>
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
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gstgleffects.h>
#include <gstgleffectscurves.h>
#include <gstgleffectlumatocurve.h>

/* Gaussian Kernel: std = 1.200000, size = 9x1 */
static gfloat gauss_kernel[9] = { 0.001285f, 0.014607f, 0.082898f,
  0.234927f, 0.332452f, 0.234927f,
  0.082898f, 0.014607f, 0.001285f
};

/* Normalization Constant = 0.999885 */

static void
gst_gl_effects_xray_step_one (gint width, gint height, guint texture,
    gpointer data)
{
  GstGLEffects *effects = GST_GL_EFFECTS (data);

  gst_gl_effects_luma_to_curve (effects, xray_curve, GST_GL_EFFECTS_CURVE_XRAY,
      width, height, texture);
}

static void
gst_gl_effects_xray_step_two (gint width, gint height, guint texture,
    gpointer data)
{
  GstGLEffects *effects = GST_GL_EFFECTS (data);
  GstGLShader *shader;

  shader = g_hash_table_lookup (effects->shaderstable, "xray1");

  if (!shader) {
    shader = gst_gl_shader_new ();
    g_hash_table_insert (effects->shaderstable, "xray1", shader);
  }

  g_return_if_fail (gst_gl_shader_compile_and_check (shader,
          hconv9_fragment_source, GST_GL_SHADER_FRAGMENT_SOURCE));

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  gst_gl_shader_use (shader);

  glActiveTexture (GL_TEXTURE1);
  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glDisable (GL_TEXTURE_RECTANGLE_ARB);

  gst_gl_shader_set_uniform_1i (shader, "tex", 1);

  gst_gl_shader_set_uniform_1fv (shader, "kernel", 9, gauss_kernel);
  gst_gl_shader_set_uniform_1f (shader, "norm_const", 0.999885f);
  gst_gl_shader_set_uniform_1f (shader, "norm_offset", 0.0f);

  gst_gl_effects_draw_texture (effects, texture);
}

static void
gst_gl_effects_xray_step_three (gint width, gint height, guint texture,
    gpointer data)
{
  GstGLEffects *effects = GST_GL_EFFECTS (data);
  GstGLShader *shader;

  shader = g_hash_table_lookup (effects->shaderstable, "xray2");

  if (!shader) {
    shader = gst_gl_shader_new ();
    g_hash_table_insert (effects->shaderstable, "xray2", shader);
  }

  g_return_if_fail (gst_gl_shader_compile_and_check (shader,
          vconv9_fragment_source, GST_GL_SHADER_FRAGMENT_SOURCE));

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  gst_gl_shader_use (shader);

  glActiveTexture (GL_TEXTURE1);
  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glDisable (GL_TEXTURE_RECTANGLE_ARB);

  gst_gl_shader_set_uniform_1i (shader, "tex", 1);

  gst_gl_shader_set_uniform_1fv (shader, "kernel", 9, gauss_kernel);
  gst_gl_shader_set_uniform_1f (shader, "norm_const", 0.999885f);
  gst_gl_shader_set_uniform_1f (shader, "norm_offset", 0.0f);

  gst_gl_effects_draw_texture (effects, texture);
}

static void
gst_gl_effects_xray_step_four (gint width, gint height, guint texture,
    gpointer data)
{
  GstGLEffects *effects = GST_GL_EFFECTS (data);
  GstGLShader *shader;

  gfloat hkern[9] = {
    1.0, 0.0, -1.0,
    2.0, 0.0, -2.0,
    1.0, 0.0, -1.0
  };

  gfloat vkern[9] = {
    1.0, 2.0, 1.0,
    0.0, 0.0, 0.0,
    -1.0, -2.0, -1.0
  };

  shader = g_hash_table_lookup (effects->shaderstable, "xray3");

  if (!shader) {
    shader = gst_gl_shader_new ();
    g_hash_table_insert (effects->shaderstable, "xray3", shader);
  }

  g_return_if_fail (gst_gl_shader_compile_and_check (shader,
          sobel_fragment_source, GST_GL_SHADER_FRAGMENT_SOURCE));

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  gst_gl_shader_use (shader);

  glActiveTexture (GL_TEXTURE1);
  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glDisable (GL_TEXTURE_RECTANGLE_ARB);

  gst_gl_shader_set_uniform_1i (shader, "tex", 1);

  gst_gl_shader_set_uniform_1fv (shader, "hkern", 9, hkern);
  gst_gl_shader_set_uniform_1fv (shader, "vkern", 9, vkern);

  gst_gl_shader_set_uniform_1i (shader, "invert", TRUE);

  gst_gl_effects_draw_texture (effects, texture);
}

void
gst_gl_effects_xray_step_five (gint width, gint height, guint texture,
    gpointer stuff)
{
  GstGLEffects *effects = GST_GL_EFFECTS (stuff);
  GstGLShader *shader;

  shader = g_hash_table_lookup (effects->shaderstable, "xray4");

  if (!shader) {
    shader = gst_gl_shader_new ();
    g_hash_table_insert (effects->shaderstable, "xray4", shader);
  }

  g_return_if_fail (gst_gl_shader_compile_and_check (shader,
          multiply_fragment_source, GST_GL_SHADER_FRAGMENT_SOURCE));

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  gst_gl_shader_use (shader);

  glActiveTexture (GL_TEXTURE2);
  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, effects->midtexture[2]);
  glDisable (GL_TEXTURE_RECTANGLE_ARB);

  gst_gl_shader_set_uniform_1i (shader, "base", 2);

  glActiveTexture (GL_TEXTURE1);
  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glDisable (GL_TEXTURE_RECTANGLE_ARB);

  gst_gl_shader_set_uniform_1f (shader, "alpha", (gfloat) 0.28f);
  gst_gl_shader_set_uniform_1i (shader, "blend", 1);

  gst_gl_effects_draw_texture (effects, texture);
}

void
gst_gl_effects_xray (GstGLEffects * effects)
{
  GstGLFilter *filter = GST_GL_FILTER (effects);

  /* map luma to xray curve */
  gst_gl_filter_render_to_target (filter, effects->intexture,
      effects->midtexture[0], gst_gl_effects_xray_step_one, effects);
  /* horizontal blur */
  gst_gl_filter_render_to_target (filter, effects->midtexture[0],
      effects->midtexture[1], gst_gl_effects_xray_step_two, effects);
  /* vertical blur */
  gst_gl_filter_render_to_target (filter, effects->midtexture[1],
      effects->midtexture[2], gst_gl_effects_xray_step_three, effects);
  /* detect edges with Sobel */
  gst_gl_filter_render_to_target (filter, effects->midtexture[2],
      effects->midtexture[3], gst_gl_effects_xray_step_four, effects);
  /* multiply edges with the blurred image */
  gst_gl_filter_render_to_target (filter, effects->midtexture[3],
      effects->outtexture, gst_gl_effects_xray_step_five, effects);
}
