#include "mainwindow.h"
#include <DMainWindow>
#include <DWidgetUtil>
#include <DTitlebar>
#include <DMessageManager>
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QCloseEvent>
#include <QDebug>
#include "webenginepage.h"

MainWindow::MainWindow(QString yTitle,
                       QString yUrl,
                       int nWidth,
                       int nHeight,
                       bool tray,
                       bool nFullScreen,
                       bool nFixSize,
                       bool nHideButtons,
                       DAboutDialog *dialog,
                       QWidget *parent)
    : DMainWindow(parent)
    , m_widget(new Widget(yUrl))
    , m_dialog(dialog)
    , m_tray(new QSystemTrayIcon)
    , btnBack(new DToolButton(titlebar()))
    , btnForward(new DToolButton(titlebar()))
    , btnRefresh(new DToolButton(titlebar()))
    , m_menu(new QMenu)
    , m_fullScreen(new QAction(tr("Full Screen")))
    , m_fixSize(new QAction(tr("Fix Size")))
    , m_hideButtons(new QAction(tr("Hide Buttons")))
    , t_menu(new QMenu)
    , t_show(new QAction(tr("Open Zalo")))
    , t_exit(new QAction(tr("Exit")))
    , downloadProgressBar(new QWidget)
    , bar(new DProgressBar)
    , pause(new DPushButton(tr("Pause")))
    , resume(new DPushButton(tr("Resume")))
    , cancel(new DPushButton(tr("Cancel")))
    , progress(new QHBoxLayout)
    , message(new DFloatingMessage(DFloatingMessage::ResidentType))
    , process(new QProcess)
    , isCanceled(false)
    , mtray(tray)
    , mFixSize(nFixSize)
    , m_width(nWidth)
    , m_height(nHeight)
{
    setCentralWidget(m_widget);
    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
    resize(m_width, m_height);
    moveToCenter(this);
    setWindowIcon(QIcon(":/images/zalo.svg"));
    titlebar()->setTitle(yTitle);
    titlebar()->setIcon(QIcon(":/images/zalo.svg"));
    btnBack->setIcon(QIcon(":/images/go-previous-24.svg"));
    btnBack->setIconSize(QSize(20, 20));
    btnForward->setIcon(QIcon(":/images/go-next-24.svg"));
    btnForward->setIconSize(QSize(20, 20));
    btnRefresh->setIcon(QIcon(":/images/view-refresh.svg"));
    btnRefresh->setIconSize(QSize(20, 20));
    titlebar()->addWidget(btnBack, Qt::AlignLeft);
    titlebar()->addWidget(btnForward, Qt::AlignLeft);
    titlebar()->addWidget(btnRefresh, Qt::AlignLeft);
    m_fullScreen->setCheckable(true);
    m_fullScreen->setChecked(nFullScreen);
    m_fullScreen->setDisabled(nFixSize);
    m_fixSize->setCheckable(true);
    m_fixSize->setChecked(nFixSize);
    m_fixSize->setDisabled(nFixSize);
    m_hideButtons->setCheckable(true);
    m_hideButtons->setChecked(nHideButtons);
    m_hideButtons->setDisabled(nHideButtons);
    if(!nFixSize)
    {
        m_menu->addAction(m_fullScreen);
        m_menu->addAction(m_fixSize);
    }
    if(!nHideButtons)
    {
        m_menu->addAction(m_hideButtons);
    }
    titlebar()->setMenu(m_menu);
    titlebar()->setAutoHideOnFullscreen(true);
    fixSize();
    hideButtons();
    /* TrayIcon */
    t_menu->addAction(t_show);
    t_menu->addAction(t_exit);
    m_tray->setContextMenu(t_menu);
    m_tray->setToolTip(yTitle);
    m_tray->setIcon(QIcon(":/images/zalo.svg"));
    if(tray)
    {
        m_tray->show();
    }
    bar->setFixedSize(250, 8);
    progress->addWidget(bar);
    progress->addSpacing(5);
    progress->addWidget(pause);
    progress->addWidget(resume);
    progress->addWidget(cancel);
    downloadProgressBar->setLayout(progress);
    message->setIcon(QIcon::fromTheme("deepin-download").pixmap(64, 64));
    message->setWidget(downloadProgressBar);
    connect(btnBack, &DToolButton::clicked, this, [&]()
    {
        m_widget->goBack();
    });
    connect(btnForward, &DToolButton::clicked, this, [&]()
    {
        m_widget->goForward();
    });
    connect(btnRefresh, &DToolButton::clicked, this, [&]()
    {
        m_widget->refresh();
    });

    connect(m_fullScreen, &QAction::triggered, this, [=]()
    {
        fullScreen();
    });
    connect(m_fixSize, &QAction::triggered, this, [=]()
    {
        fixSize();
    });
    connect(m_hideButtons, &QAction::triggered, this, [=]()
    {
        hideButtons();
    });

    connect(t_show, &QAction::triggered, this, [=]()
    {
        this->activateWindow();
        fixSize();
    });
    connect(t_exit, &QAction::triggered, this, [=]()
    {
        exit(0);
    });
    connect(m_tray, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActivated);

    connect(m_widget->getPage()->profile(), &QWebEngineProfile::downloadRequested, this, &MainWindow::on_downloadStart);
}
MainWindow::~MainWindow()
{
    emit sigQuit();
    delete m_widget;
    delete m_tray;
}
void MainWindow::setIcon(QString yIconPath)
{
    QFileInfo fi(yIconPath);
    if (fi.exists())
    {
        titlebar()->setIcon(QIcon(yIconPath));
        setWindowIcon(QIcon(yIconPath));
        m_tray->setIcon(QIcon(yIconPath));
        qDebug() << yIconPath << "is Set!";
    }
    else
    {
        qDebug() << yIconPath << "is Not Exists!";
    }
}
void MainWindow::fullScreen()
{
    if(m_fullScreen->isChecked())
    {
        m_fixSize->setChecked(false);
        m_fixSize->setDisabled(true);
        m_menu->update();
        showFullScreen();
        //DMessageManager::instance()->sendMessage(this, QIcon::fromTheme("dialog-information").pixmap(64, 64), QString(tr("%1Fullscreen Mode")).arg("    "));
    }
    else
    {
        if(!mFixSize)
        {
            m_fixSize->setDisabled(false);
        }
        m_menu->update();
        showNormal();
        //DMessageManager::instance()->sendMessage(this, QIcon::fromTheme("dialog-information").pixmap(64, 64), QString(tr("%1Windowed Mode")).arg("    "));
    }
}
void MainWindow::fixSize()
{
    if(m_fixSize->isChecked())
    {
        m_fullScreen->setChecked(false);
        m_fullScreen->setDisabled(true);
        m_menu->update();
        setFixedSize(this->size());
    }
    else
    {
        m_fullScreen->setDisabled(false);
        m_menu->update();
        setMinimumSize(m_width, m_height);
        setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    }
    fullScreen();
}
void MainWindow::hideButtons()
{
    if(m_hideButtons->isChecked())
    {
        btnBack->hide();
        btnForward->hide();
        btnRefresh->hide();
    }
    else
    {
        btnBack->show();
        btnForward->show();
        btnRefresh->show();
    }
}
QString MainWindow::saveAs(QString fileName)
{
    QString saveFile = QFileDialog::getSaveFileName(this, tr("Save As"), QDir::homePath() + "/Downloads/" + fileName);
    if(!saveFile.isEmpty())
    {
        if(QFileInfo(QFileInfo(saveFile).absolutePath()).permissions().testFlag(QFile::WriteUser))  // Determine whether the upper directory is writable
        {
            return saveFile;
        }
        else
        {
            return saveAs(fileName);
        }
    }
    return nullptr;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(!m_fixSize->isChecked())
    {
        if(event->key() == Qt::Key_F11)
        {
            m_fullScreen->trigger();
            m_menu->update();
        }
        event->accept();
    }
}
void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
        case QSystemTrayIcon::Trigger:
            this->activateWindow();
            fixSize();
            break;
        default:
            break;
    }
}

void MainWindow::on_downloadStart(QWebEngineDownloadItem *item)
{
    if(mutex.tryLock())
    {
        QString fileName = QFileInfo(item->path()).fileName();
        QString filePath = saveAs(fileName);
        if(filePath.isEmpty())
        {
            return;
        }
        item->setPath(filePath);
        filePath = QFileInfo(item->path()).absoluteFilePath();

        connect(item, &QWebEngineDownloadItem::downloadProgress, this, &MainWindow::on_downloadProgress);
        connect(item, &QWebEngineDownloadItem::finished, this, [=]()
        {
            on_downloadFinish(filePath);
        });

        connect(pause, &DPushButton::clicked, this, [=]()
        {
            on_downloadPause(item);
        });
        connect(resume, &DPushButton::clicked, this, [=]()
        {
            on_downloadResume(item);
        });
        connect(cancel, &DPushButton::clicked, this, [=]()
        {
            on_downloadCancel(item);
        });
        DFloatingMessage *message = new DFloatingMessage(DFloatingMessage::TransientType);
        message->setIcon(QIcon::fromTheme("dialog-information").pixmap(64, 64));
        message->setMessage(QString(tr("%1Start downloading %2")).arg("    ").arg(fileName));
        DMessageManager::instance()->sendMessage(this, message);
        item->accept();
        isCanceled = false;
        resume->hide();
        pause->show();
        this->message->show();
    }
    else
    {
        DMessageManager::instance()->sendMessage(this, QIcon::fromTheme("dialog-cancel").pixmap(64, 64), QString(tr("%1Wait for previous download to complete!")).arg("    "));
    }
}

void MainWindow::on_downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    int value = int(double(bytesReceived) / double(bytesTotal) * 100.0);
    bar->setValue(value);
    message->setMessage("    " + QString::number(value) + "%");
    DMessageManager::instance()->sendMessage(this, message);
}

void MainWindow::on_downloadFinish(QString filePath)
{
    mutex.unlock();
    message->hide();
    if(!isCanceled)
    {
        DPushButton *button = new DPushButton(tr("Open"));
        DFloatingMessage *message = new DFloatingMessage(DFloatingMessage::ResidentType);
        message->setIcon(QIcon::fromTheme("dialog-ok").pixmap(64, 64));
        message->setMessage(QString("    %1 %2 %3").arg(QFileInfo(filePath).fileName()).arg(tr("download finished.")).arg(tr("Show in file manager?")));
        message->setWidget(button);
        DMessageManager::instance()->sendMessage(this, message);

        connect(button, &DPushButton::clicked, this, [=]()
        {
            process->start("dde-file-manager --show-item " + filePath);
            message->hide();
        });
    }
}
void MainWindow::on_downloadPause(QWebEngineDownloadItem *item)
{
    item->pause();
    message->setIcon(QIcon::fromTheme("package-download-failed").pixmap(64, 64));
    resume->show();
    pause->hide();
}
void MainWindow::on_downloadResume(QWebEngineDownloadItem *item)
{
    item->resume();
    message->setIcon(QIcon::fromTheme("deepin-download").pixmap(64, 64));
    resume->hide();
    pause->show();
}
void MainWindow::on_downloadCancel(QWebEngineDownloadItem *item)
{
    isCanceled = true;
    item->cancel();
    mutex.unlock();
    message->hide();
    DMessageManager::instance()->sendMessage(this, QIcon::fromTheme("dialog-error").pixmap(64, 64), QString(tr("%1Download canceled!")).arg("    "));
}