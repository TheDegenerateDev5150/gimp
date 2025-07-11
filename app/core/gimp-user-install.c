/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimp-user-install.c
 * Copyright (C) 2000-2008 Michael Natterer and Sven Neumann
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

/* This file contains functions to help migrate the settings from a
 * previous GIMP version to be used with the current (newer) version.
 */

#include "config.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef PLATFORM_OSX
#include <AppKit/AppKit.h>
#endif

#include <gio/gio.h>
#include <glib/gstdio.h>

#ifdef G_OS_WIN32
#include <libgimpbase/gimpwin32-io.h>
#endif

#include "libgimpbase/gimpbase.h"

#include "core-types.h"

#include "config/gimpconfig-file.h"
#include "config/gimprc.h"

#include "gimp-templates.h"
#include "gimp-tags.h"
#include "gimp-user-install.h"

#include "gimp-intl.h"


struct _GimpUserInstall
{
  GObject                 *gimp;

  gboolean                verbose;

  gchar                  *old_dir;
  gint                    old_major;
  gint                    old_minor;

  gint                    scale_factor;

  gboolean                migrate;

  GimpUserInstallLogFunc  log;
  gpointer                log_data;

  GHashTable             *accels;
};

typedef enum
{
  USER_INSTALL_MKDIR, /* Create the directory        */
  USER_INSTALL_COPY   /* Copy from sysconf directory */
} GimpUserInstallAction;

static const struct
{
  const gchar           *name;
  GimpUserInstallAction  action;
}
gimp_user_install_items[] =
{
  { "brushes",         USER_INSTALL_MKDIR },
  { "dynamics",        USER_INSTALL_MKDIR },
  { "fonts",           USER_INSTALL_MKDIR },
  { "gradients",       USER_INSTALL_MKDIR },
  { "palettes",        USER_INSTALL_MKDIR },
  { "patterns",        USER_INSTALL_MKDIR },
  { "tool-presets",    USER_INSTALL_MKDIR },
  { "plug-ins",        USER_INSTALL_MKDIR },
  { "modules",         USER_INSTALL_MKDIR },
  { "interpreters",    USER_INSTALL_MKDIR },
  { "environ",         USER_INSTALL_MKDIR },
  { "scripts",         USER_INSTALL_MKDIR },
  { "templates",       USER_INSTALL_MKDIR },
  { "themes",          USER_INSTALL_MKDIR },
  { "icons",           USER_INSTALL_MKDIR },
  { "tmp",             USER_INSTALL_MKDIR },
  { "curves",          USER_INSTALL_MKDIR },
  { "levels",          USER_INSTALL_MKDIR },
  { "filters",         USER_INSTALL_MKDIR },
  { "fractalexplorer", USER_INSTALL_MKDIR },
  { "gfig",            USER_INSTALL_MKDIR },
  { "gflare",          USER_INSTALL_MKDIR },
  { "gimpressionist",  USER_INSTALL_MKDIR }
};


static gboolean  user_install_detect_old         (GimpUserInstall    *install,
                                                  const gchar        *gimp_dir);
static gchar   * user_install_old_style_gimpdir  (void);

#ifdef G_OS_UNIX
static gchar   * user_install_flatpak_gimpdir    (gint                minor);
#endif

static void      user_install_log                (GimpUserInstall    *install,
                                                  const gchar        *format,
                                                  ...) G_GNUC_PRINTF (2, 3);
static void      user_install_log_newline        (GimpUserInstall    *install);
static void      user_install_log_error          (GimpUserInstall    *install,
                                                  GError            **error);

static gboolean  user_install_mkdir              (GimpUserInstall    *install,
                                                  const gchar        *dirname);
static gboolean  user_install_mkdir_with_parents (GimpUserInstall    *install,
                                                  const gchar        *dirname);
static gboolean  user_install_file_copy          (GimpUserInstall    *install,
                                                  const gchar        *source,
                                                  const gchar        *dest,
                                                  const gchar        *old_options_regexp,
                                                  GRegexEvalCallback  update_callback,
                                                  GimpCopyPostProcess post_process_callback);
static gboolean  user_install_dir_copy           (GimpUserInstall    *install,
                                                  gint                level,
                                                  const gchar        *source,
                                                  const gchar        *base,
                                                  const gchar        *update_pattern,
                                                  GRegexEvalCallback  update_callback,
                                                  GimpCopyPostProcess post_process_callback);

static gboolean  user_install_create_files       (GimpUserInstall    *install);
static gboolean  user_install_migrate_files      (GimpUserInstall    *install);


/*  public functions  */

GimpUserInstall *
gimp_user_install_new (GObject  *gimp,
                       gboolean  verbose)
{
  GimpUserInstall *install = g_slice_new0 (GimpUserInstall);

  install->gimp    = gimp;
  install->verbose = verbose;
  install->accels  = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  user_install_detect_old (install, gimp_directory ());

#ifdef PLATFORM_OSX
  /* The config path on OSX has for a very short time frame (2.8.2 only)
     been "~/Library/GIMP". It changed to "~/Library/Application Support"
     in 2.8.4 and was in the home folder (as was other UNIX) before. */

  if (! install->old_dir)
    {
      gchar             *dir;
      NSAutoreleasePool *pool;
      NSArray           *path;
      NSString          *library_dir;

      pool = [[NSAutoreleasePool alloc] init];

      path = NSSearchPathForDirectoriesInDomains (NSLibraryDirectory,
                                                  NSUserDomainMask, YES);
      library_dir = [path objectAtIndex:0];

      dir = g_build_filename ([library_dir UTF8String],
                              GIMPDIR, GIMP_USER_VERSION, NULL);

      [pool drain];

      user_install_detect_old (install, dir);
      g_free (dir);
    }

#endif

  if (! install->old_dir)
    {
      /* if the default XDG-style config directory was not found, try
       * the "old-style" path in the home folder.
       */
      gchar *dir = user_install_old_style_gimpdir ();
      user_install_detect_old (install, dir);
      g_free (dir);
    }

  return install;
}

gboolean
gimp_user_install_run (GimpUserInstall *install,
                       gint             scale_factor)
{
  gchar *dirname;

  g_return_val_if_fail (install != NULL, FALSE);

  install->scale_factor = scale_factor;
  dirname = g_filename_display_name (gimp_directory ());

  if (install->migrate)
    {
      gchar *verstring;

      /* TODO: these 2 strings should be merged into one, but it was not
       * possible to do it at implementation time, in order not to break
       * string freeze.
       */
      verstring = g_strdup_printf ("%d.%d", install->old_major, install->old_minor);
      user_install_log (install,
                        _("It seems you have used GIMP %s before.  "
                          "GIMP will now migrate your user settings to '%s'."),
                        verstring, dirname);
      g_free (verstring);
    }
  else
    {
      user_install_log (install,
                        _("It appears that you are using GIMP for the "
                          "first time.  GIMP will now create a folder "
                          "named '%s' and copy some files to it."),
                        dirname);
    }

  g_free (dirname);

  user_install_log_newline (install);

  if (! user_install_mkdir_with_parents (install, gimp_directory ()))
    return FALSE;

  if (install->migrate)
    if (! user_install_migrate_files (install))
      return FALSE;

  return user_install_create_files (install);
}

void
gimp_user_install_free (GimpUserInstall *install)
{
  g_return_if_fail (install != NULL);

  g_free (install->old_dir);
  g_hash_table_destroy (install->accels);

  g_slice_free (GimpUserInstall, install);
}

void
gimp_user_install_set_log_handler (GimpUserInstall        *install,
                                   GimpUserInstallLogFunc  log,
                                   gpointer                user_data)
{
  g_return_if_fail (install != NULL);

  install->log      = log;
  install->log_data = user_data;
}


/*  Local functions  */

static gboolean
user_install_detect_old (GimpUserInstall *install,
                         const gchar     *gimp_dir)
{
  gchar    *dir     = g_strconcat (gimp_dir, "ZZZ", NULL);
  gchar    *version;
  gboolean  migrate = FALSE;

  version = strstr (dir, GIMP_APP_VERSION);

  if (version)
    {
      gint major;
      gint minor;

      for (major = GIMP_MAJOR_VERSION; major >= 2; major--)
        {
          gint max_minor;

          g_snprintf (version, 5, "%d.XY", major);

          switch (major)
            {
            case 3:
              max_minor = GIMP_MINOR_VERSION;
              break;
            case 2:
              max_minor = 10;
              break;
            default:
              g_return_val_if_reached (FALSE);
            }

          for (minor = (max_minor & ~1); minor >= 0; minor -= 2)
            {
              /*  we assume that GIMP_APP_VERSION is in the form '2.x'  */
              g_snprintf (version + 2, 3, "%d", minor);

              migrate = g_file_test (dir, G_FILE_TEST_IS_DIR);

              if (migrate)
                {
                  install->old_major = major;
                  install->old_minor = minor;

                  break;
                }

#ifdef G_OS_UNIX
              if (major == 2 && minor == 10)
                {
                  /* This is special-casing for GIMP 2.10 as flatpak where
                   * we had this weird inconsistency: depending on whether a
                   * previous $XDG_CONFIG_HOME/GIMP/ folder was found or
                   * not, the config folder would be either inside, or would
                   * be in $HOME/.var/<etc> as other flatpak (see #5331).
                   * For GIMP 3, even the flatpak will always be in
                   * $XDG_CONFIG_HOME. But then we want a migration to still
                   * find the previous config folder.
                   * If one was found in $XDG_CONFIG_HOME, it is used in
                   * priority. Then ~/.var/ is used a fallback, if found.
                   */
                  gchar *flatpak_dir = user_install_flatpak_gimpdir (minor);

                  if (flatpak_dir)
                    /* This first test is for finding a 2.10 flatpak config
                     * dir from a non-flatpak GIMP 3+.
                     */
                    migrate = g_file_test (flatpak_dir, G_FILE_TEST_IS_DIR);

                  if (! migrate && g_file_test ("/.flatpak-info", G_FILE_TEST_EXISTS))
                    {
                      /* Now we check /var/config/GIMP because this is where
                       * local ~/.var/ is mounted inside the sandbox.
                       * So this second test is for finding a 2.10 flatpak
                       * config dir from a flatpak GIMP 3+.
                       */
                      g_free (flatpak_dir);
                      flatpak_dir = g_build_filename ("/var/config/GIMP/", version, NULL);

                      migrate = g_file_test (flatpak_dir, G_FILE_TEST_IS_DIR);
                    }

                  if (migrate)
                    {
                      install->old_major = 2;
                      install->old_minor = minor;

                      g_free (dir);
                      dir = flatpak_dir;
                      break;
                    }
                  else
                    {
                      g_free (flatpak_dir);
                    }
                }
#endif
            }
          if (migrate)
            break;
        }
    }

  install->migrate = migrate;
  if (migrate)
    {
      install->old_dir = dir;
#ifdef GIMP_UNSTABLE
      g_printerr ("gimp-user-install: migrating from %s\n", dir);
#endif
    }
  else
    {
      g_free (dir);
    }

  return migrate;
}

static gchar *
user_install_old_style_gimpdir (void)
{
  const gchar *home_dir = g_get_home_dir ();
  gchar       *gimp_dir = NULL;

  if (home_dir)
    {
      gimp_dir = g_build_filename (home_dir, ".gimp-" GIMP_APP_VERSION, NULL);
    }
  else
    {
      gchar *user_name = g_strdup (g_get_user_name ());
      gchar *subdir_name;

#ifdef G_OS_WIN32
      gchar *p = user_name;

      while (*p)
        {
          /* Replace funny characters in the user name with an
           * underscore. The code below also replaces some
           * characters that in fact are legal in file names, but
           * who cares, as long as the definitely illegal ones are
           * caught.
           */
          if (!g_ascii_isalnum (*p) && !strchr ("-.,@=", *p))
            *p = '_';
          p++;
        }
#endif

#ifndef G_OS_WIN32
      g_message ("warning: no home directory.");
#endif
      subdir_name = g_strconcat (".gimp-" GIMP_APP_VERSION ".", user_name, NULL);
      gimp_dir = g_build_filename (gimp_data_directory (),
                                   subdir_name,
                                   NULL);
      g_free (user_name);
      g_free (subdir_name);
    }

  return gimp_dir;
}

#ifdef G_OS_UNIX
static gchar *
user_install_flatpak_gimpdir (gint minor)
{
  const gchar *home_dir = g_get_home_dir ();
  gchar       *version  = g_strdup_printf ("2.%d", minor);
  gchar       *gimp_dir = NULL;

  if (home_dir)
    /* AFAIK (after researching flatpak docs, and searching the web),
     * the ~/.var/ directory is hardcoded and cannot be modified by an
     * environment variable.
     */
    gimp_dir = g_build_filename (home_dir,
                                 ".var/app/org.gimp.GIMP/config/GIMP/",
                                 version, NULL);

  g_free (version);

  return gimp_dir;
}
#endif

static void
user_install_log (GimpUserInstall *install,
                  const gchar     *format,
                  ...)
{
  va_list args;

  va_start (args, format);

  if (format)
    {
      gchar *message = g_strdup_vprintf (format, args);

      if (install->verbose)
        g_print ("%s\n", message);

      if (install->log)
        install->log (message, FALSE, install->log_data);

      g_free (message);
    }

  va_end (args);
}

static void
user_install_log_newline (GimpUserInstall *install)
{
  if (install->verbose)
    g_print ("\n");

  if (install->log)
    install->log (NULL, FALSE, install->log_data);
}

static void
user_install_log_error (GimpUserInstall  *install,
                        GError          **error)
{
  if (error && *error)
    {
      const gchar *message = ((*error)->message ?
                              (*error)->message : "(unknown error)");

      if (install->log)
        install->log (message, TRUE, install->log_data);
      else
        g_print ("error: %s\n", message);

      g_clear_error (error);
    }
}

static gboolean
user_install_file_copy (GimpUserInstall    *install,
                        const gchar        *source,
                        const gchar        *dest,
                        const gchar        *old_options_regexp,
                        GRegexEvalCallback  update_callback,
                        GimpCopyPostProcess post_process_callback)
{
  GError   *error = NULL;
  gboolean  success;

  user_install_log (install, _("Copying file '%s' from '%s'..."),
                    gimp_filename_to_utf8 (dest),
                    gimp_filename_to_utf8 (source));

  success = gimp_config_file_copy (source, dest, old_options_regexp,
                                   update_callback, post_process_callback,
                                   install, &error);

  user_install_log_error (install, &error);

  return success;
}

static gboolean
user_install_mkdir (GimpUserInstall *install,
                    const gchar     *dirname)
{
  user_install_log (install, _("Creating folder '%s'..."),
                    gimp_filename_to_utf8 (dirname));

  if (g_mkdir (dirname,
               S_IRUSR | S_IWUSR | S_IXUSR |
               S_IRGRP | S_IXGRP |
               S_IROTH | S_IXOTH) == -1)
    {
      GError *error = NULL;

      g_set_error (&error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Cannot create folder '%s': %s"),
                   gimp_filename_to_utf8 (dirname), g_strerror (errno));

      user_install_log_error (install, &error);

      return FALSE;
    }

  return TRUE;
}

static gboolean
user_install_mkdir_with_parents (GimpUserInstall *install,
                                 const gchar     *dirname)
{
  user_install_log (install, _("Creating folder '%s'..."),
                    gimp_filename_to_utf8 (dirname));

  if (g_mkdir_with_parents (dirname,
                            S_IRUSR | S_IWUSR | S_IXUSR |
                            S_IRGRP | S_IXGRP |
                            S_IROTH | S_IXOTH) == -1)
    {
      GError *error = NULL;

      g_set_error (&error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Cannot create folder '%s': %s"),
                   gimp_filename_to_utf8 (dirname), g_strerror (errno));

      user_install_log_error (install, &error);

      return FALSE;
    }

  return TRUE;
}

/* The regexp pattern of all options changed from menurc of GIMP 2.8.
 * Add any pattern that we want to recognize for replacement in the menurc of
 * the next release
 */
#define MENURC_OVER20_UPDATE_PATTERN \
  "(;)? *\\(gtk_accel_path \"<Actions>/[a-zA-Z0-9-]*/([a-zA-Z0-9-]*)\" *\"([a-zA-Z0-9<>_]*)\"\\)" "|" \
  "(;.*)"

/**
 * callback to use for updating a menurc from GIMP over 2.0.
 * data is unused (always NULL).
 * The updated value will be matched line by line.
 */
static gboolean
user_update_menurc_over20 (const GMatchInfo *matched_value,
                           GString          *new_value,
                           gpointer          data)
{
  GimpUserInstall *install         = (GimpUserInstall *) data;
  gchar           *comment_match   = g_match_info_fetch (matched_value, 1);
  gchar           *action_match    = g_match_info_fetch (matched_value, 2);
  gchar           *accel_match     = g_match_info_fetch (matched_value, 3);
  gchar           *ignore_match    = g_match_info_fetch (matched_value, 4);
  gchar           *new_action_name = NULL;
  gboolean         accel_variant   = FALSE;

  if (strlen (ignore_match) == 0)
    {
      /* "*-paste-as-new" renamed to "*-paste-as-new-image" */
      if (g_strcmp0 (action_match, "buffers-paste-as-new") == 0)
        new_action_name = g_strdup ("buffers-paste-as-new-image");
      else if (g_strcmp0 (action_match, "edit-paste-as-new") == 0)
        new_action_name = g_strdup ("edit-paste-as-new-image");
      /* file-export-* changes to follow file-save-* patterns.  Actions
       * available since GIMP 2.8, changed for 2.10 in commit 4b14ed2.
       */
      else if (g_strcmp0 (action_match, "file-export") == 0 &&
               install->old_major == 2 && install->old_minor <= 8)
        new_action_name = g_strdup ("file-export-as");
      else if (g_strcmp0 (action_match, "file-export-to") == 0 &&
               install->old_major == 2 && install->old_minor <= 8)
        new_action_name = g_strdup ("file-export");
      else if (g_strcmp0 (action_match, "layers-text-tool") == 0)
        new_action_name = g_strdup ("layers-edit");
      /* plug-in-gauss doesn't exist anymore since commit ff59aebbe88.
       * The expected replacement would be filters-gaussian-blur which is
       * gegl:gaussian-blur operation. See also bug 775931.
       */
      else if (g_strcmp0 (action_match, "plug-in-gauss") == 0)
        new_action_name = g_strdup ("filters-gaussian-blur");
      /* Tools settings renamed more user-friendly.  Actions available
       * since GIMP 2.4, changed for 2.10 in commit 0bdb747.
       */
      else if (g_str_has_prefix (action_match, "tools-value-1-"))
        new_action_name = g_strdup_printf ("tools-opacity-%s", action_match + 14);
      else if (g_str_has_prefix (action_match, "tools-value-2-"))
        new_action_name = g_strdup_printf ("tools-size-%s", action_match + 14);
      else if (g_str_has_prefix (action_match, "tools-value-3-"))
        new_action_name = g_strdup_printf ("tools-aspect-%s", action_match + 14);
      else if (g_str_has_prefix (action_match, "tools-value-4-"))
        new_action_name = g_strdup_printf ("tools-angle-%s", action_match + 14);
      else if (g_strcmp0 (action_match, "tools-blend") == 0)
        new_action_name = g_strdup ("tools-gradient");
      else if (g_strcmp0 (action_match, "vectors-path-tool") == 0)
        new_action_name = g_strdup ("paths-edit");
      /* Following the s/GimpVectors/GimpPath/ renaming to be more
       * consistent with the GUI, we also rename all the action names.
       * Since GIMP 3.0, commit XXXX.
       */
      else if (g_strcmp0 (action_match, "vectors-selection-from-vectors") == 0)
        new_action_name = g_strdup ("paths-selection-from-paths");
      else if (g_str_has_prefix (action_match, "vectors-selection-to-vectors"))
        new_action_name = g_strdup_printf ("paths-selection-to-path%s",
                                           action_match + strlen ("vectors-selection-to-vectors"));
      else if (g_str_has_prefix (action_match, "vectors-"))
        new_action_name = g_strdup_printf ("paths-%s", action_match + 8);

      /* view-rotate-reset became view-reset and new view-rotate-reset and
       * view-flip-reset actions were created.  See commit 15fb4a7be0.
       */
      else if (g_strcmp0 (action_match, "view-rotate-reset") == 0 &&
               install->old_major == 2)
        new_action_name = g_strdup ("view-reset");
      /* select-float became select-cut-float in 3.0 (select-copy-float added). */
      else if (g_strcmp0 (action_match, "select-float") == 0)
        new_action_name = g_strdup ("select-cut-float");
      /* edit-paste-as-new-layer* actions removed in 3.0.0 (commit
       * 2c4f91f585) because the default "edit-paste" pastes as new layer.
       *
       * We used to migrate these to "edit-paste" and
       * "edit-paste-in-place", but it meant that it would override the
       * default shortcut (ctrl-v) for the basic paste, which may be
       * counter-productive (cf. #13414). Furthermore, if you had a
       * shortcut to "edit-paste*" actions (since these existed in 2.10
       * too), you'd have a shortcut conflict.
       *
       * Instead since GIMP 3.0.4, we migrate them to "edit-paste-merged*"
       * (introduced in 143496af22) though it's not an exact equivalent
       * (but neither were "edit-paste*"), in particular:
       *
       * - If a layer mask is selected, "edit-paste-merged*" would still
       *   create a floating mask (unlike old "edit-paste-as-new-layer*"
       *   which was always creating a new layer).
       * - Even when multiple layers are copied, the paste is always a
       *   single layer. But the latter is not a big issue since in GIMP
       *   2.x, you could not copy multiple layers anyway, so in this
       *   aspect, we may say it works the same.
       */
      else if (g_strcmp0 (action_match, "edit-paste-as-new-layer") == 0)
        new_action_name = g_strdup ("edit-paste-merged");
      else if (g_strcmp0 (action_match, "edit-paste-as-new-layer-in-place") == 0)
        new_action_name = g_strdup ("edit-paste-merged-in-place");
      /* These actions had an -accel variant which got removed in commit
       * 71c8ff1f21. Since we cannot know if both variants were given a
       * custom shortcut when processing per-line, we temporarily store
       * them all and will do a second pass allowing us to store one or
       * both shortcuts if needed.
       */
      else if (g_str_has_suffix (action_match, "-accel")  ||
               g_strcmp0 (action_match, "view-zoom-out")  == 0 ||
               g_strcmp0 (action_match, "view-zoom-in")   == 0 ||
               g_strcmp0 (action_match, "view-zoom-16-1") == 0 ||
               g_strcmp0 (action_match, "view-zoom-8-1")  == 0 ||
               g_strcmp0 (action_match, "view-zoom-4-1")  == 0 ||
               g_strcmp0 (action_match, "view-zoom-2-1")  == 0 ||
               g_strcmp0 (action_match, "view-zoom-1-1")  == 0)
        accel_variant = TRUE;

      if (new_action_name == NULL)
        new_action_name = g_strdup (action_match);

      if (g_strcmp0 (comment_match, ";") == 0)
        {
          g_string_append (new_value, "# ");
        }
      else if (accel_variant)
        {
          g_hash_table_insert (install->accels, action_match, accel_match);
          action_match = NULL;
          accel_match  = NULL;
        }

      if (action_match)
        {
          if (strlen (accel_match) > 0)
            g_string_append_printf (new_value, "(action \"%s\" \"%s\")",
                                    new_action_name, accel_match);
          else
            g_string_append_printf (new_value, "(action \"%s\")",
                                    new_action_name);
        }
    }

  g_free (comment_match);
  g_free (action_match);
  g_free (accel_match);
  g_free (ignore_match);
  g_free (new_action_name);

  return FALSE;
}

static gchar *
user_update_post_process_menurc_over20 (gpointer user_data)
{
  GString         *string  = g_string_new (NULL);
  GimpUserInstall *install = (GimpUserInstall *) user_data;

  static gchar    * gimp_2_accels[][3] =
  {
    { "view-zoom-out",  "minus", "KP_Subtract" },
    { "view-zoom-in",   "plus",  "KP_Add" },
    { "view-zoom-16-1", "5",     "KP_5" },
    { "view-zoom-8-1",  "4",     "KP_4" },
    { "view-zoom-4-1",  "3",     "KP_3" },
    { "view-zoom-2-1",  "2",     "KP_2" },
    { "view-zoom-1-1",  "1",     "KP_1" }
  };

  for (gint i = 0; i < G_N_ELEMENTS (gimp_2_accels); i++)
    {
      const gchar *action = gimp_2_accels[i][0];
      gchar       *action_variant = g_strconcat (action, "-accel", NULL);
      gchar       *accel;
      gchar       *accel_variant;

      accel         = g_hash_table_lookup (install->accels, action);
      accel_variant = g_hash_table_lookup (install->accels, action_variant);
      if (accel != NULL && strlen (accel) > 0 &&
          accel_variant != NULL && strlen (accel_variant) > 0)
        {
          g_string_append_printf (string, "\n(action \"%s\" \"%s\" \"%s\")",
                                  action, accel, accel_variant);
        }
      else if (accel != NULL)
        {
          if (strlen (accel) > 0)
            {
              if (accel_variant == NULL)
                g_string_append_printf (string, "\n(action \"%s\" \"%s\" \"%s\")", action, accel, gimp_2_accels[i][2]);
              else
                g_string_append_printf (string, "\n(action \"%s\" \"%s\")", action, accel);
            }
          else if (accel_variant != NULL)
            {
              if (strlen (accel_variant) > 0)
                g_string_append_printf (string, "\n(action \"%s\" \"%s\")", action, accel_variant);
              else
                g_string_append_printf (string, "\n(action \"%s\")", action);
            }
          else
            {
              g_string_append_printf (string, "\n(action \"%s\" \"%s\")", action, gimp_2_accels[i][2]);
            }
        }
      else if (accel_variant != NULL)
        {
          if (strlen (accel_variant) > 0)
            {
              g_string_append_printf (string, "\n(action \"%s\" \"%s\" \"%s\")", action, accel_variant, gimp_2_accels[i][1]);
            }
          else
            {
              g_string_append_printf (string, "\n(action \"%s\" \"%s\")", action, gimp_2_accels[i][1]);
            }
        }

      g_free (action_variant);
    }

  return g_string_free (string, FALSE);
}

#define SHORTCUTSRC_UPDATE_PATTERN \
  "\"("                            \
  "tools-vector|"                  \
  "dialogs-vectors|"               \
  "layers-text-.*-vectors|"        \
  "view-snap-to-vectors"           \
  ")\""

static gboolean
user_update_shortcutsrc (const GMatchInfo *matched_value,
                         GString          *new_value,
                         gpointer          data)
{
  gchar *match = g_match_info_fetch (matched_value, 0);

  if (g_strcmp0 (match, "\"tools-vector\"") == 0)
    {
      /* Renamed in GIMP 3.2. */
      g_string_append (new_value, "\"tools-path\"");
    }
  else if (g_strcmp0 (match, "\"dialogs-vectors\"") == 0)
    {
      /* Renamed in GIMP 3.2. */
      g_string_append (new_value, "\"dialogs-path\"");
    }
  else if (g_strcmp0 (match, "\"layers-text-along-vectors\"") == 0)
    {
      /* Renamed in GIMP 3.2. */
      g_string_append (new_value, "\"layers-text-along-path\"");
    }
  else if (g_strcmp0 (match, "\"layers-text-to-vectors\"") == 0)
    {
      /* Renamed in GIMP 3.2. */
      g_string_append (new_value, "\"layers-text-to-path\"");
    }
  else if (g_strcmp0 (match, "\"view-snap-to-vectors\"") == 0)
    {
      /* Renamed in GIMP 3.2. */
      g_string_append (new_value, "\"view-snap-to-path\"");
    }

  g_free (match);

  return FALSE;
}

#define TEMPLATERC_UPDATE_PATTERN \
  "\\(precision (.*)-gamma\\)"

static gboolean
user_update_templaterc (const GMatchInfo *matched_value,
                        GString          *new_value,
                        gpointer          data)
{
  gchar *original = g_match_info_fetch (matched_value, 0);
  gchar *match    = g_match_info_fetch (matched_value, 1);

  /* GIMP_PRECISION_*_GAMMA removed in GIMP 3.0.0 (commit 2559138931). */
  g_string_append_printf (new_value, "(precision %s-non-linear)", match);

  g_free (original);
  g_free (match);

  return FALSE;
}

#define CONTROLLERRC_UPDATE_PATTERN \
  "\\(map \"(scroll|cursor)-[^\"]*\\bcontrol\\b[^\"]*\""

static gboolean
user_update_controllerrc (const GMatchInfo *matched_value,
                          GString          *new_value,
                          gpointer          data)
{
  gchar  *original;
  gchar  *replacement;
  GRegex *regexp = NULL;

  /* No need of a complicated pattern here.
   * CONTROLLERRC_UPDATE_PATTERN took care of it first.
   */
  regexp   = g_regex_new ("\\bcontrol\\b", 0, 0, NULL);
  original = g_match_info_fetch (matched_value, 0);

  replacement = g_regex_replace (regexp, original, -1, 0,
                                 "primary", 0, NULL);
  g_string_append (new_value, replacement);

  g_free (original);
  g_free (replacement);
  g_regex_unref (regexp);

  return FALSE;
}

#define SESSIONRC_UPDATE_PATTERN_2TO3 \
  "\\(position [0-9]* [0-9]*\\)"         "|" \
  "\\(size [0-9]* [0-9]*\\)"             "|" \
  "\\(left-docks-width \"?[0-9]*\"?\\)"  "|" \
  "\\(right-docks-width \"?[0-9]*\"?\\)"

static gboolean
user_update_sessionrc_2to3 (const GMatchInfo *matched_value,
                            GString          *new_value,
                            gpointer          data)
{
  GimpUserInstall *install = (GimpUserInstall *) data;
  gchar           *original;

  original = g_match_info_fetch (matched_value, 0);

  if (install->scale_factor != 1 && install->scale_factor > 0)
    {
      /* GTK < 3.0 didn't have scale factor support. It means that any
       * size and position back then would be in real pixel size. Now
       * with GTK3 and over, we need to think of position and size in
       * virtual/application pixels.
       * In particular it means that if we were to just copy the numbers
       * from GTK2 to GTK3 on a display with scale factor of 2, every
       * width, height and position would be 2 times too big (a full
       * screen window would end up off-screen).
       */
      GRegex     *regexp;
      GMatchInfo *match_info;
      gchar      *match;

      /* First copy the pattern title. */
      regexp = g_regex_new ("\\([a-z-]* ", 0, 0, NULL);
      g_regex_match (regexp, original, 0, &match_info);
      match = g_match_info_fetch (match_info, 0);
      g_string_append (new_value, match);

      g_match_info_free (match_info);
      g_regex_unref (regexp);
      g_free (match);

      /* Now copy the numbers. */
      regexp = g_regex_new ("[0-9]+|\"", 0, 0, NULL);
      g_regex_match (regexp, original, 0, &match_info);

      while (g_match_info_matches (match_info))
        {
          match = g_match_info_fetch (match_info, 0);
          if (g_strcmp0 (match, "\"") == 0)
            {
              g_string_append (new_value, match);
            }
          else
            {
              gint num;

              num = g_ascii_strtoll (match, NULL, 10);
              num /= install->scale_factor;

              g_string_append_printf (new_value, " %d", num);
            }

          g_free (match);
          g_match_info_next (match_info, NULL);
        }

      g_match_info_free (match_info);
      g_regex_unref (regexp);

      g_string_append (new_value, ")");
    }
  else
    {
      /* Just copy as-is. */
      g_string_append (new_value, original);
    }

  g_free (original);

  return FALSE;
}

#define TOOLRC_UPDATE_PATTERN \
  "\"gimp-vector-tool\""

static gboolean
user_update_toolrc (const GMatchInfo *matched_value,
                    GString          *new_value,
                    gpointer          data)
{
  gchar *original;

  original = g_match_info_fetch (matched_value, 0);

  if (g_strcmp0 (original, "\"gimp-vector-tool\"") == 0)
    {
      /* Renamed in GIMP 3.2. */
      g_string_append (new_value, "\"gimp-path-tool\"");
    }

  g_free (original);

  return FALSE;
}

#define SESSIONRC_UPDATE_PATTERN \
  "\"gimp-vectors-list\""

static gboolean
user_update_sessionrc (const GMatchInfo *matched_value,
                       GString          *new_value,
                       gpointer          data)
{
  gchar *original;

  original = g_match_info_fetch (matched_value, 0);

  if (g_strcmp0 (original, "\"gimp-vectors-list\"") == 0)
    {
      /* Renamed in GIMP 3.2. */
      g_string_append (new_value, "\"gimp-path-list\"");
    }

  g_free (original);

  return FALSE;
}

#define GIMPRC_UPDATE_PATTERN \
  "\\(show-tooltips [^)]*\\)"  "|" \
  "\\(theme [^)]*\\)"          "|" \
  "^ *\\(.*-path \".*\"\\) *$" "|" \
  "\\(style solid\\)"          "|" \
  "\\(precision (.*)-gamma\\)" "|" \
  "\\(filter-tool-show-color-options [^)]*\\)"

static gboolean
user_update_gimprc (const GMatchInfo *matched_value,
                    GString          *new_value,
                    gpointer          data)
{
  gchar *match = g_match_info_fetch (matched_value, 0);

  if (g_strcmp0 (match, "(style solid)") == 0)
    {
      /* See MR !706: GIMP_FILL_STYLE_SOLID was split in
       * GIMP_FILL_STYLE_FG_COLOR and GIMP_FILL_STYLE_BG_COLOR.
       */
      g_string_append (new_value, "(style fg-color)");
    }
  else if (g_str_has_prefix (match, "(precision "))
    {
      gchar *precision_match = g_match_info_fetch (matched_value, 1);

      /* GIMP_PRECISION_*_GAMMA removed in GIMP 3.0.0 (commit 2559138931). */
      g_string_append_printf (new_value, "(precision %s-non-linear)", precision_match);

      g_free (precision_match);
    }
  else
    {
      /* Do not migrate show-tooltips GIMP < 3.0. Cf. #1965. */

      /* Do not migrate paths and themes from GIMP < 3.0. */

      /* Do not migrate the advanced color options which was the gamma
       * hack removed for GIMP 3.0. Cf. #12577.
       */
    }

  g_free (match);
  return FALSE;
}

#define GIMPRESSIONIST_UPDATE_PATTERN \
  "selectedbrush=Brushes/paintbrush.pgm"

static gboolean
user_update_gimpressionist (const GMatchInfo *matched_value,
                            GString          *new_value,
                            gpointer          data)
{
  gchar *match = g_match_info_fetch (matched_value, 0);

  /* See bug 791934: both brushes are identical. */
  if (g_strcmp0 (match, "selectedbrush=Brushes/paintbrush.pgm") == 0)
    {
      g_string_append (new_value, "selectedbrush=Brushes/paintbrush01.pgm");
    }
  else
    {
      g_message ("(WARNING) %s: invalid match \"%s\"", G_STRFUNC, match);
      g_string_append (new_value, match);
    }

  g_free (match);
  return FALSE;
}

#define TOOL_PRESETS_UPDATE_PATTERN \
  "GimpImageMapOptions"       "|" \
  "GimpBlendOptions"          "|" \
  "gimp-blend-tool"           "|" \
  "gimp-tool-blend"           "|" \
  "dynamics \"Dynamics Off\"" "|" \
  "\\(dynamics-expanded yes\\)"

static gboolean
user_update_tool_presets (const GMatchInfo *matched_value,
                          GString          *new_value,
                          gpointer          data)
{
  gchar *match = g_match_info_fetch (matched_value, 0);

  if (g_strcmp0 (match, "GimpImageMapOptions") == 0)
    {
      g_string_append (new_value, "GimpFilterOptions");
    }
  else if (g_strcmp0 (match, "GimpBlendOptions") == 0)
    {
      g_string_append (new_value, "GimpGradientOptions");
    }
  else if (g_strcmp0 (match, "gimp-blend-tool") == 0)
    {
      g_string_append (new_value, "gimp-gradient-tool");
    }
  else if (g_strcmp0 (match, "gimp-tool-blend") == 0)
    {
      g_string_append (new_value, "gimp-tool-gradient");
    }
  else if (g_strcmp0 (match, "dynamics \"Dynamics Off\"") == 0)
    {
      g_string_append (new_value, "dynamics-enabled no");
    }
  else if (g_strcmp0 (match, "(dynamics-expanded yes)") == 0)
    {
      /* This option just doesn't exist anymore. */
    }
  else
    {
      g_message ("(WARNING) %s: invalid match \"%s\"", G_STRFUNC, match);
      g_string_append (new_value, match);
    }

  g_free (match);
  return FALSE;
}

/* Actually not only for contextrc, but all other files where
 * gimp-blend-tool may appear. Apparently that is also "devicerc", as
 * well as "toolrc" (but this one is skipped anyway).
 */
#define CONTEXTRC_UPDATE_PATTERN \
  "gimp-blend-tool"             "|" \
  "dynamics \"Dynamics Off\""   "|" \
  "\\(dynamics-expanded yes\\)" "|" \
  "\\(color-options-expanded [^)]*\\)"

static gboolean
user_update_contextrc_over20 (const GMatchInfo *matched_value,
                              GString          *new_value,
                              gpointer          data)
{
  gchar *match = g_match_info_fetch (matched_value, 0);

  if (g_strcmp0 (match, "gimp-blend-tool") == 0)
    {
      g_string_append (new_value, "gimp-gradient-tool");
    }
  else if (g_strcmp0 (match, "dynamics \"Dynamics Off\"") == 0)
    {
      g_string_append (new_value, "dynamics-enabled no");
    }
  else if (g_strcmp0 (match, "(dynamics-expanded yes)") == 0)
    {
      /* This option just doesn't exist anymore. */
    }
  else if (g_str_has_prefix (match, "(color-options-expanded "))
    {
      /* This option was removed with the gamma-hack. Cf. #12577. */
    }
  else
    {
      g_message ("(WARNING) %s: invalid match \"%s\"", G_STRFUNC, match);
      g_string_append (new_value, match);
    }

  g_free (match);
  return FALSE;
}

static gboolean
user_install_dir_copy (GimpUserInstall    *install,
                       gint                level,
                       const gchar        *source,
                       const gchar        *base,
                       const gchar        *update_pattern,
                       GRegexEvalCallback  update_callback,
                       GimpCopyPostProcess post_process_callback)
{
  GDir        *source_dir = NULL;
  GDir        *dest_dir   = NULL;
  gchar        dest[1024];
  const gchar *basename;
  gchar       *dirname = NULL;
  gchar       *name;
  GError      *error   = NULL;
  gboolean     success = FALSE;

  if (level >= 5)
    {
      /* Config migration is recursive, but we can't go on forever,
       * since we may fall into recursive symlinks in particular (which
       * is a security risk to fill a disk, and would also block GIMP
       * forever at migration stage).
       * Let's just break the recursivity at 5 levels, which is just an
       * arbitrary value (but I don't think there should be any data
       * deeper than this).
       */
      goto error;
    }

  name = g_path_get_basename (source);
  dirname = g_build_filename (base, name, NULL);
  g_free (name);

  success = user_install_mkdir (install, dirname);
  if (! success)
    goto error;

  success = (dest_dir = g_dir_open (dirname, 0, &error)) != NULL;
  if (! success)
    goto error;

  success = (source_dir = g_dir_open (source, 0, &error)) != NULL;
  if (! success)
    goto error;

  while ((basename = g_dir_read_name (source_dir)) != NULL)
    {
      name = g_build_filename (source, basename, NULL);

      if (g_file_test (name, G_FILE_TEST_IS_REGULAR))
        {
          g_snprintf (dest, sizeof (dest), "%s%c%s",
                      dirname, G_DIR_SEPARATOR, basename);

          success = user_install_file_copy (install, name, dest,
                                            update_pattern,
                                            update_callback,
                                            post_process_callback);
          if (! success)
            {
              g_free (name);
              goto error;
            }
        }
      else
        {
          user_install_dir_copy (install, level + 1, name, dirname,
                                 update_pattern, update_callback,
                                 post_process_callback);
        }

      g_free (name);
    }

 error:
  user_install_log_error (install, &error);

  if (source_dir)
    g_dir_close (source_dir);

  if (dest_dir)
    g_dir_close (dest_dir);

  if (dirname)
    g_free (dirname);

  return success;
}

static gboolean
user_install_create_files (GimpUserInstall *install)
{
  gchar dest[1024];
  gchar source[1024];
  gint  i;

  for (i = 0; i < G_N_ELEMENTS (gimp_user_install_items); i++)
    {
      g_snprintf (dest, sizeof (dest), "%s%c%s",
                  gimp_directory (),
                  G_DIR_SEPARATOR,
                  gimp_user_install_items[i].name);

      if (g_file_test (dest, G_FILE_TEST_EXISTS))
        continue;

      switch (gimp_user_install_items[i].action)
        {
        case USER_INSTALL_MKDIR:
          if (! user_install_mkdir (install, dest))
            return FALSE;
          break;

        case USER_INSTALL_COPY:
          g_snprintf (source, sizeof (source), "%s%c%s",
                      gimp_sysconf_directory (), G_DIR_SEPARATOR,
                      gimp_user_install_items[i].name);

          if (! user_install_file_copy (install, source, dest, NULL, NULL, NULL))
            return FALSE;
          break;
        }
    }

  g_snprintf (dest, sizeof (dest), "%s%c%s",
              gimp_directory (), G_DIR_SEPARATOR, "tags.xml");

  if (! g_file_test (dest, G_FILE_TEST_IS_REGULAR))
    {
      /* if there was no tags.xml, install it with default tag set.
       */
      if (! gimp_tags_user_install ())
        {
          return FALSE;
        }
    }

  return TRUE;
}

static gboolean
user_install_migrate_files (GimpUserInstall *install)
{
  GDir        *dir;
  const gchar *basename;
  gchar        dest[1024];
  GimpRc      *gimprc;
  GError      *error = NULL;

  dir = g_dir_open (install->old_dir, 0, &error);

  if (! dir)
    {
      user_install_log_error (install, &error);
      return FALSE;
    }

  while ((basename = g_dir_read_name (dir)) != NULL)
    {
      gchar       *source   = g_build_filename (install->old_dir, basename, NULL);
      const gchar *new_dest = NULL;

      if (g_file_test (source, G_FILE_TEST_IS_REGULAR))
        {
          const gchar        *update_pattern        = NULL;
          GRegexEvalCallback  update_callback       = NULL;
          GimpCopyPostProcess post_process_callback = NULL;

          /*  skip these files for all old versions  */
          if (strcmp (basename, "documents") == 0      ||
              g_str_has_prefix (basename, "gimpswap.") ||
              strcmp (basename, "pluginrc") == 0       ||
              strcmp (basename, "themerc") == 0        ||
              strcmp (basename, "gtkrc") == 0)
            {
              goto next_file;
            }
          else if (strcmp (basename, "toolrc") == 0)
            {
              if (install->old_major < 2 ||
                  (install->old_major == 2 && install->old_minor < 10))
                {
                  /* No new tool since GIMP 2.10. */
                  goto next_file;
                }
              else
                {
                  update_pattern  = TOOLRC_UPDATE_PATTERN;
                  update_callback = user_update_toolrc;
                }
            }
          else if (install->old_major < 3 &&
                   strcmp (basename, "sessionrc") == 0)
            {
              /* We need to update size and positions because of scale
               * factor support.
               */
              update_pattern  = SESSIONRC_UPDATE_PATTERN_2TO3;
              update_callback = user_update_sessionrc_2to3;
            }
          else if (strcmp (basename, "sessionrc") == 0)
            {
              /* sessionrc updates since 3.0. */
              update_pattern  = SESSIONRC_UPDATE_PATTERN;
              update_callback = user_update_sessionrc;
            }
          else if (strcmp (basename, "menurc") == 0)
            {
              if (install->old_major < 2 ||
                  (install->old_major == 2 && install->old_minor == 0))
                {
                  /*  skip menurc for gimp 2.0 as the format has changed  */
                  goto next_file;
                }
              else
                {
                  update_pattern        = MENURC_OVER20_UPDATE_PATTERN;
                  update_callback       = user_update_menurc_over20;
                  post_process_callback = user_update_post_process_menurc_over20;
                  /* menurc becomes shortcutsrc in 3.0. */
                  new_dest              = "shortcutsrc";
                  break;
                }
            }
          else if (strcmp (basename, "shortcutsrc") == 0)
            {
              update_pattern  = SHORTCUTSRC_UPDATE_PATTERN;
              update_callback = user_update_shortcutsrc;
            }
          else if (strcmp (basename, "templaterc") == 0)
            {
              update_pattern  = TEMPLATERC_UPDATE_PATTERN;
              update_callback = user_update_templaterc;
            }
          else if (strcmp (basename, "controllerrc") == 0)
            {
              update_pattern  = CONTROLLERRC_UPDATE_PATTERN;
              update_callback = user_update_controllerrc;
            }
          else if (strcmp (basename, "gimprc") == 0)
            {
              update_pattern  = GIMPRC_UPDATE_PATTERN;
              update_callback = user_update_gimprc;
            }
          else if (strcmp (basename, "contextrc") == 0      ||
                   strcmp (basename, "devicerc") == 0)
            {
              update_pattern  = CONTEXTRC_UPDATE_PATTERN;
              update_callback = user_update_contextrc_over20;
            }

          g_snprintf (dest, sizeof (dest), "%s%c%s",
                      gimp_directory (), G_DIR_SEPARATOR,
                      new_dest ? new_dest : basename);

          user_install_file_copy (install, source, dest,
                                  update_pattern, update_callback,
                                  post_process_callback);
        }
      else if (g_file_test (source, G_FILE_TEST_IS_DIR))
        {
          const gchar        *update_pattern = NULL;
          GRegexEvalCallback  update_callback = NULL;

          /*  skip these directories for all old versions  */
          if (strcmp (basename, "tmp") == 0          ||
              strcmp (basename, "tool-options") == 0 ||
              strcmp (basename, "themes") == 0)
            {
              goto next_file;
            }
          else if (install->old_major < 3 &&
                   (strcmp (basename, "plug-ins") == 0 ||
                    strcmp (basename, "scripts") == 0))
            {
              /* Major API update. */
              goto next_file;
            }

          if (strcmp (basename, "gimpressionist") == 0)
            {
              update_pattern  = GIMPRESSIONIST_UPDATE_PATTERN;
              update_callback = user_update_gimpressionist;
            }
          else if (strcmp (basename, "tool-presets") == 0)
            {
              update_pattern  = TOOL_PRESETS_UPDATE_PATTERN;
              update_callback = user_update_tool_presets;
            }
          user_install_dir_copy (install, 0, source, gimp_directory (),
                                 update_pattern, update_callback, NULL);
        }

    next_file:
      g_free (source);
    }

  /*  create the tmp directory that was explicitly not copied  */

  g_snprintf (dest, sizeof (dest), "%s%c%s",
              gimp_directory (), G_DIR_SEPARATOR, "tmp");

  user_install_mkdir (install, dest);
  g_dir_close (dir);

  gimp_templates_migrate (install->old_dir);

  gimprc = gimp_rc_new (install->gimp, NULL, NULL, FALSE);
  gimp_rc_migrate (gimprc);
  gimp_rc_save (gimprc);
  g_object_unref (gimprc);

  return TRUE;
}
