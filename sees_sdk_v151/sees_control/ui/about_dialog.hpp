/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

/**
 * @brief The AboutDialog widget. Shows version info about the software.
 */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = 0);
    ~AboutDialog();

private slots:
    void on_pushButtonOk_clicked();

private:
    Ui::AboutDialog *ui;
    QString getClickableLabelText(QString url, QString title = nullptr);
};

#endif // ABOUT_DIALOG_H
