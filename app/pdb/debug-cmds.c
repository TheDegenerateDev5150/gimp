/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995-2003 Spencer Kimball and Peter Mattis
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/* NOTE: This file is auto-generated by pdbgen.pl. */

#include "config.h"

#include "stamp-pdbgen.h"

#include <gegl.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libgimpbase/gimpbase.h"

#include "pdb-types.h"

#include "core/gimpparamspecs.h"

#include "gimppdb.h"
#include "gimpprocedure.h"
#include "internal-procs.h"


static GTimer *gimp_debug_timer         = NULL;
static gint    gimp_debug_timer_counter = 0;

static GimpValueArray *
debug_timer_start_invoker (GimpProcedure         *procedure,
                           Gimp                  *gimp,
                           GimpContext           *context,
                           GimpProgress          *progress,
                           const GimpValueArray  *args,
                           GError               **error)
{
  if (gimp_debug_timer_counter++ == 0)
    gimp_debug_timer = g_timer_new ();

  return gimp_procedure_get_return_values (procedure, TRUE, NULL);
}

static GimpValueArray *
debug_timer_end_invoker (GimpProcedure         *procedure,
                         Gimp                  *gimp,
                         GimpContext           *context,
                         GimpProgress          *progress,
                         const GimpValueArray  *args,
                         GError               **error)
{
  gboolean success = TRUE;
  GimpValueArray *return_vals;
  gdouble elapsed = 0.0;

  elapsed = 0.0;

  if (gimp_debug_timer_counter == 0)
    success = FALSE;
  else if (--gimp_debug_timer_counter == 0)
    {
      elapsed = g_timer_elapsed (gimp_debug_timer, NULL);

      g_printerr ("GIMP debug timer: %g seconds\n", elapsed);

      g_timer_destroy (gimp_debug_timer);

      gimp_debug_timer = NULL;
    }

  return_vals = gimp_procedure_get_return_values (procedure, success,
                                                  error ? *error : NULL);

  if (success)
    g_value_set_double (gimp_value_array_index (return_vals, 1), elapsed);

  return return_vals;
}

void
register_debug_procs (GimpPDB *pdb)
{
  GimpProcedure *procedure;

  /*
   * gimp-debug-timer-start
   */
  procedure = gimp_procedure_new (debug_timer_start_invoker, FALSE);
  gimp_object_set_static_name (GIMP_OBJECT (procedure),
                               "gimp-debug-timer-start");
  gimp_procedure_set_static_help (procedure,
                                  "Starts measuring elapsed time.",
                                  "This procedure starts a timer, measuring the elapsed time since the call. Each call to this procedure should be matched by a call to 'gimp-debug-timer-end', which returns the elapsed time.\n"
                                  "If there is already an active timer, it is not affected by the call, however, a matching 'gimp-debug-timer-end' call is still required.\n"
                                  "\n"
                                  "This is a debug utility procedure. It is subject to change at any point, and should not be used in production.",
                                  NULL);
  gimp_procedure_set_static_attribution (procedure,
                                         "Ell",
                                         "Ell",
                                         "2017");
  gimp_pdb_register_procedure (pdb, procedure);
  g_object_unref (procedure);

  /*
   * gimp-debug-timer-end
   */
  procedure = gimp_procedure_new (debug_timer_end_invoker, FALSE);
  gimp_object_set_static_name (GIMP_OBJECT (procedure),
                               "gimp-debug-timer-end");
  gimp_procedure_set_static_help (procedure,
                                  "Finishes measuring elapsed time.",
                                  "This procedure stops the timer started by a previous 'gimp-debug-timer-start' call, and prints and returns the elapsed time.\n"
                                  "If there was already an active timer at the time of corresponding call to 'gimp-debug-timer-start', a dummy value is returned.\n"
                                  "\n"
                                  "This is a debug utility procedure. It is subject to change at any point, and should not be used in production.",
                                  NULL);
  gimp_procedure_set_static_attribution (procedure,
                                         "Ell",
                                         "Ell",
                                         "2017");
  gimp_procedure_add_return_value (procedure,
                                   g_param_spec_double ("elapsed",
                                                        "elapsed",
                                                        "The elapsed time, in seconds",
                                                        -G_MAXDOUBLE, G_MAXDOUBLE, 0,
                                                        GIMP_PARAM_READWRITE));
  gimp_pdb_register_procedure (pdb, procedure);
  g_object_unref (procedure);
}
