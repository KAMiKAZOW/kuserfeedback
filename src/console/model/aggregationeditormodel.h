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

#ifndef USERFEEDBACK_CONSOLE_AGGREGATIONEDITORMODEL_H
#define USERFEEDBACK_CONSOLE_AGGREGATIONEDITORMODEL_H

#include <core/product.h>

#include <QAbstractTableModel>

namespace UserFeedback {
namespace Console {

class AggregationEditorModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit AggregationEditorModel(QObject *parent = nullptr);
    ~AggregationEditorModel();

    Product product() const;
    void setProduct(const Product &product);

    int columnCount(const QModelIndex & parent) const override;
    int rowCount(const QModelIndex & parent) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role) override;

private:
    Product m_product;
};
}
}

#endif // USERFEEDBACK_CONSOLE_AGGREGATIONEDITORMODEL_H
