 #include "mainwindow.h"
#include "logindialog.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 启动时统一加载全局样式表，让所有页面共享同一套工业风设计语言。
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        const QString styleSheet = QLatin1String(styleFile.readAll());
        qApp->setStyleSheet(styleSheet);
        styleFile.close();
    }

    LoginDialog loginDig;
    if (loginDig.exec() == LoginDialog::Accepted) {
        MainWindow window;
        window.show();
        return app.exec();
    }
    return 0;

}
