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

#include "schemaentry.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QSharedData>

using namespace UserFeedback::Analyzer;

namespace UserFeedback {
namespace Analyzer {

class SchemaEntryData : public QSharedData
{
public:

    QString name;
    int internalId = -1;
    SchemaEntry::Type type = SchemaEntry::StringType;
};

}
}

SchemaEntry::SchemaEntry() : d(new SchemaEntryData) {}
SchemaEntry::SchemaEntry(const SchemaEntry&) = default;
SchemaEntry::~SchemaEntry() = default;
SchemaEntry& SchemaEntry::operator=(const SchemaEntry&) = default;

int SchemaEntry::internalId() const
{
    return d->internalId;
}

void SchemaEntry::setInternalId(int internalId)
{
    d->internalId = internalId;
}

QString SchemaEntry::name() const
{
    return d->name;
}

void SchemaEntry::setName(const QString& name)
{
    d->name = name;
}

SchemaEntry::Type SchemaEntry::type() const
{
    return d->type;
}

void SchemaEntry::setType(SchemaEntry::Type type)
{
    d->type = type;
}

QString SchemaEntry::displayString(SchemaEntry::Type type)
{
    switch (type) {
        case InvalidType: return QObject::tr("Invalid");
        case IntegerType: return QObject::tr("Integer");
        case StringType: return QObject::tr("String");
        case StringListType: return QObject::tr("String List");
        case RatioSetType: return QObject::tr("Ratio Set");
    }

    Q_UNREACHABLE();
}

QJsonObject SchemaEntry::toJsonObject() const
{
    QJsonObject obj;
    if (d->internalId >= 0)
        obj.insert(QStringLiteral("id"), d->internalId);
    obj.insert(QStringLiteral("name"), d->name);

    QString t;
    switch (d->type) {
        case InvalidType: break;
        case StringType: t = QStringLiteral("string"); break;
        case IntegerType: t = QStringLiteral("int"); break;
        case StringListType: t = QStringLiteral("string_list"); break;
        case RatioSetType: t = QStringLiteral("ratio_set"); break;
    }
    obj.insert(QStringLiteral("type"), t);
    return obj;
}

QVector<SchemaEntry> SchemaEntry::fromJson(const QJsonArray &array)
{
    QVector<SchemaEntry> res;
    res.reserve(array.size());

    foreach (const auto &v, array) {
        const auto obj = v.toObject();
        SchemaEntry entry;
        entry.setName(obj.value(QStringLiteral("name")).toString());
        const auto t = obj.value(QStringLiteral("type")).toString();
        if (t == QStringLiteral("string"))
            entry.setType(StringType);
        else if (t == QStringLiteral("int"))
            entry.setType(IntegerType);
        else if (t == QStringLiteral("string_list"))
            entry.setType(StringListType);
        else if (t == QStringLiteral("ratio_set"))
            entry.setType(RatioSetType);
        res.push_back(entry);
    }

    return res;
}
