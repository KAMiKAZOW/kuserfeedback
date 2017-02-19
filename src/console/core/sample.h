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

#ifndef USERFEEDBACK_CONSOLE_SAMPLE_H
#define USERFEEDBACK_CONSOLE_SAMPLE_H

#include <QMetaType>
#include <QSharedDataPointer>
#include <QVector>

class QDateTime;
class QString;

namespace UserFeedback {
namespace Console {

class Product;
class SampleData;

/** One data sample reported by a client. */
class Sample
{
public:
    Sample();
    Sample(const Sample&);
    ~Sample();
    Sample& operator=(const Sample&);

    QDateTime timestamp() const;
    QVariant value(const QString &name) const;

    static QVector<Sample> fromJson(const QByteArray &json, const Product &product);
    static QByteArray toJson(const QVector<Sample> &samples, const Product &product);

private:
    QSharedDataPointer<SampleData> d;
};

}
}

Q_DECLARE_TYPEINFO(UserFeedback::Console::Sample, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(UserFeedback::Console::Sample)

#endif // USERFEEDBACK_CONSOLE_SAMPLE_H
