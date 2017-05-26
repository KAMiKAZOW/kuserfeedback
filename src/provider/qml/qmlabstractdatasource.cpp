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

#include "qmlabstractdatasource.h"

#include <AbstractDataSource>

using namespace UserFeedback;

QmlAbstractDataSource::QmlAbstractDataSource(AbstractDataSource *source, QObject* parent)
    : QObject(parent)
    , m_source(source)
{
}

QmlAbstractDataSource::~QmlAbstractDataSource()
{
}

Provider::StatisticsCollectionMode QmlAbstractDataSource::collectionMode() const
{
    return m_source->collectionMode();
}

void QmlAbstractDataSource::setCollectionMode(Provider::StatisticsCollectionMode mode)
{
    if (m_source->collectionMode() == mode)
        return;
    m_source->setCollectionMode(mode);
    emit collectionModeChanged();
}

AbstractDataSource* QmlAbstractDataSource::source() const
{
    return m_source;
}