/*
    Copyright (C) 2016 Volker Krause <vkrause@kde.org>

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#ifndef KUSERFEEDBACK_CONSOLE_AGGREGATIONELEMENTMODEL_H
#define KUSERFEEDBACK_CONSOLE_AGGREGATIONELEMENTMODEL_H

#include <core/aggregation.h>

#include <QAbstractListModel>
#include <QVector>

namespace KUserFeedback {
namespace Console {

class Product;

class AggregationElementModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit AggregationElementModel(QObject *parent = nullptr);
    ~AggregationElementModel();

    void setProduct(const Product &product);

    int rowCount(const QModelIndex & parent) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    QModelIndexList match(const QModelIndex & start, int role, const QVariant & value, int hits, Qt::MatchFlags flags) const override;

private:
    QVector<AggregationElement> m_elements;
};
}
}

#endif // KUSERFEEDBACK_CONSOLE_AGGREGATIONELEMENTMODEL_H
