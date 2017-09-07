#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QSystemTrayIcon>
#include "qxtglobalshortcut.h"
#include "screenshotform.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);


protected slots:
    void DrawTemplateText();
    void MouseMove();
    void ActivateRGBCatcher();
    void ActivateScreenShot();
    void copyRGBData();
    void exitProcess();

private:
    Ui::MainWindow *ui;

    QxtGlobalShortcut *m_globalShortcut_rgb;
    QxtGlobalShortcut *m_globalShortcut_shotting;
    QxtGlobalShortcut *m_globalShortcut_close;
    QxtGlobalShortcut *m_globalShortcut_copy;

    bool m_bMousePressed;
    QPoint m_ptMouse;

    bool bRgbCatching;
    QRect m_rgbGraphRect;

    ScreenShotForm *m_screenShotForm;
    QSystemTrayIcon *trayIcon;
    QMenu   *trayIconMenu;
    QAction *m_quitAction;
};

#endif // MAINWINDOW_H
