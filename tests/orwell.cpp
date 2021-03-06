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

#include "orwell.h"
#include "ui_orwell.h"

#include <FeedbackConfigDialog>
#include <NotificationPopup>
#include <StyleInfoSource>
#include <ApplicationVersionSource>
#include <CompilerInfoSource>
#include <CpuInfoSource>
#include <LocaleInfoSource>
#include <OpenGLInfoSource>
#include <PlatformInfoSource>
#include <PropertyRatioSource>
#include <QPAInfoSource>
#include <QtVersionSource>
#include <ScreenInfoSource>
#include <StartCountSource>
#include <SurveyInfo>
#include <UsageTimeSource>

#include <QApplication>
#include <QDesktopServices>
#include <QSettings>

static std::unique_ptr<KUserFeedback::Provider> provider; // TODO make this nicer

Orwell::Orwell(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::Orwell)
{
    ui->setupUi(this);
    loadStats();

    connect(ui->version, &QLineEdit::textChanged, this, [this]() {
        QCoreApplication::setApplicationVersion(ui->version->text());
    });

    connect(ui->submitButton, &QPushButton::clicked, provider.get(), &KUserFeedback::Provider::submit);
    connect(ui->overrideButton, &QPushButton::clicked, this, [this] (){
        writeStats();
        QMetaObject::invokeMethod(provider.get(), "load");
        loadStats();
    });

    connect(ui->surveyInterval, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [](int value) {
        provider->setSurveyInterval(value);
    });

    ui->actionContribute->setVisible(provider->isEnabled());
    connect(ui->actionContribute, &QAction::triggered, this, [this]() {
        KUserFeedback::FeedbackConfigDialog dlg(this);
        dlg.setFeedbackProvider(provider.get());
        dlg.exec();
    });

    connect(ui->actionQuit, &QAction::triggered, qApp, &QCoreApplication::quit);

    auto propertyMonitorSource = new KUserFeedback::PropertyRatioSource(ui->dial, "value", QStringLiteral("dialRatio"));
    propertyMonitorSource->setDescription(QStringLiteral("The dial position."));
    propertyMonitorSource->addValueMapping(0, QStringLiteral("off"));
    propertyMonitorSource->addValueMapping(11, QStringLiteral("max"));
    propertyMonitorSource->setTelemetryMode(KUserFeedback::Provider::DetailedUsageStatistics);
    provider->addDataSource(propertyMonitorSource);
    auto notifyPopup = new KUserFeedback::NotificationPopup(this);
    notifyPopup->setFeedbackProvider(provider.get());
}

Orwell::~Orwell() = default;

void Orwell::loadStats()
{
    ui->version->setText(QCoreApplication::applicationVersion());

    QSettings settings(QStringLiteral("KDE"), QStringLiteral("UserFeedback.org.kde.orwell"));
    ui->startCount->setValue(settings.value(QStringLiteral("UserFeedback/ApplicationStartCount")).toInt());
    ui->runtime->setValue(settings.value(QStringLiteral("UserFeedback/ApplicationTime")).toInt());
    ui->surveys->setText(settings.value(QStringLiteral("UserFeedback/CompletedSurveys")).toStringList().join(QStringLiteral(", ")));
    ui->surveyInterval->setValue(provider->surveyInterval());
}

void Orwell::writeStats()
{
    QSettings settings(QStringLiteral("KDE"), QStringLiteral("UserFeedback.org.kde.orwell"));
    settings.setValue(QStringLiteral("UserFeedback/ApplicationStartCount"), ui->startCount->value());
    settings.setValue(QStringLiteral("UserFeedback/ApplicationTime"), ui->runtime->value());
    settings.setValue(QStringLiteral("UserFeedback/CompletedSurveys"), ui->surveys->text().split(QStringLiteral(", ")));
}


int main(int argc, char** argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("orwell"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1984"));

    QApplication app(argc, argv);

    provider.reset(new KUserFeedback::Provider);
    provider->setFeedbackServer(QUrl(QStringLiteral("https://feedback.volkerkrause.eu")));
    provider->setSubmissionInterval(1);
    provider->setApplicationStartsUntilEncouragement(5);
    provider->setEncouragementDelay(10);
    provider->addDataSource(new KUserFeedback::ApplicationVersionSource);
    provider->addDataSource(new KUserFeedback::CompilerInfoSource);
    provider->addDataSource(new KUserFeedback::CpuInfoSource);
    provider->addDataSource(new KUserFeedback::LocaleInfoSource);
    provider->addDataSource(new KUserFeedback::OpenGLInfoSource);
    provider->addDataSource(new KUserFeedback::PlatformInfoSource);
    provider->addDataSource(new KUserFeedback::QPAInfoSource);
    provider->addDataSource(new KUserFeedback::QtVersionSource);
    provider->addDataSource(new KUserFeedback::ScreenInfoSource);
    provider->addDataSource(new KUserFeedback::StartCountSource);
    provider->addDataSource(new KUserFeedback::UsageTimeSource);
    provider->addDataSource(new KUserFeedback::StyleInfoSource);

    Orwell mainWindow;
    mainWindow.show();

    return app.exec();
}
