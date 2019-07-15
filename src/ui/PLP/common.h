/*
 * This file is part of the Line Catcher distribution (https://github.com/AlexandrSachkov/LineCatcher).
 * Copyright (c) 2019 Alexandr Sachkov.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QColor>

#define LC_DEPLOYMENT

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
