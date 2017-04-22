/*
    Copyright (C) 2016 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "serverinfo.h"

#include <QSharedData>
#include <QSettings>
#include <QUrl>

using namespace UserFeedback::Console;

namespace UserFeedback {
namespace Console {
class ServerInfoData : public QSharedData
{
public:
    static QString groupName(const QString &name);

    QString name;
    QUrl url;
    QString userName;
    QString password;
};

}
}

QString ServerInfoData::groupName(const QString &name)
{
    return QString::fromLatin1(QUrl::toPercentEncoding(name));
}

ServerInfo::ServerInfo() : d(new ServerInfoData) {}
ServerInfo::ServerInfo(const ServerInfo&) = default;
ServerInfo::~ServerInfo() = default;
ServerInfo& ServerInfo::operator=(const ServerInfo&) = default;

bool ServerInfo::isValid() const
{
    return d->url.isValid();
}

QString ServerInfo::name() const
{
    // ### temporary
    if (d->name.isEmpty())
        return d->url.toString();
    return d->name;
}

void ServerInfo::setName(const QString& name)
{
    d->name = name;
}

QUrl ServerInfo::url() const
{
    return d->url;
}

void ServerInfo::setUrl(const QUrl& url)
{
    d->url = url;
}

QString ServerInfo::userName() const
{
    return d->userName;
}

void ServerInfo::setUserName(const QString& userName)
{
    d->userName = userName;
}

QString ServerInfo::password() const
{
    return d->password;
}

void ServerInfo::setPassword(const QString& password)
{
    d->password = password;
}

void ServerInfo::save() const
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("ServerInfo"));
    settings.beginGroup(ServerInfoData::groupName(name()));
    settings.setValue(QStringLiteral("name"), name());
    settings.setValue(QStringLiteral("url"), url().toString());
    settings.setValue(QStringLiteral("userName"), userName());
    settings.setValue(QStringLiteral("password"), password()); // TODO
    settings.endGroup();

    auto allServers = settings.value(QStringLiteral("ServerNames")).toStringList();
    if (!allServers.contains(name()))
        allServers.push_back(name());
    settings.setValue(QStringLiteral("ServerNames"), allServers);
}

ServerInfo ServerInfo::load(const QString &name)
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("ServerInfo"));
    settings.beginGroup(ServerInfoData::groupName(name));

    ServerInfo info;
    info.setName(settings.value(QStringLiteral("name")).toString());
    info.setUrl(QUrl(settings.value(QStringLiteral("url")).toString()));
    info.setUserName(settings.value(QStringLiteral("userName")).toString());
    info.setPassword(settings.value(QStringLiteral("password")).toString());
    return info;
}

void ServerInfo::remove(const QString& name)
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("ServerInfo"));
    settings.remove(ServerInfoData::groupName(name));

    auto allServers = settings.value(QStringLiteral("ServerNames")).toStringList();
    allServers.removeAll(name);
    settings.setValue(QStringLiteral("ServerNames"), allServers);
}

QStringList ServerInfo::allServerInfoNames()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("ServerInfo"));
    return settings.value(QStringLiteral("ServerNames")).toStringList();
}
