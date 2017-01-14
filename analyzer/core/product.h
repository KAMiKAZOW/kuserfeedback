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

#ifndef USERFEEDBACK_ANALYZER_PRODUCT_H
#define USERFEEDBACK_ANALYZER_PRODUCT_H

#include "schemaentry.h"

#include <QMetaType>
#include <QSharedDataPointer>
#include <QVector>

class QString;

namespace UserFeedback {
namespace Analyzer {

class ProductData;

/** Product data. */
class Product
{
public:
    Product();
    Product(const Product&);
    ~Product();
    Product& operator=(const Product&);

    bool isValid() const;

    QString name() const;
    void setName(const QString &name);

    QVector<SchemaEntry> schema() const;
    void setSchema(const QVector<SchemaEntry>& schema);

    QByteArray toJson() const;
    static QVector<Product> fromJson(const QByteArray &data);

private:
    QSharedDataPointer<ProductData> d;

};

}
}

Q_DECLARE_TYPEINFO(UserFeedback::Analyzer::Product, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(UserFeedback::Analyzer::Product)

#endif // USERFEEDBACK_ANALYZER_PRODUCT_H
