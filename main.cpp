#include "mainwindow.h"
#include <DApplication>
#include <DWidgetUtil>
#include <DAboutDialog>
#include <DMainWindow>
#include <QFileInfo>
#include <QSettings>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    DAboutDialog dialog;
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
    a.setProductName("Zalo");
    a.setApplicationName("Zalo");
    a.setAboutDialog(&dialog);
    dialog.setProductIcon(QIcon(":/images/zalo.svg"));
    QString yDefaultDesc = QString("<span style=' font-size:8pt; font-weight:600;'>This is the simple app with dtk lib and WebApp Runtime. Customize by yaua</span>");
    dialog.setDescription(yDefaultDesc);
    dialog.setVersion(DApplication::buildVersion(QString("%1 %2").arg(QObject::tr("Version")).arg("1.7.0")));
    dialog.setLicense(QObject::tr("This program is open source under GPLv3"));
    QString yTitle = "Zalo";
    QString yUrl = "https://chat.zalo.me/";
    int width = 700;
    int height = 800;
    bool tray = true;
    bool hideButtons = false;
    a.setQuitOnLastWindowClosed(!tray);
    MainWindow w(yTitle, yUrl, width, height, tray, hideButtons);
    w.show();
    return a.exec();
}
