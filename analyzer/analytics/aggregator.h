/*
    Copyright (C) 2017 Volker Krause <vkrause@kde.org>

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

#ifndef USERFEEDBACK_ANALYZER_AGGREGATOR_H
#define USERFEEDBACK_ANALYZER_AGGREGATOR_H

#include <core/aggregation.h>

class QAbstractItemModel;

namespace QtCharts {
class QChart;
}

namespace UserFeedback {
namespace Analyzer {

class Aggregation;

class Aggregator
{
public:
    explicit Aggregator();
    virtual ~Aggregator();

    enum ChartMode {
        None = 0x0,
        Singular = 0x1,
        Timeline = 0x2
    };
    Q_DECLARE_FLAGS(ChartModes, ChartMode)

    Aggregation aggregation() const;
    void setAggregation(const Aggregation &aggr);

    QAbstractItemModel *sourceModel() const;
    void setSourceModel(QAbstractItemModel *model);

    virtual QString displayName() const = 0;

    virtual ChartModes chartModes() const;

    virtual QAbstractItemModel *timeAggregationModel();

    virtual QtCharts::QChart *singlularChart();
    virtual QtCharts::QChart *timelineChart();

private:
    QAbstractItemModel *m_sourceModel = nullptr;
    Aggregation m_aggregation;
};

}}

Q_DECLARE_METATYPE(UserFeedback::Analyzer::Aggregator*)
Q_DECLARE_OPERATORS_FOR_FLAGS(UserFeedback::Analyzer::Aggregator::ChartModes)

#endif // USERFEEDBACK_ANALYZER_AGGREGATOR_H
