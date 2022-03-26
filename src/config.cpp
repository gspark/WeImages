//
// Created by Spark on 2021-04-20.
//

#include "config.h"

#include <QSettings>

ConfigIni::ConfigIni() : pathFile("cryptool.ini"), settings(new QSettings(pathFile, QSettings::IniFormat)) {
//    Qt5
//    settings->setIniCodec("UTF-8");
}

ConfigIni::~ConfigIni() {
    if (nullptr != settings) {
        // 销毁前同步
        settings->sync();
        settings->deleteLater();
        settings = nullptr;
    }
}

ConfigIni &ConfigIni::getInstance() {
//    1. 变量在代码第一次执行到变量声明的地方时初始化。
//    2. 初始化过程中发生异常的话视为未完成初始化，未完成初始化的话，需要下次有代码执行到相同位置时再次初始化。
//    3. 在当前线程执行到需要初始化变量的地方时，如果有其他线程正在初始化该变量，则阻塞当前线程，直到初始化完成为止。
//    4. 如果初始化过程中发生了对初始化的递归调用，则视为未定义行为。

    static ConfigIni instance;
    return instance;
}

void ConfigIni::setPathFile(const QString &path_file) {
    if (path_file != this->pathFile) {
        this->pathFile = path_file;
        // 及时同步之前的
        settings->sync();
        // 改变路径
        QSettings::setPath(settings->format(), settings->scope(), path_file);
//        Qt5
//        settings->setIniCodec("UTF-8");
    }
}

void ConfigIni::iniWrite(const QString &key, const QVariant &value) {
    settings->setValue(key, value);
}

bool ConfigIni::iniContains(const QString &key) {
    return settings->contains(key);
}

QVariant ConfigIni::iniRead(const QString &key, const QVariant &defaultValue) {
    if (!settings->contains(key)) {
        // 不存在
        // 使用默认值创建
        settings->setValue(key, defaultValue);
        return defaultValue;
    }
    return settings->value(key);
}


