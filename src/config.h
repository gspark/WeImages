//
// Created by shrill on 2021-04-20.
//

#ifndef CRYPTOOL_CONFIG_H
#define CRYPTOOL_CONFIG_H

#include <QtGlobal>
#include <QString>
#include <QVariant>

//#define CONFIG_GROUP_MAIN_WIN    "MainWidget"
//#define CONFIG_GROUP_FILE_DOCK   "FileDock"
//#define CONFIG_GROUP_NAV_DOCK    "NavDock"
//#define CONFIG_GROUP_WINDOW     "MainView/size"
//#define CONFIG_WIN_GEOMETRY     "Geometry"
//#define CONFIG_WIN_STATE        "WindowState"
//#define CONFIG_DOCK_HIDE        "hide"
//#define CONFIG_TAB_INDEX        "index"
//#define ITEM_DEFAULT_HEIGHT     (30)

QT_BEGIN_NAMESPACE
class QSettings;

QT_END_NAMESPACE


class ConfigIni {
private:
    ConfigIni();

    ~ConfigIni();

    ConfigIni(const ConfigIni &);

    ConfigIni &operator=(const ConfigIni &);

    // ini文件路径
    QString pathFile;
    QSettings *settings;

public:
    static ConfigIni &getInstance();

    // 重设config.ini文件路径
    void setPathFile(const QString &path_file);

    // 检测配置存在
    bool iniContains(const QString &key);

    QVariant iniRead(const QString &key, const QVariant &defaultValue = QVariant());

    // 写配置文件
    void iniWrite(const QString &key, const QVariant &value);

};


#endif //CRYPTOOL_CONFIG_H
