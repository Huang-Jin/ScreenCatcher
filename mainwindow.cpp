#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "iconhelper.h"

#include <QDebug>
#include <QMouseEvent>
#include <QRgb>
#include <QDesktopWidget>
#include <QScreen>
#include <QTimer>
#include <QPainter>
#include <QClipboard>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Screen Catcher");
    setWindowIcon(QIcon(":/images/wblue.ico"));
    setWindowFlags(Qt::Tool|Qt::CustomizeWindowHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_QuitOnClose,true);

    setWindowOpacity(0.9);

    ui->horizontalLayoutWidget->setStyleSheet("#horizontalLayoutWidget{background-color:#dfdfdf}");
    ui->horizontalLayoutWidget_2->setStyleSheet("#horizontalLayoutWidget_2{border-image:url(:/images/bg_main.jpg)}");
    ui->btn_closeWindow->setStyleSheet("background-color:#AFAFAF;");
    ui->statusBar->setStyleSheet("background-color:#dfdfdf");

    IconHelper::Instance()->SetIcon(ui->btn_closeWindow, QChar(0xf00d), 10);
    connect(ui->btn_closeWindow,SIGNAL(clicked(bool)),this,SLOT(close()));

    int winheight = height();
    int winwidth = width();

    setMaximumSize(winwidth,winheight);
    setMinimumSize(winwidth,winheight);

//    QRect rect = ui->label_titleName->geometry();
//    rect.setLeft(30);
//    ui->label_titleName->setGeometry(rect);

    ui->statusBar->showMessage("Nothing now...");

    ui->centralWidget->setMouseTracking(true);
    this->setMouseTracking(true);

    connect(ui->text_r,SIGNAL(textChanged(QString)),SLOT(DrawTemplateText()));
    connect(ui->text_g,SIGNAL(textChanged(QString)),SLOT(DrawTemplateText()));
    connect(ui->text_b,SIGNAL(textChanged(QString)),SLOT(DrawTemplateText()));

    QIntValidator* intvalidator = new QIntValidator(0,255,this);
    ui->text_r->setValidator(intvalidator);
    ui->text_g->setValidator(intvalidator);
    ui->text_b->setValidator(intvalidator);

    QTimer *moveTimer = new QTimer(this);
    connect(moveTimer,SIGNAL(timeout()),SLOT(MouseMove()));
    moveTimer->start(50);

    bRgbCatching = true;
    m_globalShortcut_rgb = new QxtGlobalShortcut(QKeySequence("Ctrl+P"),this);
    QObject::connect(m_globalShortcut_rgb,SIGNAL(activated()),this,SLOT(ActivateRGBCatcher()));

    m_rgbGraphRect = ui->rgbGraph->geometry();

    m_screenShotForm = new ScreenShotForm(0);
    m_screenShotForm->setShotting(false);
    m_screenShotForm->setMainWindow(this);

    m_globalShortcut_shotting = new QxtGlobalShortcut(QKeySequence("Ctrl+A"),this);
    QObject::connect(m_globalShortcut_shotting,SIGNAL(activated()),this,SLOT(ActivateScreenShot()));

    m_globalShortcut_close = new QxtGlobalShortcut(QKeySequence("Ctrl+W"),this);
    QObject::connect(m_globalShortcut_close,SIGNAL(activated()),this,SLOT(close()));

    m_globalShortcut_copy = new QxtGlobalShortcut(QKeySequence("Ctrl+Alt+C"),this);
    QObject::connect(m_globalShortcut_copy,SIGNAL(activated()),this,SLOT(copyRGBData()));

    QIcon icon = QIcon(":/images/wblue.ico");
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(icon);
    trayIcon->setToolTip(tr("ScreenCatcher"));
    trayIcon->show();

    m_quitAction = new QAction(tr("Quit (&Q)"), this);
    m_quitAction->setIcon(QIcon(":/images/close.png"));
    connect(m_quitAction, SIGNAL(triggered()), this, SLOT(close()));

    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(m_quitAction);
    trayIcon->setContextMenu(trayIconMenu);
}

MainWindow::~MainWindow()
{
    if(m_globalShortcut_rgb) {
        m_globalShortcut_rgb->deleteLater();
    }
    if(m_globalShortcut_shotting) {
        m_globalShortcut_shotting->deleteLater();
    }

    if(m_screenShotForm)
        m_screenShotForm->close();

    if(m_quitAction)
        delete m_quitAction;

    if(trayIconMenu)
        delete trayIconMenu;

    if(trayIcon)
        delete trayIcon;

    delete ui;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_bMousePressed = true;
        m_ptMouse = event->globalPos() - this->pos();
    }
}
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_bMousePressed = false;
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bMousePressed && (event->buttons() && Qt::LeftButton))
    {
        this->move(event->globalPos() - m_ptMouse);
    }
}

void MainWindow::DrawTemplateText()
{
    QString strTem;
    int r,g,b;

    strTem = ui->text_r->text();
    r = strTem.toInt();
    strTem = ui->text_g->text();
    g = strTem.toInt();
    strTem = ui->text_b->text();
    b = strTem.toInt();

    QColor color;

    if (r < 0 || r>255 || g < 0 || g>255 || b < 0 || b>255) {
        color.setRgb(0,0,0);
        strTem = "数据格式错误,数值应在0-255之间.";
        ui->statusBar->showMessage(strTem);
    }
    else {
        color.setRgb(r,g,b);
        QRgb rgb = qRgb(r,g,b);
        QString name;
        name =  QString("%1").arg(rgb,4,16,QLatin1Char('0'));
        ui->label_test->setText("#" + name.mid(2));
        ui->statusBar->showMessage("Color is Changing, Ctrl+P to pause...");
    }

//    qDebug() << color.colorNames().at(0);

    QPalette palette;
    palette.setColor(QPalette::WindowText,color);
    ui->label_test->setPalette(palette);
}

void MainWindow::MouseMove()
{
    if(!bRgbCatching)
    {
        return;
    }

    QRect wRect = this->frameGeometry();
    QScreen *screen = QGuiApplication::primaryScreen();
    QPoint pt = cursor().pos();

    if(wRect.contains(pt))
    {
        return;
    }

    QImage image = screen->grabWindow(QApplication::desktop()->winId()).toImage();

    QRgb rgb = image.pixel(pt);
    ui->text_r->setText(QString::number(qRed(rgb)));
    ui->text_g->setText(QString::number(qGreen(rgb)));
    ui->text_b->setText(QString::number(qBlue(rgb)));

    int crosssize = 3;
    int scale = 3;
    QPainter painter(&image);
    painter.setPen(QColor(255,0,0));
    painter.drawLine(pt.x()-crosssize,pt.y(),pt.x()+crosssize,pt.y());
    painter.drawLine(pt.x(),pt.y()-crosssize,pt.x(),pt.y()+crosssize);
    QImage rgbImage = image.copy(pt.x()-m_rgbGraphRect.width()/(scale*2),pt.y()-m_rgbGraphRect.height()/(scale*2),
                                 m_rgbGraphRect.width()/scale,m_rgbGraphRect.height()/scale);
    rgbImage = rgbImage.scaled(m_rgbGraphRect.width(),m_rgbGraphRect.height());
    ui->rgbGraph->setPixmap(QPixmap::fromImage(rgbImage));
}

void MainWindow::ActivateRGBCatcher()
{
    bRgbCatching = !bRgbCatching;
    if(!bRgbCatching)
        ui->statusBar->showMessage("Paused, Ctrl+P to continue...");
    else
        ui->statusBar->showMessage("Continued, Ctrl+P to pause...");
}

void MainWindow::ActivateScreenShot()
{
    this->hide();
    update();
    m_screenShotForm->setShotting(true);
    m_screenShotForm->beginShotting();
    m_screenShotForm->show();
}

void MainWindow::copyRGBData()
{
    QClipboard *board = QApplication::clipboard();

    QString text =  QString("R:%1 G:%2 B:%3 HTML:%4")
            .arg(ui->text_r->text()).arg(ui->text_g->text()).arg(ui->text_b->text())
                 .arg(ui->label_test->text());

    board->setText(text);
    ui->statusBar->showMessage("Message copy is completed.");
}

void MainWindow::exitProcess()
{
    exit(0);
}
