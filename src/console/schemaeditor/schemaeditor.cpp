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

#include "schemaeditor.h"
#include "ui_schemaeditor.h"

#include <core/aggregation.h>
#include <core/product.h>
#include <core/schemaentrytemplates.h>
#include <rest/restapi.h>

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkReply>

using namespace UserFeedback::Console;


SchemaEditor::SchemaEditor(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SchemaEditor)
{
    ui->setupUi(this);

    connect(ui->schema, &SchemaEditWidget::logMessage, this, &SchemaEditor::logMessage);
    connect(ui->schema, &SchemaEditWidget::productChanged, ui->aggregation, [this]() {
        ui->aggregation->setProduct(product());
    });

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &SchemaEditor::updateState);

    auto templateMenu = new QMenu(tr("Schema Entry Templates"), this);
    for (const auto &t : SchemaEntryTemplates::availableTemplates()) {
        auto a = templateMenu->addAction(t.name());
        a->setData(QVariant::fromValue(t));
        connect(a, &QAction::triggered, this, [this, a]() {
            const auto t = a->data().value<Product>();
            auto p = product();
            p.addTemplate(t);
            setProduct(p);
        });
    }

    m_createFromTemplateAction = templateMenu->menuAction();
    m_createFromTemplateAction->setIcon(QIcon::fromTheme(QStringLiteral("document-new-from-template")));

    ui->actionSave->setShortcut(QKeySequence::Save);
    connect(ui->actionSave, &QAction::triggered, this, &SchemaEditor::save);
    connect(ui->actionImportSchema, &QAction::triggered, this, &SchemaEditor::importSchema);
    connect(ui->actionExportSchema, &QAction::triggered, this, &SchemaEditor::exportSchema);

    addActions({ m_createFromTemplateAction, ui->actionSave, ui->actionImportSchema, ui->actionExportSchema });
    auto sep = new QAction(this);
    sep->setSeparator(true);
    addAction(sep);
    addActions(ui->schema->actions());
    addActions(ui->aggregation->actions());

    updateState();
}

SchemaEditor::~SchemaEditor() = default;

void SchemaEditor::setRESTClient(RESTClient* client)
{
    m_restClient = client;
    ui->schema->setRESTClient(client);
}

Product SchemaEditor::product() const
{
    auto p = ui->schema->product();
    p.setAggregations(ui->aggregation->product().aggregations());
    return p;
}

void SchemaEditor::setProduct(const Product& product)
{
    ui->schema->setProduct(product);
    ui->aggregation->setProduct(product);
    updateState();
}

void SchemaEditor::save()
{
    auto reply = RESTApi::updateProduct(m_restClient, product());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError)
            return;
        emit logMessage(QString::fromUtf8((reply->readAll())));
        emit productChanged(product());
    });
}

void SchemaEditor::exportSchema()
{
    const auto fileName = QFileDialog::getSaveFileName(this, tr("Export Schema"));
    if (fileName.isEmpty())
        return;

    QFile f(fileName);
    if (!f.open(QFile::WriteOnly)) {
        QMessageBox::critical(this, tr("Export Failed"), tr("Could not open file: %1").arg(f.errorString()));
        return;
    }
    f.write(product().toJson());
    logMessage(tr("Schema of %1 exported to %2.").arg(product().name(), f.fileName()));
}

void SchemaEditor::importSchema()
{
    const auto fileName = QFileDialog::getOpenFileName(this, tr("Import Schema"));
    if (fileName.isEmpty())
        return;

    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        QMessageBox::critical(this, tr("Import Failed"), tr("Could not open file: %1").arg(f.errorString()));
        return;
    }
    const auto products = Product::fromJson(f.readAll());
    if (products.size() != 1 || !products.at(0).isValid()) {
        QMessageBox::critical(this, tr("Import Failed"), tr("Selected file contains no valid product schema."));
        return;
    }

    auto p = products.at(0);
    p.setName(product().name());
    setProduct(p);
    logMessage(tr("Schema of %1 imported from %2.").arg(product().name(), f.fileName()));
}

void SchemaEditor::updateState()
{
    const auto p = product();

    m_createFromTemplateAction->setEnabled(p.isValid());
    ui->actionSave->setEnabled(p.isValid());
    ui->actionExportSchema->setEnabled(p.isValid());
    ui->actionImportSchema->setEnabled(p.isValid());

    const auto schemaEditActive = ui->tabWidget->currentWidget() == ui->schema;
    const auto aggrEditActive = ui->tabWidget->currentWidget() == ui->aggregation;
    for (auto action : ui->schema->actions())
        action->setVisible(schemaEditActive);
    for (auto action : ui->aggregation->actions())
        action->setVisible(aggrEditActive);
}
