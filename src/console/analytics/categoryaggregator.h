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

#ifndef KUSERFEEDBACK_CONSOLE_CATEGORYAGGREGATOR_H
#define KUSERFEEDBACK_CONSOLE_CATEGORYAGGREGATOR_H

#include "aggregator.h"

#include <memory>
#include <vector>

namespace QtCharts {
class QPieSeries;
}

namespace KUserFeedback {
namespace Console {

class CategoryAggregationModel;
class SingleRowFilterProxyModel;

class CategoryAggregator : public Aggregator
{
public:
    CategoryAggregator();
    ~CategoryAggregator();

    ChartModes chartModes() const override;
    QAbstractItemModel* timeAggregationModel() override;
    QtCharts::QChart* timelineChart() override;
    QtCharts::QChart* singlularChart() override;
    void setSingularTime(int row) override;

private:
    void updateTimelineChart();
    void updateSingularChart();
    void decorateSeries(QtCharts::QPieSeries *series, int ring) const;

    std::unique_ptr<CategoryAggregationModel> m_model;
    std::unique_ptr<QtCharts::QChart> m_timelineChart;
    std::unique_ptr<QtCharts::QChart> m_singularChart;
    std::vector<SingleRowFilterProxyModel*> m_hierachicalCategories;
};

}}

#endif // KUSERFEEDBACK_CONSOLE_CATEGORYAGGREGATOR_H
