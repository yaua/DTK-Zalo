QT += core widgets gui webengine webenginewidgets svg concurrent

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget

SOURCES += main.cpp\
        mainwindow.cpp \
        webenginepage.cpp \
        widget.cpp

HEADERS  += mainwindow.h \
         webenginepage.h \
         widget.h \
         main.h

FORMS += widget.ui

RESOURCES += imgs.qrc


