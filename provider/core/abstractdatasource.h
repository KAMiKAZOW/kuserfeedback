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

#ifndef USERFEEDBACK_ABSTRACTDATASOURCE_H
#define USERFEEDBACK_ABSTRACTDATASOURCE_H

#include "userfeedbackcore_export.h"
#include "provider.h"

class QJsonObject;

namespace UserFeedback {

class AbstractDataSourcePrivate;

/** Base class for data sources for statistical data. */
class USERFEEDBACKCORE_EXPORT AbstractDataSource
{
public:
    AbstractDataSource();
    virtual ~AbstractDataSource();

    /** Override this to serialize the data you collected. */
    virtual void toJson(QJsonObject &obj) = 0;

    /** Returns which colleciton mode this data source belongs to. */
    Provider::StatisticsCollectionMode collectionMode() const;
    /** Sets which colleciton mode this data source belongs to. */
    void setCollectionMode(Provider::StatisticsCollectionMode mode);

protected:
    explicit AbstractDataSource(AbstractDataSourcePrivate *dd);
    class AbstractDataSourcePrivate* const d_ptr;

private:
    Q_DECLARE_PRIVATE(AbstractDataSource)
    Q_DISABLE_COPY(AbstractDataSource)
};
}

#endif // USERFEEDBACK_ABSTRACTDATASOURCE_H
