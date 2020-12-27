/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#include "ui/mainwindow.hpp"
#include <QApplication>

#include "versioninfo.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // configure where user settings are stored
    QCoreApplication::setOrganizationName(version_info::company_name);
    QCoreApplication::setOrganizationDomain(version_info::company_url);
    QCoreApplication::setApplicationName(version_info::application_name);

    MainWindow w;
    w.show();

    return a.exec();
}
