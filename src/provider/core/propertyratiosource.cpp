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

#include "propertyratiosource.h"
#include "abstractdatasource_p.h"
#include "logging_p.h"

#include <QDebug>
#include <QHash>
#include <QMap>
#include <QMetaProperty>
#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QTime>

using namespace UserFeedback;

namespace UserFeedback {
class PropertyRatioSourcePrivate : public AbstractDataSourcePrivate
{
public:
    ~PropertyRatioSourcePrivate();

    void propertyChanged();
    QString valueToString(const QVariant &value) const;

    QString description;
    QObject *obj = nullptr; // TODO make this a QPointer?
    QObject *signalMonitor = nullptr;
    QMetaProperty property;
    QString previousValue;
    QTime lastChangeTime;
    QHash<QString, int> ratioSet;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QMap<QVariant, QString> valueMap;
#else
    struct ValueMapEntry { QVariant v; QString n; };
    QVector<ValueMapEntry> valueMap;
#endif
};

// inefficient workaround for not being able to connect QMetaMethod to a function directly
class SignalMonitor : public QObject
{
    Q_OBJECT
public:
    explicit SignalMonitor(PropertyRatioSourcePrivate *r) : m_receiver(r) {}
public slots:
    void propertyChanged()
    {
        m_receiver->propertyChanged();
    }
private:
    PropertyRatioSourcePrivate *m_receiver;
};

}

PropertyRatioSourcePrivate::~PropertyRatioSourcePrivate()
{
    delete signalMonitor;
}

void PropertyRatioSourcePrivate::propertyChanged()
{
    if (!previousValue.isEmpty() && lastChangeTime.elapsed() > 1000) {
        ratioSet[previousValue] += lastChangeTime.elapsed() / 1000;
    }

    lastChangeTime.start();
    previousValue = valueToString(property.read(obj));
}

QString PropertyRatioSourcePrivate::valueToString(const QVariant &value) const
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    const auto it = valueMap.constFind(value);
    if (it != valueMap.constEnd() && it.key() == value) {
        return it.value();
    }
#else
    foreach (const auto &e, valueMap) {
        if (e.v == value)
            return e.n;
    }
#endif
    return value.toString();
}

PropertyRatioSource::PropertyRatioSource(QObject *obj, const char *propertyName, const QString &sampleName) :
    AbstractDataSource(sampleName, new PropertyRatioSourcePrivate)
{
    Q_D(PropertyRatioSource);

    d->obj = obj;
    Q_ASSERT(obj);

    auto idx = obj->metaObject()->indexOfProperty(propertyName);
    Q_ASSERT(idx >= 0);
    if (idx < 0) {
        qCWarning(Log) << "Property" << propertyName << "not found in" << obj << "!";
        return;
    }

    d->property = obj->metaObject()->property(idx);
    if (!d->property.hasNotifySignal()) {
        qCWarning(Log) << "Property" << propertyName << "has no notification signal!";
        return;
    }

    d->signalMonitor = new SignalMonitor(d);
    idx = d->signalMonitor->metaObject()->indexOfMethod("propertyChanged()");
    Q_ASSERT(idx >= 0);
    const auto propertyChangedMethod = d->signalMonitor->metaObject()->method(idx);
    QObject::connect(obj, d->property.notifySignal(), d->signalMonitor, propertyChangedMethod);

    d->lastChangeTime.start();
    propertyChangedMethod.invoke(d->signalMonitor, Qt::QueuedConnection);
}

void PropertyRatioSource::addValueMapping(const QVariant &value, const QString &str)
{
    Q_D(PropertyRatioSource);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    d->valueMap.insert(value, str);
#else
    PropertyRatioSourcePrivate::ValueMapEntry e;
    e.v = value;
    e.n = str;
    d->valueMap.push_back(e);
#endif
}

QString PropertyRatioSource::description() const
{
    Q_D(const PropertyRatioSource);
    return d->description;
}

void PropertyRatioSource::setDescription(const QString& desc)
{
    Q_D(PropertyRatioSource);
    d->description = desc;
}

QVariant PropertyRatioSource::data()
{
    Q_D(PropertyRatioSource);
    d->propertyChanged();

    QVariantMap m;
    int total = 0;
    for (auto it = d->ratioSet.constBegin(); it != d->ratioSet.constEnd(); ++it)
        total += it.value();

    for (auto it = d->ratioSet.constBegin(); it != d->ratioSet.constEnd(); ++it) {
        QVariantMap v;
        v.insert(QStringLiteral("property"), (double)it.value() / (double)(total));
        m.insert(it.key(), v);
    }

    return m;
}

void PropertyRatioSource::load(QSettings *settings)
{
    Q_D(PropertyRatioSource);
    foreach (const auto &value, settings->childKeys()) {
        const auto amount = settings->value(value, 0).toInt();
        d->ratioSet.insert(value, amount);
    }
}


void PropertyRatioSource::store(QSettings *settings)
{
    Q_D(PropertyRatioSource);
    d->propertyChanged();

    settings->remove(QString());
    for (auto it = d->ratioSet.constBegin(); it != d->ratioSet.constEnd(); ++it) {
        settings->setValue(it.key(), it.value());
    }
}

#include "propertyratiosource.moc"
