#ifndef COMMON_H
#define COMMON_H

#include <QString>

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
}

#endif // COMMON_H
