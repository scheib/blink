/*
 * Copyright (C) 2008 Christian Dywan <christian@imendio.com>
 * Copyright (C) 2008 Nuanti Ltd.
 * Copyright (C) 2008 Collabora Ltd.
 * Copyright (C) 2008 Holger Hans Peter Freyther
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
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"

#include "webkitwebsettings.h"
#include "webkitprivate.h"

#include <glib/gi18n-lib.h>
#include "FileSystem.h"
#include "PluginDatabase.h"

/**
 * SECTION:webkitwebsettings
 * @short_description: Control the behaviour of a #WebKitWebView
 *
 * #WebKitWebSettings can be applied to a #WebKitWebView to control
 * the to be used text encoding, color, font sizes, printing mode,
 * script support, loading of images and various other things.
 *
 * <informalexample><programlisting>
 * /<!-- -->* Create a new websettings and disable java script *<!-- -->/
 * WebKitWebSettings *settings = webkit_web_settings_new ();
 * g_object_set (G_OBJECT(settings), "enable-scripts", FALSE, NULL);
 *
 * /<!-- -->* Apply the result *<!-- -->/
 * webkit_web_view_set_settings (WEBKIT_WEB_VIEW(my_webview), settings);
 * </programlisting></informalexample>
 */

using namespace WebCore;

extern "C" {

G_DEFINE_TYPE(WebKitWebSettings, webkit_web_settings, G_TYPE_OBJECT)

struct _WebKitWebSettingsPrivate {
    gchar* default_encoding;
    gchar* cursive_font_family;
    gchar* default_font_family;
    gchar* fantasy_font_family;
    gchar* monospace_font_family;
    gchar* sans_serif_font_family;
    gchar* serif_font_family;
    guint default_font_size;
    guint default_monospace_font_size;
    guint minimum_font_size;
    guint minimum_logical_font_size;
    gboolean enforce_96_dpi;
    gboolean auto_load_images;
    gboolean auto_shrink_images;
    gboolean print_backgrounds;
    gboolean enable_scripts;
    gboolean enable_plugins;
    gboolean resizable_text_areas;
    gchar* user_stylesheet_uri;
    gfloat zoom_step;
    gboolean enable_developer_extras;
    gboolean enable_private_browsing;
};

#define WEBKIT_WEB_SETTINGS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), WEBKIT_TYPE_WEB_SETTINGS, WebKitWebSettingsPrivate))

enum {
    PROP_0,

    PROP_DEFAULT_ENCODING,
    PROP_CURSIVE_FONT_FAMILY,
    PROP_DEFAULT_FONT_FAMILY,
    PROP_FANTASY_FONT_FAMILY,
    PROP_MONOSPACE_FONT_FAMILY,
    PROP_SANS_SERIF_FONT_FAMILY,
    PROP_SERIF_FONT_FAMILY,
    PROP_DEFAULT_FONT_SIZE,
    PROP_DEFAULT_MONOSPACE_FONT_SIZE,
    PROP_MINIMUM_FONT_SIZE,
    PROP_MINIMUM_LOGICAL_FONT_SIZE,
    PROP_ENFORCE_96_DPI,
    PROP_AUTO_LOAD_IMAGES,
    PROP_AUTO_SHRINK_IMAGES,
    PROP_PRINT_BACKGROUNDS,
    PROP_ENABLE_SCRIPTS,
    PROP_ENABLE_PLUGINS,
    PROP_RESIZABLE_TEXT_AREAS,
    PROP_USER_STYLESHEET_URI,
    PROP_ZOOM_STEP,
    PROP_ENABLE_DEVELOPER_EXTRAS,
    PROP_ENABLE_PRIVATE_BROWSING
};

static void webkit_web_settings_finalize(GObject* object);

static void webkit_web_settings_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec);

static void webkit_web_settings_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec);

static void webkit_web_settings_class_init(WebKitWebSettingsClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->finalize = webkit_web_settings_finalize;
    gobject_class->set_property = webkit_web_settings_set_property;
    gobject_class->get_property = webkit_web_settings_get_property;

    GParamFlags flags = (GParamFlags)(WEBKIT_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    g_object_class_install_property(gobject_class,
                                    PROP_DEFAULT_ENCODING,
                                    g_param_spec_string(
                                    "default-encoding",
                                    _("Default Encoding"),
                                    _("The default encoding used to display text."),
                                    "iso-8859-1",
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_CURSIVE_FONT_FAMILY,
                                    g_param_spec_string(
                                    "cursive-font-family",
                                    _("Cursive Font Family"),
                                    _("The default Cursive font family used to display text."),
                                    "serif",
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_DEFAULT_FONT_FAMILY,
                                    g_param_spec_string(
                                    "default-font-family",
                                    _("Default Font Family"),
                                    _("The default font family used to display text."),
                                    "sans-serif",
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_FANTASY_FONT_FAMILY,
                                    g_param_spec_string(
                                    "fantasy-font-family",
                                    _("Fantasy Font Family"),
                                    _("The default Fantasy font family used to display text."),
                                    "serif",
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_MONOSPACE_FONT_FAMILY,
                                    g_param_spec_string(
                                    "monospace-font-family",
                                    _("Monospace Font Family"),
                                    _("The default font family used to display monospace text."),
                                    "monospace",
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_SANS_SERIF_FONT_FAMILY,
                                    g_param_spec_string(
                                    "sans-serif-font-family",
                                    _("Sans Serif Font Family"),
                                    _("The default Sans Serif font family used to display text."),
                                    "sans-serif",
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_SERIF_FONT_FAMILY,
                                    g_param_spec_string(
                                    "serif-font-family",
                                    _("Serif Font Family"),
                                    _("The default Serif font family used to display text."),
                                    "serif",
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_DEFAULT_FONT_SIZE,
                                    g_param_spec_int(
                                    "default-font-size",
                                    _("Default Font Size"),
                                    _("The default font size used to display text."),
                                    5, G_MAXINT, 12,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_DEFAULT_MONOSPACE_FONT_SIZE,
                                    g_param_spec_int(
                                    "default-monospace-font-size",
                                    _("Default Monospace Font Size"),
                                    _("The default font size used to display monospace text."),
                                    5, G_MAXINT, 10,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_MINIMUM_FONT_SIZE,
                                    g_param_spec_int(
                                    "minimum-font-size",
                                    _("Minimum Font Size"),
                                    _("The minimum font size used to display text."),
                                    1, G_MAXINT, 5,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_MINIMUM_LOGICAL_FONT_SIZE,
                                    g_param_spec_int(
                                    "minimum-logical-font-size",
                                    _("Minimum Logical Font Size"),
                                    _("The minimum logical font size used to display text."),
                                    1, G_MAXINT, 5,
                                    flags));

    /**
    * WebKitWebSettings:enforce-96-dpi:
    *
    * Enforce a resolution of 96 DPI. This is meant for compatibility
    * with web pages which cope badly with different screen resolutions
    * and for automated testing.
    * Web browsers and applications that typically display arbitrary
    * content from the web should provide a preference for this.
    *
    * Since: 1.0.3
    */
    g_object_class_install_property(gobject_class,
                                    PROP_ENFORCE_96_DPI,
                                    g_param_spec_boolean(
                                    "enforce-96-dpi",
                                    _("Enforce 96 DPI"),
                                    _("Enforce a resolution of 96 DPI"),
                                    FALSE,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_AUTO_LOAD_IMAGES,
                                    g_param_spec_boolean(
                                    "auto-load-images",
                                    _("Auto Load Images"),
                                    _("Load images automatically."),
                                    TRUE,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_AUTO_SHRINK_IMAGES,
                                    g_param_spec_boolean(
                                    "auto-shrink-images",
                                    _("Auto Shrink Images"),
                                    _("Automatically shrink standalone images to fit."),
                                    TRUE,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_PRINT_BACKGROUNDS,
                                    g_param_spec_boolean(
                                    "print-backgrounds",
                                    _("Print Backgrounds"),
                                    _("Whether background images should be printed."),
                                    TRUE,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_ENABLE_SCRIPTS,
                                    g_param_spec_boolean(
                                    "enable-scripts",
                                    _("Enable Scripts"),
                                    _("Enable embedded scripting languages."),
                                    TRUE,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_ENABLE_PLUGINS,
                                    g_param_spec_boolean(
                                    "enable-plugins",
                                    _("Enable Plugins"),
                                    _("Enable embedded plugin objects."),
                                    TRUE,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_RESIZABLE_TEXT_AREAS,
                                    g_param_spec_boolean(
                                    "resizable-text-areas",
                                    _("Resizable Text Areas"),
                                    _("Whether text areas are resizable."),
                                    TRUE,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_USER_STYLESHEET_URI,
                                    g_param_spec_string("user-stylesheet-uri",
                                    _("User Stylesheet URI"),
                                    _("The URI of a stylesheet that is applied to every page."),
                                    0,
                                    flags));

    /**
    * WebKitWebSettings:zoom-step:
    *
    * The value by which the zoom level is changed when zooming in or out.
    *
    * Since: 1.0.1
    */
    g_object_class_install_property(gobject_class,
                                    PROP_ZOOM_STEP,
                                    g_param_spec_float(
                                    "zoom-step",
                                    _("Zoom Stepping Value"),
                                    _("The value by which the zoom level is changed when zooming in or out."),
                                    0.0f, G_MAXFLOAT, 0.1f,
                                    flags));

    /**
    * WebKitWebSettings:enable-developer-extras:
    *
    * Whether developer extensions should be enabled. This enables,
    * for now, the Web Inspector, which can be controlled using the
    * #WebKitWebInspector instance held by the #WebKitWebView this
    * setting is enabled for.
    *
    * Since: 1.0.3
    */
    g_object_class_install_property(gobject_class,
                                    PROP_ENABLE_DEVELOPER_EXTRAS,
                                    g_param_spec_boolean(
                                    "enable-developer-extras",
                                    _("Enable Developer Extras"),
                                    _("Enables special extensions that help developers"),
                                    FALSE,
                                    flags));

    /**
    * WebKitWebSettings:enable-private-browsing:
    *
    * Whether to enable private browsing mode. Private browsing mode prevents
    * WebKit from updating the global history and storing any session
    * information e.g., on-disk cache, as well as suppressing any messages
    * from being printed into the (javascript) console.
    *
    * This is currently experimental for WebKitGtk.
    *
    * Since 1.1.2
    */
    g_object_class_install_property(gobject_class,
                                    PROP_ENABLE_PRIVATE_BROWSING,
                                    g_param_spec_boolean(
                                    "enable-private-browsing",
                                    _("Enable Private Browsing"),
                                    _("Enables private browsing mode"),
                                    FALSE,
                                    flags));

    g_type_class_add_private(klass, sizeof(WebKitWebSettingsPrivate));
}

static void webkit_web_settings_init(WebKitWebSettings* web_settings)
{
    web_settings->priv = WEBKIT_WEB_SETTINGS_GET_PRIVATE(web_settings);
}

static void webkit_web_settings_finalize(GObject* object)
{
    WebKitWebSettings* web_settings = WEBKIT_WEB_SETTINGS(object);
    WebKitWebSettingsPrivate* priv = web_settings->priv;

    g_free(priv->default_encoding);
    g_free(priv->cursive_font_family);
    g_free(priv->default_font_family);
    g_free(priv->fantasy_font_family);
    g_free(priv->monospace_font_family);
    g_free(priv->sans_serif_font_family);
    g_free(priv->serif_font_family);
    g_free(priv->user_stylesheet_uri);

    G_OBJECT_CLASS(webkit_web_settings_parent_class)->finalize(object);
}

static void webkit_web_settings_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec)
{
    WebKitWebSettings* web_settings = WEBKIT_WEB_SETTINGS(object);
    WebKitWebSettingsPrivate* priv = web_settings->priv;

    switch(prop_id) {
    case PROP_DEFAULT_ENCODING:
        g_free(priv->default_encoding);
        priv->default_encoding = g_strdup(g_value_get_string(value));
        break;
    case PROP_CURSIVE_FONT_FAMILY:
        g_free(priv->cursive_font_family);
        priv->cursive_font_family = g_strdup(g_value_get_string(value));
        break;
    case PROP_DEFAULT_FONT_FAMILY:
        g_free(priv->default_font_family);
        priv->default_font_family = g_strdup(g_value_get_string(value));
        break;
    case PROP_FANTASY_FONT_FAMILY:
        g_free(priv->fantasy_font_family);
        priv->fantasy_font_family = g_strdup(g_value_get_string(value));
        break;
    case PROP_MONOSPACE_FONT_FAMILY:
        g_free(priv->monospace_font_family);
        priv->monospace_font_family = g_strdup(g_value_get_string(value));
        break;
    case PROP_SANS_SERIF_FONT_FAMILY:
        g_free(priv->sans_serif_font_family);
        priv->sans_serif_font_family = g_strdup(g_value_get_string(value));
        break;
    case PROP_SERIF_FONT_FAMILY:
        g_free(priv->serif_font_family);
        priv->serif_font_family = g_strdup(g_value_get_string(value));
        break;
    case PROP_DEFAULT_FONT_SIZE:
        priv->default_font_size = g_value_get_int(value);
        break;
    case PROP_DEFAULT_MONOSPACE_FONT_SIZE:
        priv->default_monospace_font_size = g_value_get_int(value);
        break;
    case PROP_MINIMUM_FONT_SIZE:
        priv->minimum_font_size = g_value_get_int(value);
        break;
    case PROP_MINIMUM_LOGICAL_FONT_SIZE:
        priv->minimum_logical_font_size = g_value_get_int(value);
        break;
    case PROP_ENFORCE_96_DPI:
        priv->enforce_96_dpi = g_value_get_boolean(value);
        break;
    case PROP_AUTO_LOAD_IMAGES:
        priv->auto_load_images = g_value_get_boolean(value);
        break;
    case PROP_AUTO_SHRINK_IMAGES:
        priv->auto_shrink_images = g_value_get_boolean(value);
        break;
    case PROP_PRINT_BACKGROUNDS:
        priv->print_backgrounds = g_value_get_boolean(value);
        break;
    case PROP_ENABLE_SCRIPTS:
        priv->enable_scripts = g_value_get_boolean(value);
        break;
    case PROP_ENABLE_PLUGINS:
        priv->enable_plugins = g_value_get_boolean(value);
        break;
    case PROP_RESIZABLE_TEXT_AREAS:
        priv->resizable_text_areas = g_value_get_boolean(value);
        break;
    case PROP_USER_STYLESHEET_URI:
        g_free(priv->user_stylesheet_uri);
        priv->user_stylesheet_uri = g_strdup(g_value_get_string(value));
        break;
    case PROP_ZOOM_STEP:
        priv->zoom_step = g_value_get_float(value);
        break;
    case PROP_ENABLE_DEVELOPER_EXTRAS:
        priv->enable_developer_extras = g_value_get_boolean(value);
        break;
    case PROP_ENABLE_PRIVATE_BROWSING:
        priv->enable_private_browsing = g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void webkit_web_settings_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec)
{
    WebKitWebSettings* web_settings = WEBKIT_WEB_SETTINGS(object);
    WebKitWebSettingsPrivate* priv = web_settings->priv;

    switch (prop_id) {
    case PROP_DEFAULT_ENCODING:
        g_value_set_string(value, priv->default_encoding);
        break;
    case PROP_CURSIVE_FONT_FAMILY:
        g_value_set_string(value, priv->cursive_font_family);
        break;
    case PROP_DEFAULT_FONT_FAMILY:
        g_value_set_string(value, priv->default_font_family);
        break;
    case PROP_FANTASY_FONT_FAMILY:
        g_value_set_string(value, priv->fantasy_font_family);
        break;
    case PROP_MONOSPACE_FONT_FAMILY:
        g_value_set_string(value, priv->monospace_font_family);
        break;
    case PROP_SANS_SERIF_FONT_FAMILY:
        g_value_set_string(value, priv->sans_serif_font_family);
        break;
    case PROP_SERIF_FONT_FAMILY:
        g_value_set_string(value, priv->serif_font_family);
        break;
    case PROP_DEFAULT_FONT_SIZE:
        g_value_set_int(value, priv->default_font_size);
        break;
    case PROP_DEFAULT_MONOSPACE_FONT_SIZE:
        g_value_set_int(value, priv->default_monospace_font_size);
        break;
    case PROP_MINIMUM_FONT_SIZE:
        g_value_set_int(value, priv->minimum_font_size);
        break;
    case PROP_MINIMUM_LOGICAL_FONT_SIZE:
        g_value_set_int(value, priv->minimum_logical_font_size);
        break;
    case PROP_ENFORCE_96_DPI:
        g_value_set_boolean(value, priv->enforce_96_dpi);
        break;
    case PROP_AUTO_LOAD_IMAGES:
        g_value_set_boolean(value, priv->auto_load_images);
        break;
    case PROP_AUTO_SHRINK_IMAGES:
        g_value_set_boolean(value, priv->auto_shrink_images);
        break;
    case PROP_PRINT_BACKGROUNDS:
        g_value_set_boolean(value, priv->print_backgrounds);
        break;
    case PROP_ENABLE_SCRIPTS:
        g_value_set_boolean(value, priv->enable_scripts);
        break;
    case PROP_ENABLE_PLUGINS:
        g_value_set_boolean(value, priv->enable_plugins);
        break;
    case PROP_RESIZABLE_TEXT_AREAS:
        g_value_set_boolean(value, priv->resizable_text_areas);
        break;
    case PROP_USER_STYLESHEET_URI:
        g_value_set_string(value, priv->user_stylesheet_uri);
        break;
    case PROP_ZOOM_STEP:
        g_value_set_float(value, priv->zoom_step);
        break;
    case PROP_ENABLE_DEVELOPER_EXTRAS:
        g_value_set_boolean(value, priv->enable_developer_extras);
        break;
    case PROP_ENABLE_PRIVATE_BROWSING:
        g_value_set_boolean(value, priv->enable_private_browsing);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

/**
 * webkit_web_settings_new:
 *
 * Creates a new #WebKitWebSettings instance with default values. It must
 * be manually attached to a WebView.
 *
 * Returns: a new #WebKitWebSettings instance
 **/
WebKitWebSettings* webkit_web_settings_new()
{
    return WEBKIT_WEB_SETTINGS(g_object_new(WEBKIT_TYPE_WEB_SETTINGS, NULL));
}

/**
 * webkit_web_settings_copy:
 *
 * Copies an existing #WebKitWebSettings instance.
 *
 * Returns: a new #WebKitWebSettings instance
 **/
WebKitWebSettings* webkit_web_settings_copy(WebKitWebSettings* web_settings)
{
    WebKitWebSettingsPrivate* priv = web_settings->priv;

    WebKitWebSettings* copy = WEBKIT_WEB_SETTINGS(g_object_new(WEBKIT_TYPE_WEB_SETTINGS,
                 "default-encoding", priv->default_encoding,
                 "cursive-font-family", priv->cursive_font_family,
                 "default-font-family", priv->default_font_family,
                 "fantasy-font-family", priv->fantasy_font_family,
                 "monospace-font-family", priv->monospace_font_family,
                 "sans-serif-font-family", priv->sans_serif_font_family,
                 "serif-font-family", priv->serif_font_family,
                 "default-font-size", priv->default_font_size,
                 "default-monospace-font-size", priv->default_monospace_font_size,
                 "minimum-font-size", priv->minimum_font_size,
                 "minimum-logical-font-size", priv->minimum_logical_font_size,
                 "auto-load-images", priv->auto_load_images,
                 "auto-shrink-images", priv->auto_shrink_images,
                 "print-backgrounds", priv->print_backgrounds,
                 "enable-scripts", priv->enable_scripts,
                 "enable-plugins", priv->enable_plugins,
                 "resizable-text-areas", priv->resizable_text_areas,
                 "user-stylesheet-uri", priv->user_stylesheet_uri,
                 "zoom-step", priv->zoom_step,
                 "enable-developer-extras", priv->enable_developer_extras,
                 "enable-private-browsing", priv->enable_private_browsing,
                 NULL));

    return copy;
}

/**
 * webkit_web_settings_add_extra_plugin_directory:
 * @web_view: a #WebKitWebView
 * @directory: the directory to add
 *
 * Adds the @directory to paths where @web_view will search for plugins.
 *
 * Since: 1.0.3
 */
void webkit_web_settings_add_extra_plugin_directory(WebKitWebView* webView, const gchar* directory)
{
    g_return_if_fail(WEBKIT_IS_WEB_VIEW(webView));

    PluginDatabase::installedPlugins()->addExtraPluginDirectory(filenameToString(directory));
}

}
