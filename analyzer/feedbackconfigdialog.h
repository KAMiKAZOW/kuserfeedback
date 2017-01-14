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

#ifndef USERFEEDBACK_ANALYZER_FEEDBACKCONFIGDIALOG_H
#define USERFEEDBACK_ANALYZER_FEEDBACKCONFIGDIALOG_H

#include <QDialog>

#include <memory>

namespace UserFeedback {

class Provider;

namespace Analyzer {

namespace Ui
{
class FeedbackConfigDialog;
}

/** Configure which feedback a user wants to provide. */
class FeedbackConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FeedbackConfigDialog(QWidget *parent = nullptr);
    ~FeedbackConfigDialog();

    void setFeedbackProvider(UserFeedback::Provider *provider);
    void accept() override;

private:
    void linkActivated(const QString &link);
    void updateButtonState();

    std::unique_ptr<Ui::FeedbackConfigDialog> ui;
    UserFeedback::Provider *m_provider;
};

}
}

#endif // USERFEEDBACK_ANALYZER_FEEDBACKCONFIGDIALOG_H
