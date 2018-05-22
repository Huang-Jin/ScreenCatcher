#include "screenshotform.h"
#include "ui_screenshotform.h"

#include <QDesktopWidget>
#include <QScreen>
#include <QGuiApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QLabel>
#include <QClipboard>
#include <QFileDialog>
#include <QCursor>
#include <QMessageBox>

ScreenShotForm::ScreenShotForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScreenShotForm)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint|Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    QScreen *screen = QGuiApplication::primaryScreen();
    setMaximumSize(screen->size());
    setMinimumSize(screen->size());
    setMouseTracking(true);

    m_arrowCursor = QCursor(QPixmap(":/images/arrow.cur"),5,2);
    this->setCursor(m_arrowCursor);

    QRect rect = ui->horizontalLayoutWidget->geometry();
    m_popMenuDefautRect.setBottom(0);
    m_popMenuDefautRect.setRight(0);
    m_popMenuDefautRect.setLeft(-rect.width());
    m_popMenuDefautRect.setTop(-rect.height());

    ui->horizontalLayoutWidget->setGeometry(m_popMenuDefautRect);

    rect = ui->label_size->geometry();
    m_labelDefautRect.setBottom(0);
    m_labelDefautRect.setRight(0);
    m_labelDefautRect.setLeft(-rect.width());
    m_labelDefautRect.setTop(-rect.height());

    ui->label_size->setGeometry(m_labelDefautRect);

    rect = ui->screenscaled->geometry();
    m_scaledscreenDefautRect.setBottom(0);
    m_scaledscreenDefautRect.setRight(0);
    m_scaledscreenDefautRect.setLeft(-rect.width());
    m_scaledscreenDefautRect.setTop(-rect.height());

    ui->horizontalLayoutWidget->setStyleSheet("background-color:#969696;");

    connect(ui->btn_closeRect,SIGNAL(clicked(bool)),this,SLOT(hide()));
    connect(ui->btn_saveRect,SIGNAL(clicked(bool)),this,SLOT(savePresetRect()));
    connect(ui->btn_saveclip,SIGNAL(clicked(bool)),this,SLOT(saveIntoclip()));
    connect(ui->btn_savefile,SIGNAL(clicked(bool)),this,SLOT(saveIntofile()));

    m_bScreenShot = false;
    m_bDrawRect = false;
    m_bPopMenu = false;
    m_bMoving = false;
    m_bScaling = false;
    m_bPresetRect = false;

    m_scalePt.setX(-1);
    m_scalePt.setY(-1);
    ui->screenscaled->setGeometry(m_scaledscreenDefautRect);
}

ScreenShotForm::~ScreenShotForm()
{
    delete ui;
}

void ScreenShotForm::paintEvent(QPaintEvent *event)
{
    if(!m_bScreenShot)
    {
        return;
    }

    QPainter painter(this);
    painter.drawImage(0,0,m_screenImage);

    if(m_bDrawRect)
    {
        QRect rect;

        int x1 = m_bPt.x();
        int y1 = m_bPt.y();
        int x2 = m_ePt.x();
        int y2 = m_ePt.y();

        if(x1 < x2)
        {
            rect.setLeft(x1);
            rect.setRight(x2);
        }
        else
        {
            rect.setLeft(x2);
            rect.setRight(x1);
        }

        if(y1 < y2)
        {
            rect.setTop(y1);
            rect.setBottom(y2);
        }
        else
        {
            rect.setTop(y2);
            rect.setBottom(y1);
        }

        m_rect = rect;

        painter.setBrush(QBrush(QColor(128,128,128,128)));
        painter.setPen(Qt::NoPen);

        QRect rectImage = m_screenImage.rect();

        QRect r1(QPoint(rectImage.left(),rectImage.top()),QPoint(rectImage.right(),m_rect.top()));
        QRect r2(QPoint(rectImage.left(),m_rect.top()+1),QPoint(m_rect.left(),m_rect.bottom()));
        QRect r3(QPoint(m_rect.right(),m_rect.top()+1),QPoint(rectImage.right(),m_rect.bottom()));
        QRect r4(QPoint(rectImage.left(),m_rect.bottom()+1),QPoint(rectImage.right(),rectImage.bottom()));

        painter.drawRect(r1);
        painter.drawRect(r2);
        painter.drawRect(r3);
        painter.drawRect(r4);

        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor(255,0,0),1,Qt::DotLine));
        painter.drawRect(rect);

        showSizeLabel(m_rect.left(),m_rect.top());
    }

    if(m_bMoving || m_bScaling || m_bPopMenu)
    {
        painter.setBrush(QBrush(QColor(128,128,128,128)));
        painter.setPen(Qt::NoPen);

        QRect rect = m_screenImage.rect();

        QRect r1(QPoint(rect.left(),rect.top()),QPoint(rect.right(),m_rect.top()));
        QRect r2(QPoint(rect.left(),m_rect.top()+1),QPoint(m_rect.left(),m_rect.bottom()));
        QRect r3(QPoint(m_rect.right(),m_rect.top()+1),QPoint(rect.right(),m_rect.bottom()));
        QRect r4(QPoint(rect.left(),m_rect.bottom()+1),QPoint(rect.right(),rect.bottom()));

        painter.drawRect(r1);
        painter.drawRect(r2);
        painter.drawRect(r3);
        painter.drawRect(r4);

        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor(255,0,0),1,Qt::DotLine));
        painter.drawRect(m_rect);
        showSizeLabel(m_rect.left(),m_rect.top());
        showPopMenu(m_rect.right(),m_rect.bottom());
        showScaleCorner(m_rect,&painter);
    }

    if(m_scalePt.x() != -1)
    {
        showScaledPoint(m_scalePt);
    }

    painter.end();
}

void ScreenShotForm::hideEvent(QHideEvent *event)
{
    m_bScreenShot = false;
    m_bDrawRect = false;
    m_bPopMenu = false;
    m_bMoving = false;
    m_bScaling = false;

    m_scalePt.setX(-1);
    m_scalePt.setY(-1);
    ui->screenscaled->setGeometry(m_scaledscreenDefautRect);

    ui->horizontalLayoutWidget->setGeometry(m_popMenuDefautRect);
    ui->label_size->setGeometry(m_labelDefautRect);
    setCursor(m_arrowCursor);
    m_parent->show();
}

void ScreenShotForm::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape){
        hide();
    }
}

void ScreenShotForm::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(!m_bScreenShot)
            return;

        m_bPt = event->pos();

        if(m_bPopMenu)
        {
            QPoint pt = cursor().pos();
            pt = pt - this->geometry().topLeft();

            // TopLeft
            if(abs(pt.x() - m_rect.left()) <= m_tolerance
                    && abs(pt.y() - m_rect.top()) <= m_tolerance)
            {
                setCursor(Qt::SizeFDiagCursor);
                m_oldPt = m_rect.topLeft();
                m_curCornerType = LEFTTOP;
                m_bScaling = true;
            }
            // Top
            else if(abs(pt.x() - m_rect.center().x()) <= m_tolerance
                    && abs(pt.y() - m_rect.top()) <= m_tolerance)
            {
                setCursor(Qt::SizeVerCursor);
                m_oldPt = m_rect.topLeft();
                m_oldPt2 = m_rect.topRight();
                m_curCornerType = TOP;
                m_bScaling = true;
            }
            // TopRight
            else if(abs(pt.x() - m_rect.right()) <= m_tolerance
                    && abs(pt.y() - m_rect.top()) <= m_tolerance)
            {
                setCursor(Qt::SizeBDiagCursor);
                m_oldPt = m_rect.topRight();
                m_curCornerType = RIGHTTOP;
                m_bScaling = true;
            }
            // Right
            else if(abs(pt.x() - m_rect.right()) <= m_tolerance
                    && abs(pt.y() - m_rect.center().y()) <= m_tolerance)
            {
                setCursor(Qt::SizeHorCursor);
                m_oldPt = m_rect.topRight();
                m_oldPt2 = m_rect.bottomRight();
                m_curCornerType = RIGHT;
                m_bScaling = true;
            }
            // RightBottom
            else if(abs(pt.x() - m_rect.right()) <= m_tolerance
                    && abs(pt.y() - m_rect.bottom()) <= m_tolerance)
            {
                setCursor(Qt::SizeFDiagCursor);
                m_oldPt = m_rect.bottomRight();
                m_curCornerType = RIGHTBOTTOM;
                m_bScaling = true;
            }
            // Bottom
            else if(abs(pt.x() - m_rect.center().x()) <= m_tolerance
                    && abs(pt.y() - m_rect.bottom()) <= m_tolerance)
            {
                setCursor(Qt::SizeVerCursor);
                m_oldPt = m_rect.bottomRight();
                m_oldPt2 = m_rect.bottomLeft();
                m_curCornerType = BOTTOM;
                m_bScaling = true;
            }
            // LeftBottom
            else if(abs(pt.x() - m_rect.left()) <= m_tolerance
                    && abs(pt.y() - m_rect.bottom()) <= m_tolerance)
            {
                setCursor(Qt::SizeBDiagCursor);
                m_oldPt = m_rect.bottomLeft();
                m_curCornerType = LEFTBOTTOM;
                m_bScaling = true;
            }
            // Left
            else if(abs(pt.x() - m_rect.left()) <= m_tolerance
                    && abs(pt.y() - m_rect.center().y()) <= m_tolerance)
            {
                setCursor(Qt::SizeHorCursor);
                m_oldPt = m_rect.bottomLeft();
                m_oldPt2 = m_rect.topLeft();
                m_curCornerType = LEFT;
                m_bScaling = true;
            }

            // Center
            else if(m_rect.contains(pt))
            {
                setCursor(Qt::SizeAllCursor);
                m_bPt = event->pos();
                m_oldPt = m_rect.center();
                m_bMoving = true;
            }
            else
            {
                resetScreenShot();
                m_bDrawRect = true;
                m_bPt = event->pos();
                m_ePt = event->pos();
            }
            return;
        }
        if(!m_bDrawRect)
        {
            m_bDrawRect = true;
            m_bPt = event->pos();
            m_ePt = event->pos();
        }
    }
    else if(event->button() == Qt::RightButton)
    {
        if(m_bPopMenu)
        {
            resetScreenShot();
        }
        else
        {
            hide();
        }
    }
}

void ScreenShotForm::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_scalePt.setX(-1);
        m_scalePt.setY(-1);
        ui->screenscaled->setGeometry(m_scaledscreenDefautRect);

        if(m_bPopMenu)
        {
            if(m_bScaling)
            {
                m_bScaling = false;
                setCursor(m_arrowCursor);
            }

            if(m_bMoving)
            {
                m_bMoving = false;
                setCursor(m_arrowCursor);
            }
            update();
        }

        if(m_bDrawRect)
        {
            m_bDrawRect = false;
            m_ePt = event->pos();

            m_bPopMenu = true;            
            ui->screenscaled->setGeometry(m_scaledscreenDefautRect);
            showPopMenu(m_ePt.x(),m_ePt.y());
        }
    }

}

void ScreenShotForm::mouseMoveEvent(QMouseEvent *event)
{
    if(m_bScaling)
    {
        QPoint pt1;
        pt1 = m_oldPt+m_ePt-m_bPt;
        m_ePt = event->pos();
        int temp;

        switch(m_curCornerType)
        {
        case LEFTTOP:
            m_rect.setTopLeft(pt1);
            m_scalePt = pt1;
            break;
        case TOP:
            temp = m_oldPt.y()+m_ePt.y()-m_bPt.y();
            m_rect.setTopLeft(QPoint(m_oldPt.x(),temp));
            m_rect.setTopRight(QPoint(m_oldPt2.x(),temp));
            m_scalePt = QPoint(m_ePt.x(),temp);
            break;
        case RIGHTTOP:
            m_rect.setTopRight(pt1);
            m_scalePt = pt1;
            break;
        case RIGHT:
            temp = m_oldPt.x()+m_ePt.x()-m_bPt.x();
            m_rect.setTopRight(QPoint(temp,m_oldPt.y()));
            m_rect.setBottomRight(QPoint(temp,m_oldPt2.y()));
            m_scalePt = QPoint(temp,m_ePt.y());
            break;
        case RIGHTBOTTOM:
            m_rect.setBottomRight(pt1);
            m_scalePt = pt1;
            break;
        case BOTTOM:
            temp = m_oldPt.y()+m_ePt.y()-m_bPt.y();
            m_rect.setBottomRight(QPoint(m_oldPt.x(),temp));
            m_rect.setBottomLeft(QPoint(m_oldPt2.x(),temp));
            m_scalePt = QPoint(m_ePt.x(),temp);
            break;
        case LEFTBOTTOM:
            m_rect.setBottomLeft(pt1);
            m_scalePt = pt1;
            break;
        case LEFT:
            temp = m_oldPt.x()+m_ePt.x()-m_bPt.x();
            m_rect.setBottomLeft(QPoint(temp,m_oldPt.y()));
            m_rect.setTopLeft(QPoint(temp,m_oldPt2.y()));
            m_scalePt = QPoint(temp,m_ePt.y());
            break;
        }
        update();
    }

    if(m_bMoving)
    {
        m_ePt = event->pos();
        QRect oldrect = m_rect;
        m_rect.moveCenter(m_oldPt+m_ePt-m_bPt);

        if(!m_screenImage.rect().contains(m_rect))
        {
            m_rect = oldrect;
        }

        update();
    }

    if(m_bScreenShot && m_bDrawRect)
    {
        m_ePt = event->pos();
        m_scalePt = m_ePt;
        update();
    }

    // change style of cursor
    if(m_bScreenShot && m_bPopMenu && !m_bMoving && !m_bScaling)
    {
        QPoint pt = cursor().pos();
        pt = pt - this->geometry().topLeft();

        // TopLeft
        if(abs(pt.x() - m_rect.left()) <= m_tolerance
                && abs(pt.y() - m_rect.top()) <= m_tolerance)
        {
            setCursor(Qt::SizeFDiagCursor);
        }
        // Top
        else if(abs(pt.x() - m_rect.center().x()) <= m_tolerance
                && abs(pt.y() - m_rect.top()) <= m_tolerance)
        {
            setCursor(Qt::SizeVerCursor);
        }
        // TopRight
        else if(abs(pt.x() - m_rect.right()) <= m_tolerance
                && abs(pt.y() - m_rect.top()) <= m_tolerance)
        {
            setCursor(Qt::SizeBDiagCursor);
        }
        // Right
        else if(abs(pt.x() - m_rect.right()) <= m_tolerance
                && abs(pt.y() - m_rect.center().y()) <= m_tolerance)
        {
            setCursor(Qt::SizeHorCursor);
        }
        // RightBottom
        else if(abs(pt.x() - m_rect.right()) <= m_tolerance
                && abs(pt.y() - m_rect.bottom()) <= m_tolerance)
        {
            setCursor(Qt::SizeFDiagCursor);
        }
        // Bottom
        else if(abs(pt.x() - m_rect.center().x()) <= m_tolerance
                && abs(pt.y() - m_rect.bottom()) <= m_tolerance)
        {
            setCursor(Qt::SizeVerCursor);
        }
        // LeftBottom
        else if(abs(pt.x() - m_rect.left()) <= m_tolerance
                && abs(pt.y() - m_rect.bottom()) <= m_tolerance)
        {
            setCursor(Qt::SizeBDiagCursor);
        }
        // Left
        else if(abs(pt.x() - m_rect.left()) <= m_tolerance
                && abs(pt.y() - m_rect.center().y()) <= m_tolerance)
        {
            setCursor(Qt::SizeHorCursor);
        }

        // Center
        else if(m_rect.contains(pt))
        {
            setCursor(Qt::SizeAllCursor);
        }
        else
        {
            setCursor(m_arrowCursor);
        }
    }
}

void ScreenShotForm::showScaleCorner(QRect rect, QPainter* painter)
{
    int width = 5;

    (*painter).setPen(QPen(QColor(0,0,255),3));

    // TopLeft
    (*painter).drawLine(rect.topLeft(),rect.topLeft()+QPoint(width,0));
    (*painter).drawLine(rect.topLeft(),rect.topLeft()+QPoint(0,width));

    // Top
    (*painter).drawLine(QPoint(rect.center().x(),rect.top())-QPoint(width,0),
                        QPoint(rect.center().x(),rect.top())+QPoint(width,0));

    // Bottom
    (*painter).drawLine(QPoint(rect.center().x(),rect.bottom())-QPoint(width,0),
                        QPoint(rect.center().x(),rect.bottom())+QPoint(width,0));

    // TopRight
    (*painter).drawLine(rect.topRight(),rect.topRight()-QPoint(width,0));
    (*painter).drawLine(rect.topRight(),rect.topRight()+QPoint(0,width));

    // Left
    (*painter).drawLine(QPoint(rect.left(),rect.center().y())-QPoint(0,width),
                        QPoint(rect.left(),rect.center().y())+QPoint(0,width));

    // Right
    (*painter).drawLine(QPoint(rect.right(),rect.center().y())-QPoint(0,width),
                        QPoint(rect.right(),rect.center().y())+QPoint(0,width));

    // BottomRight
    (*painter).drawLine(rect.bottomRight(),rect.bottomRight()-QPoint(width,0));
    (*painter).drawLine(rect.bottomRight(),rect.bottomRight()-QPoint(0,width));

    // BottomLeft
    (*painter).drawLine(rect.bottomLeft(),rect.bottomLeft()+QPoint(width,0));
    (*painter).drawLine(rect.bottomLeft(),rect.bottomLeft()-QPoint(0,width));
}

void ScreenShotForm::showScaledPoint(QPoint pt)
{
    QImage image = m_screenImage.copy(pt.x()-m_size/(m_scale*2),pt.y()-m_size/(m_scale*2),
                                    m_size/m_scale,m_size/m_scale);
    image = image.scaled(m_size,m_size);

    QRect rectImage = m_screenImage.rect();

    QPoint ptLeftTop;
    ptLeftTop = pt + QPoint(m_space,m_space);

    if(ptLeftTop.x() + m_size > rectImage.right())
    {
        ptLeftTop.setX(pt.x()-m_size -m_space);
        if(ptLeftTop.y() + m_size > rectImage.bottom())
        {
            ptLeftTop.setY(pt.y()-m_size -m_space);
        }
    }
    else if(ptLeftTop.y() + m_size > rectImage.bottom())
    {
        ptLeftTop.setY(pt.y()-m_size -m_space);
    }

    QRect rect;
    rect.setTopLeft(ptLeftTop);
    rect.setRight(ptLeftTop.x()+m_size);
    rect.setBottom(ptLeftTop.y()+m_size);

    QPainter painter(&image);
    painter.setPen(QPen(QColor(0,0,255),2));
    painter.drawRect(0,0,m_size,m_size);
    painter.setPen(QPen(QColor(255,0,0),1));
    painter.drawLine(0,m_size/2+m_scale,m_size,m_size/2+m_scale);
    painter.drawLine(m_size/2+m_scale,0,m_size/2+m_scale,m_size);

    ui->screenscaled->setPixmap(QPixmap::fromImage(image));
    ui->screenscaled->setGeometry(rect);
}

void ScreenShotForm::showPopMenu(int x,int y)
{
    int m_space = 5;

    QRect rectImage = m_screenImage.rect();
    QPoint pt(x,y+m_space);
    int width = m_popMenuDefautRect.width();
    int height = m_popMenuDefautRect.height();

    if(pt.y() + height > rectImage.bottom())
    {
        pt.setY(pt.y() - height - 2*m_space);
        pt.setX(pt.x() - m_space);
    }

    QRect rect;
    rect.setTopRight(pt);
    rect.setLeft(pt.x()-width);
    rect.setBottom(pt.y()+height);

    ui->horizontalLayoutWidget->setGeometry(rect);

    update();
}

void ScreenShotForm::showSizeLabel(int x, int y)
{
    int m_space = 5;

    QRect rectImage = m_screenImage.rect();
    QPoint pt(x,y-m_space);

    int width = m_labelDefautRect.width();
    int height = m_labelDefautRect.height();

    if(pt.y() - height < rectImage.top())
    {
        pt.setY(pt.y() + height + 2*m_space);
        pt.setX(pt.x() + m_space);
    }

    QRect rect;
    rect.setBottomLeft(pt);
    rect.setRight(pt.x()+width);
    rect.setTop(pt.y()-height);

    ui->label_size->setGeometry(rect);

    QString sizetext = QString("%1x%2").arg(m_rect.width()).arg(m_rect.height());
    ui->label_size->setText(sizetext);
}

void ScreenShotForm::saveIntofile()
{
    QImage image = m_screenImage.copy(m_rect);
    QString fileName = QString("screenshot.png");
#ifdef Q_OS_WIN
    fileName = QFileDialog::getSaveFileName(this, tr("Save file"), "./screenshot.png",  tr("Allfiles(*.*);;Png(*.png);;"));

    if(fileName.isEmpty())
    {
        return;
    }
#endif

    image.save(fileName);
    hide();
}

void ScreenShotForm::saveIntoclip()
{
    QClipboard *board = QApplication::clipboard();
    QImage image = m_screenImage.copy(m_rect);
    board->setImage(image);
    board = NULL;
    hide();
}

void ScreenShotForm::resetScreenShot()
{
    m_bDrawRect = false;
    m_bPopMenu = false;
    m_bMoving = false;
    m_bScaling = false;
    m_bPresetRect = false;

    m_scalePt.setX(-1);
    m_scalePt.setY(-1);

    ui->horizontalLayoutWidget->setGeometry(m_popMenuDefautRect);
    ui->label_size->setGeometry(m_labelDefautRect);
    ui->screenscaled->setGeometry(m_scaledscreenDefautRect);

    setCursor(m_arrowCursor);
    update();
}

void ScreenShotForm::savePresetRect()
{
    if(!m_bPopMenu)
        return;

    m_bPresetRect = true;
    m_presetRect = m_rect;

    QMessageBox::information(this,"提示","截图区域已保存！",QMessageBox::Ok);
}

void ScreenShotForm::beginShotting()
{
    m_bScreenShot = true;
    QScreen *screen = QGuiApplication::primaryScreen();
    m_screenImage = screen->grabWindow(QApplication::desktop()->winId()).toImage();

    if(m_bPresetRect)
    {
        m_rect = m_presetRect;
        m_bPopMenu = true;
        update();
    }
}
