/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#include "about_dialog.hpp"
#include "ui_about_dialog.h"

#include "versioninfo.hpp"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // set version info
    ui->label_application->setText(version_info::application_name + " " + version_info::version);
    std::wstring str = L"\u24D2";
    QString copyright = QString::fromStdWString(str);
    QString company_text = copyright + ' ' + version_info::company_name;
    ui->label_company_name->setText(company_text);
    ui->label_company_url->setText(getClickableLabelText(version_info::company_url));
    if (version_info::wiki_url != "")
        ui->label_wiki_url->setText(getClickableLabelText(version_info::wiki_url));
    else
        ui->label_wiki_url->setText("");
}

AboutDialog::~AboutDialog()
{
    delete ui;
}


void AboutDialog::on_pushButtonOk_clicked()
{
    close();
}

QString AboutDialog::getClickableLabelText(QString url, QString title)
{
    if(title == nullptr)
        title = url;
    QString labelText = "<a href=\"" + url + "\">" + title + "</a>";
    return labelText;
}
