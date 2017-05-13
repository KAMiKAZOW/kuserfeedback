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

#include "feedbackconfigdialog.h"
#include "ui_feedbackconfigdialog.h"

#include <provider.h>

#include <QDebug>
#include <QPushButton>

using namespace UserFeedback;

namespace UserFeedback {
class FeedbackConfigDialogPrivate {
public:
    void updateButtonState();

    std::unique_ptr<Ui::FeedbackConfigDialog> ui;
};
}

FeedbackConfigDialog::FeedbackConfigDialog(QWidget *parent) :
    QDialog(parent),
    d(new FeedbackConfigDialogPrivate)
{
    d->ui.reset(new Ui::FeedbackConfigDialog);
    d->ui->setupUi(this);
    d->ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Contribute!"));
    d->ui->buttonBox->button(QDialogButtonBox::Close)->setText(tr("No, I do not want to contribute."));
}

FeedbackConfigDialog::~FeedbackConfigDialog()
{
}

void FeedbackConfigDialog::setFeedbackProvider(UserFeedback::Provider* provider)
{
    d->ui->configWidget->setFeedbackProvider(provider);
    connect(d->ui->configWidget, SIGNAL(configurationChanged()), this, SLOT(updateButtonState()));
    d->updateButtonState();
}

void FeedbackConfigDialog::accept()
{
    auto p = d->ui->configWidget->feedbackProvider();
    p->setStatisticsCollectionMode(d->ui->configWidget->statisticsCollectionMode());
    p->setSurveyInterval(d->ui->configWidget->surveyInterval());
    QDialog::accept();
}

void FeedbackConfigDialogPrivate::updateButtonState()
{
    const auto any = ui->configWidget->surveyInterval() >= 0
        || ui->configWidget->statisticsCollectionMode() != Provider::NoStatistics;

    ui->buttonBox->button(QDialogButtonBox::Ok)->setVisible(any);
    ui->buttonBox->button(QDialogButtonBox::Close)->setVisible(!any);
}

#include "moc_feedbackconfigdialog.cpp"
