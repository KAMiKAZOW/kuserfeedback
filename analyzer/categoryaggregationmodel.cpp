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

#include "categoryaggregationmodel.h"
#include "sample.h"
#include "timeaggregationmodel.h"

#include <QDebug>
#include <QSet>

#include <algorithm>
#include <string.h>

using namespace UserFeedback::Analyzer;

CategoryAggregationModel::CategoryAggregationModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

CategoryAggregationModel::~CategoryAggregationModel()
{
    delete[] m_data;
}

void CategoryAggregationModel::setSourceModel(QAbstractItemModel* model)
{
    Q_ASSERT(model);
    m_sourceModel = model;
    connect(model, &QAbstractItemModel::modelReset, this, &CategoryAggregationModel::recompute);
    recompute();
}

int CategoryAggregationModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_categories.size() + 1;
}

int CategoryAggregationModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_sourceModel)
        return 0;
    return m_sourceModel->rowCount();
}

QVariant CategoryAggregationModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_sourceModel)
        return {};

    if (index.column() == 0) {
        const auto srcIdx = m_sourceModel->index(index.row(), 0);
        return m_sourceModel->data(srcIdx, role);
    }
    if (role == Qt::DisplayRole) {
        return m_data[index.row() * m_categories.size() + index.column() - 1];
    } else if (role == TimeAggregationModel::MaximumValueRole) {
        return m_maxValue;
    }

    return {};
}

QVariant CategoryAggregationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && m_sourceModel) {
        if (section == 0)
            return m_sourceModel->headerData(section, orientation, role);
        if (role == Qt::DisplayRole) {
            const auto cat = m_categories.at(section - 1);
            if (cat.isEmpty())
                return tr("<empty>");
            return cat;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

void CategoryAggregationModel::recompute()
{
    Q_ASSERT(m_sourceModel);
    const auto rowCount = m_sourceModel->rowCount();
    beginResetModel();
    m_categories.clear();
    delete[] m_data;
    m_maxValue = 0;

    if (rowCount <= 0) {
        endResetModel();
        return;
    }

    // scan all samples to find all categories
    // TODO do this in the database
    QSet<QString> categories;
    const auto allSamples = m_sourceModel->index(0, 0).data(TimeAggregationModel::AllSamplesRole).value<QVector<Sample>>();
    foreach (const auto &s, allSamples)
        categories.insert(s.version()); // TODO customize this
    m_categories.reserve(categories.size());
    foreach (const auto &cat, categories)
        m_categories.push_back(cat);
    std::sort(m_categories.begin(), m_categories.end());

    // compute the counts per cell, we could do that on demand, but we need the maximum for QtCharts...
    m_data = new int[m_categories.size() * rowCount];
    memset(m_data, 0, sizeof(int) * m_categories.size() * rowCount);
    for (int row = 0; row < rowCount; ++row) {
        const auto samples = m_sourceModel->index(row, 0).data(TimeAggregationModel::SamplesRole).value<QVector<Sample>>();
        foreach (const auto &sample, samples) {
            const auto catIt = std::lower_bound(m_categories.constBegin(), m_categories.constEnd(), sample.version());
            Q_ASSERT(catIt != m_categories.constEnd());
            const auto idx = m_categories.size() * row + std::distance(m_categories.constBegin(), catIt);
            m_data[idx]++;
            m_maxValue = std::max(m_maxValue, m_data[idx]);
        }
    }

    endResetModel();
}

