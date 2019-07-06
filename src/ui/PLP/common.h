#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QColor>

//#define LC_DEPLOYMENT

static const QString SEARCH_GROUP_STYLESHEET =
        "QGroupBox {"
            "border: 2px solid silver;"
            "font: bold 14px;"
            "margin-top: 6px;"
        "}"
        "QGroupBox::title {"
            "subcontrol-origin: margin;"
            "left: 7px;"
            "padding: 0px 5px 0px 5px;"
        "}"
        "* {font-size: 14px;}"
        ;

namespace Common {
    static QString getDirFromPath(const QString& path){
        int pos = path.lastIndexOf('/');
        if(pos == -1){
            return "";
        }

        return path.left(pos);
    }

    static QColor LineNumberAreaBGColor = Qt::black;
    static QColor LineNumberAreaTextColor = QColor(192,250,174);
    static QColor LineHighlightBGColor = Qt::black;
    static QColor LineHighlightTextColor = QColor(192,250,174);

#ifdef LC_DEPLOYMENT
    static const char* RESOURCE_PATH = "resources";
#else
    static const char* RESOURCE_PATH = "D:/Repositories/LogParser/resources";
#endif
}

#endif // COMMON_H
