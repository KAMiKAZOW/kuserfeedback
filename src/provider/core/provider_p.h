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

#ifndef USERFEEDBACK_PROVIDER_P_H
#define USERFEEDBACK_PROVIDER_P_H

#include "provider.h"

#include <QDateTime>
#include <QStringList>
#include <QTime>
#include <QTimer>
#include <QVector>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
QT_END_NAMESPACE

namespace UserFeedback {
class ProviderPrivate
{
public:
    ProviderPrivate(Provider *qq);
    ~ProviderPrivate();

    void reset();
    int currentApplicationTime() const;

    void load();
    void store();

    void aboutToQuit();

    QByteArray jsonData() const;
    void scheduleNextSubmission();
    void submitFinished();

    void selectSurvey(const SurveyInfo &survey) const;

    void scheduleEncouragement();
    void emitShowEncouragementMessage();

    Provider *q;

    QString productId;

    QTimer submissionTimer;
    QNetworkAccessManager *networkAccessManager;
    QUrl serverUrl;
    QDateTime lastSubmitTime;
    int submissionInterval;
    Provider::StatisticsCollectionMode statisticsMode;

    int surveyInterval;
    QDateTime lastSurveyTime;
    QStringList completedSurveys;

    QTime startTime;
    int startCount;
    int usageTime;

    QTimer encouragementTimer;
    int encouragementStarts;
    int encouragementTime;
    int encouragementDelay;
    bool encouragementDisplayed;

    QVector<AbstractDataSource*> dataSources;
};
}

#endif
