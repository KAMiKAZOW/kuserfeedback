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

#include <config-userfeedback-version.h>
#include "provider.h"
#include "surveyinfo.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QStringList>
#include <QTime>
#include <QTimer>
#include <QUrl>

#include <algorithm>
#include <numeric>

using namespace UserFeedback;

namespace UserFeedback {
class ProviderPrivate
{
public:
    ProviderPrivate(Provider *qq);

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
};
}

ProviderPrivate::ProviderPrivate(Provider *qq)
    : q(qq)
    , networkAccessManager(Q_NULLPTR)
    , submissionInterval(-1)
    , statisticsMode(Provider::NoStatistics)
    , surveyInterval(-1)
    , startCount(0)
    , usageTime(0)
    , encouragementStarts(-1)
    , encouragementTime(-1)
    , encouragementDelay(300)
    , encouragementDisplayed(false)
{
    auto domain = QCoreApplication::organizationDomain().split(QLatin1Char('.'));
    std::reverse(domain.begin(), domain.end());
    productId = domain.join(QLatin1Char('.')) + QLatin1Char('.') + QCoreApplication::applicationName();

    submissionTimer.setSingleShot(true);
    QObject::connect(&submissionTimer, SIGNAL(timeout()), q, SLOT(submit()));

    startTime.start();

    encouragementTimer.setSingleShot(true);
    QObject::connect(&encouragementTimer, SIGNAL(timeout()), q, SLOT(emitShowEncouragementMessage()));
}

void ProviderPrivate::reset()
{
    startCount = 0;
    usageTime = 0;
    startTime.start();
}

int ProviderPrivate::currentApplicationTime() const
{
    return usageTime + (startTime.elapsed() / 1000);
}

void ProviderPrivate::load()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("UserFeedback"));
    lastSubmitTime = settings.value(QStringLiteral("LastSubmission")).toDateTime();
    // TODO store as string via QMetaEnum
    statisticsMode = static_cast<Provider::StatisticsCollectionMode>(settings.value(QStringLiteral("StatisticsCollectionMode"), Provider::NoStatistics).toInt());

    surveyInterval = settings.value(QStringLiteral("SurveyInterval"), -1).toInt();
    lastSurveyTime = settings.value(QStringLiteral("LastSurvey")).toDateTime();
    completedSurveys = settings.value(QStringLiteral("CompletedSurveys"), QStringList()).toStringList();

    startCount = std::max(settings.value(QStringLiteral("ApplicationStartCount"), 0).toInt() + 1, 1);
    usageTime = std::max(settings.value(QStringLiteral("ApplicationTime"), 0).toInt(), 0);

    encouragementDisplayed = settings.value(QStringLiteral("EncouragementDisplayed"), false).toBool();
}

void ProviderPrivate::store()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("UserFeedback"));
    settings.setValue(QStringLiteral("LastSubmission"), lastSubmitTime);
    settings.setValue(QStringLiteral("StatisticsCollectionMode"), statisticsMode);

    settings.setValue(QStringLiteral("SurveyInterval"), surveyInterval);
    settings.setValue(QStringLiteral("LastSurvey"), lastSurveyTime);
    settings.setValue(QStringLiteral("CompletedSurveys"), completedSurveys);

    settings.setValue(QStringLiteral("ApplicationStartCount"), startCount);
    settings.setValue(QStringLiteral("ApplicationTime"), currentApplicationTime());

    settings.setValue(QStringLiteral("EncouragementDisplayed"), encouragementDisplayed);
}

void ProviderPrivate::aboutToQuit()
{
    qDebug() << Q_FUNC_INFO;
    store();
}

QByteArray ProviderPrivate::jsonData() const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("productId"), productId);

    if (statisticsMode != Provider::NoStatistics) {
        obj.insert(QStringLiteral("startCount"), startCount);
        obj.insert(QStringLiteral("usageTime"), currentApplicationTime());
        obj.insert(QStringLiteral("version"), QCoreApplication::applicationVersion());
    }

    QJsonDocument doc(obj);
    return doc.toJson();
}

void ProviderPrivate::scheduleNextSubmission()
{
    submissionTimer.stop();
    if (submissionInterval <= 0 || (statisticsMode == Provider::NoStatistics && surveyInterval < 0))
        return;

    Q_ASSERT(submissionInterval > 0);

    const auto nextSubmission = lastSubmitTime.addDays(submissionInterval);
    const auto now = QDateTime::currentDateTime();
    submissionTimer.start(std::max(0ll, now.msecsTo(nextSubmission)));
}

void ProviderPrivate::submitFinished()
{
    auto reply = qobject_cast<QNetworkReply*>(q->sender());
    Q_ASSERT(reply);

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "failed to submit user feedback:" << reply->errorString();
        return;
    }

    lastSubmitTime = QDateTime::currentDateTime();

    const auto obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.contains(QStringLiteral("survey"))) {
        const auto surveyObj = obj.value(QStringLiteral("survey")).toObject();
        const auto survey = SurveyInfo::fromJson(surveyObj);
        selectSurvey(survey);
    }

    scheduleNextSubmission();
}

void ProviderPrivate::selectSurvey(const SurveyInfo &survey) const
{
    qDebug() << Q_FUNC_INFO << "got survey:" << survey.url();
    if (surveyInterval < 0) // surveys disabled
        return;

    if (!survey.isValid() || completedSurveys.contains(QString::number(survey.id())))
        return;

    if (lastSurveyTime.addDays(surveyInterval) > QDateTime::currentDateTime())
        return;

    emit q->surveyAvailable(survey);
}

void ProviderPrivate::scheduleEncouragement()
{
    encouragementTimer.stop();
    if (encouragementStarts < 0 && encouragementTime < 0) // encouragement disabled
        return;

    if (encouragementStarts > startCount) // we need more starts
        return;

    if (statisticsMode == Provider::AllStatistics && surveyInterval == 0) // already everything enabled
        return;

    Q_ASSERT(encouragementDelay >= 0);
    int timeToEncouragement = encouragementDelay;
    if (encouragementTime > 0)
        timeToEncouragement = std::max(timeToEncouragement, (encouragementTime * 60) - currentApplicationTime());
    encouragementTimer.start(timeToEncouragement);
}

void ProviderPrivate::emitShowEncouragementMessage()
{
    encouragementDisplayed = true; // TODO make this explicit, in case the host application decides to delay?
    emit q->showEncouragementMessage();
}


Provider::Provider(QObject *parent) :
    QObject(parent),
    d(new ProviderPrivate(this))
{
    qDebug() << Q_FUNC_INFO;

    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));

    d->load();
}

Provider::~Provider()
{
    delete d;
}

void Provider::setProductIdentifier(const QString &productId)
{
    d->productId = productId;
}

void Provider::setFeedbackServer(const QUrl &url)
{
    d->serverUrl = url;
}

void Provider::setSubmissionInterval(int days)
{
    d->submissionInterval = days;
    d->scheduleNextSubmission();
}

Provider::StatisticsCollectionMode Provider::statisticsCollectionMode() const
{
    return d->statisticsMode;
}

void Provider::setStatisticsCollectionMode(StatisticsCollectionMode mode)
{
    d->statisticsMode = mode;
    d->scheduleNextSubmission();
    d->scheduleEncouragement();
}

int Provider::surveyInterval() const
{
    return d->surveyInterval;
}

void Provider::setSurveyInterval(int days)
{
    d->surveyInterval = days;
    d->scheduleNextSubmission();
    d->scheduleEncouragement();
}

void Provider::setApplicationStartsUntilEncouragement(int starts)
{
    d->encouragementStarts = starts;
    d->scheduleEncouragement();
}

void Provider::setApplicationUsageTimeUntilEncouragement(int minutes)
{
    d->encouragementTime = minutes;
    d->scheduleEncouragement();
}

void Provider::setEncouragementDelay(int secs)
{
    d->encouragementDelay = std::max(0, secs);
    d->scheduleEncouragement();
}

void Provider::setSurveyCompleted(const SurveyInfo &info)
{
    d->completedSurveys.push_back(QString::number(info.id()));
    d->lastSurveyTime = QDateTime::currentDateTime();
    d->store();
}

void Provider::submit()
{
    if (!d->serverUrl.isValid()) {
        qWarning() << "No feedback server URL specified!";
        return;
    }

    if (!d->networkAccessManager)
        d->networkAccessManager = new QNetworkAccessManager(this);

    auto url = d->serverUrl;
    url.setPath(url.path() + QStringLiteral("/receiver/submit"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("UserFeedback/") + QStringLiteral(USERFEEDBACK_VERSION));
    auto reply = d->networkAccessManager->post(request, d->jsonData());
    connect(reply, SIGNAL(finished()), this, SLOT(submitFinished()));
}

#include "moc_provider.cpp"
