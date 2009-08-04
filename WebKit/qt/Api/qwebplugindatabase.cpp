/*
    Copyright (C) 2009 Jakub Wieczorek <faw217@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"
#include "qwebplugindatabase.h"
#include "qwebplugindatabase_p.h"

#include "PluginDatabase.h"
#include "PluginPackage.h"

using namespace WebCore;

/*!
    \class QWebPlugin::MimeType
    \since 4.6
    \brief Represents a single MIME type supported by a plugin.
*/

/*!
    \variable QWebPlugin::MimeType::name
    \brief the name of the MIME type e.g. text/plain
*/

/*!
    \variable QWebPlugin::MimeType::description
    \brief the description of the MIME type
*/

/*!
    \variable QWebPlugin::MimeType::fileExtensions
    \brief a list of file extensions that are associated with the mime type
*/

QWebPluginPrivate::QWebPluginPrivate(RefPtr<PluginPackage> pluginPackage)
    : plugin(pluginPackage)
{
}

QWebPluginDatabasePrivate::QWebPluginDatabasePrivate(PluginDatabase* pluginDatabase)
    : database(pluginDatabase)
{
}

/*!
    \class QWebPlugin
    \since 4.6
    \brief The QWebPlugin class represents a single Netscape plugin.

    The QWebPlugin represents a Netscape plugin, that is picked up by WebKit
    and included in the plugin database. The class contains information about
    the plugin: its name(), description(), a list of MIME types that it supports:
    mimeTypes() and the path of the plugin file.

    Particular plugins can be disabled, this way they won't be used by WebKit
    for handling supported MIME types: setEnabled(), enabled().

    \sa QWebPluginDatabase
*/

/*!
    Constructs a null QWebPlugin.
*/
QWebPlugin::QWebPlugin()
    : d(new QWebPluginPrivate(0))
{
}

QWebPlugin::QWebPlugin(PluginPackage* plugin)
    : d(new QWebPluginPrivate(plugin))
{
}

/*!
    Contructs a copy of \a other.
*/
QWebPlugin::QWebPlugin(const QWebPlugin& other)
    : d(new QWebPluginPrivate(other.d->plugin))
{
}

QWebPlugin::~QWebPlugin()
{
    delete d;
}

/*!
    Returns the name of the plugin.

    \sa description()
*/
QString QWebPlugin::name() const
{
    if (!d->plugin)
        return QString();
    return d->plugin->name();
}

/*!
    Returns the description of the plugin.

    \sa name()
*/
QString QWebPlugin::description() const
{
    if (!d->plugin)
        return QString();
    return d->plugin->description();
}

/*!
    Returns a list of MIME types that are supported by the plugin.

    \sa supportsMimeType()
*/
QList<QWebPlugin::MimeType> QWebPlugin::mimeTypes() const
{
    if (!d->plugin)
        return QList<MimeType>();

    QList<MimeType> mimeTypes;
    const MIMEToDescriptionsMap& mimeToDescriptions = d->plugin->mimeToDescriptions();
    MIMEToDescriptionsMap::const_iterator end = mimeToDescriptions.end();
    for (MIMEToDescriptionsMap::const_iterator it = mimeToDescriptions.begin(); it != end; ++it) {
        MimeType mimeType;
        mimeType.name = it->first;
        mimeType.description = it->second;

        QStringList fileExtensions;
        Vector<String> extensions = d->plugin->mimeToExtensions().get(mimeType.name);

        for (unsigned i = 0; i < extensions.size(); ++i)
            fileExtensions.append(extensions[i]);

        mimeType.fileExtensions = fileExtensions;
        mimeTypes.append(mimeType);
    }

    return mimeTypes;
}

/*!
    Returns true if the plugin supports a specific \a mimeType, false otherwise.

    \sa mimeTypes()
*/
bool QWebPlugin::supportsMimeType(const QString& mimeType) const
{
    QList<MimeType> types = mimeTypes();
    foreach (const MimeType& type, types) {
        if (type.name == mimeType)
            return true;
    }

    return false;
}

/*!
    Returns an absolute path to the plugin file.
*/
QString QWebPlugin::path() const
{
    if (!d->plugin)
        return QString();
    return d->plugin->path();
}

/*!
    Returns true if the plugin is a null plugin, false otherwise.
*/
bool QWebPlugin::isNull() const
{
    return !d->plugin;
}

/*!
    Enables or disables the plugin.

    Disabled plugins will not be picked up by WebKit when looking for a plugin supporting
    a particular MIME type.

    \sa isEnabled()
*/
void QWebPlugin::setEnabled(bool enabled)
{
    if (!d->plugin)
        return;
    d->plugin->setEnabled(enabled);
}

/*!
    Returns true if the plugin is enabled, false otherwise.

    \sa setEnabled()
*/
bool QWebPlugin::isEnabled() const
{
    if (!d->plugin)
        return false;
    return d->plugin->isEnabled();
}

bool QWebPlugin::operator==(const QWebPlugin& other) const
{
    return d->plugin == other.d->plugin;
}

bool QWebPlugin::operator!=(const QWebPlugin& other) const
{
    return d->plugin != other.d->plugin;
}

QWebPlugin QWebPlugin::operator=(const QWebPlugin& other)
{
    if (this == &other)
        return *this;

    d->plugin = other.d->plugin;
    return *this;
}

/*!
    \class QWebPluginDatabase
    \since 4.6
    \brief The QWebPluginDatabase class provides an interface for managing
    Netscape plugins used by WebKit in QWebPages.

    The QWebPluginDatabase class is a database of Netscape plugins that are used
    by WebKit. The plugins are picked up by WebKit by looking up a set of search paths.
    The default set can be accessed using defaultSearchPaths(). The search paths
    can be changed, see searchPaths() and setSearchPaths(). Additional search paths
    can also be added using addSearchPath().

    The plugins that have been detected are exposed by the plugins() method.
    The list contains QWebPlugin objects that hold both the metadata and the MIME
    types that are supported by particular plugins.

    WebKit specifies a plugin for a MIME type by looking for the first plugin that
    supports the specific MIME type. To get a plugin, that is used by WebKit to
    handle a specific MIME type, you can use the pluginForMimeType() function.

    To change the way of resolving MIME types ambiguity, you can explicitly set
    a preferred plugin for a specific MIME type, using setPreferredPluginForMimeType().

    \sa QWebPlugin, QWebSettings::pluginDatabase()
*/

QWebPluginDatabase::QWebPluginDatabase(QObject* parent)
    : QObject(parent)
    , d(new QWebPluginDatabasePrivate(PluginDatabase::installedPlugins()))
{
}

QWebPluginDatabase::~QWebPluginDatabase()
{
    delete d;
}

/*!
    Returns a list of plugins installed in the search paths.
    The list includes disabled plugins, even though they are not used.

    \sa pluginForMimeType()
*/
QList<QWebPlugin> QWebPluginDatabase::plugins() const
{
    QList<QWebPlugin> qwebplugins;
    const Vector<PluginPackage*>& plugins = d->database->plugins();

    for (unsigned int i = 0; i < plugins.size(); ++i) {
        PluginPackage* plugin = plugins[i];
        qwebplugins.append(QWebPlugin(plugin));
    }

    return qwebplugins;
}

/*!
    Returns a default set of search paths.

    \sa searchPaths(), setSearchPaths()
*/
QStringList QWebPluginDatabase::defaultSearchPaths()
{
    QStringList paths;

    const Vector<String>& directories = PluginDatabase::defaultPluginDirectories();
    for (unsigned int i = 0; i < directories.size(); ++i)
        paths.append(directories[i]);

    return paths;
}

/*!
    Returns a list of search paths that are used by WebKit to look for plugins.

    \sa defaultSearchPaths(), setSearchPaths()
*/
QStringList QWebPluginDatabase::searchPaths() const
{
    QStringList paths;

    const Vector<String>& directories = d->database->pluginDirectories();
    for (unsigned int i = 0; i < directories.size(); ++i)
        paths.append(directories[i]);

    return paths;
}

/*!
    Changes the search paths to \a paths.
    The database is automatically refreshed.

    \sa searchPaths(), defaultSearchPaths()
*/
void QWebPluginDatabase::setSearchPaths(const QStringList& paths)
{
    Vector<String> directories;

    for (unsigned int i = 0; i < paths.count(); ++i)
        directories.append(paths.at(i));

    d->database->setPluginDirectories(directories);
    // PluginDatabase::setPluginDirectories() does not refresh the database.
    d->database->refresh();
}

/*!
    Adds an additional \a path to the current set.
    The database is automatically refreshed.

    \sa searchPaths(), setSearchPaths()
*/
void QWebPluginDatabase::addSearchPath(const QString& path)
{
    d->database->addExtraPluginDirectory(path);
    // PluginDatabase::addExtraPluginDirectory() does refresh the database.
}

/*!
    Refreshes the plugin database, adds new plugins that have been found and removes
    the ones that are no longer available in the search paths.

    You can call this function when the set of plugins installed in the search paths
    changes. You do not need to call this function when changing search paths,
    in that case WebKit automatically refreshes the database.
*/
void QWebPluginDatabase::refresh()
{
    d->database->refresh();
}

/*!
    Returns the plugin that is currently used by WebKit for a given \a mimeType.

    \sa setPreferredPluginForMimeType()
*/
QWebPlugin QWebPluginDatabase::pluginForMimeType(const QString& mimeType)
{
    return QWebPlugin(d->database->pluginForMIMEType(mimeType));
}

/*!
    Changes the preferred plugin for a given \a mimeType to \a plugin. The \a plugin
    has to support the given \a mimeType, otherwise the setting will have no effect.

    Calling the function with a null \a plugin resets the setting.

    \sa pluginForMimeType()
*/
void QWebPluginDatabase::setPreferredPluginForMimeType(const QString& mimeType, const QWebPlugin& plugin)
{
    d->database->setPreferredPluginForMIMEType(mimeType, plugin.d->plugin.get());
}
