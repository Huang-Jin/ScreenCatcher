#ifndef SCREENSHOTFORM_H
#define SCREENSHOTFORM_H

#include <QWidget>

namespace Ui {
class ScreenShotForm;
}

enum CornerType{
    LEFTTOP=0,
    TOP,
    RIGHTTOP,
    RIGHT,
    RIGHTBOTTOM,
    BOTTOM,
    LEFTBOTTOM,
    LEFT
};

class ScreenShotForm : public QWidget
{
    Q_OBJECT

public:
    explicit ScreenShotForm(QWidget *parent = 0);
    ~ScreenShotForm();

protected slots:
    void saveIntofile();
    void saveIntoclip();
    void resetScreenShot();

protected:
    void paintEvent(QPaintEvent *event);
    void hideEvent(QHideEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void showScaleCorner(QRect rect,QPainter* painter);
    void showScaledPoint();
    void showPopMenu(int x,int y);
    void showSizeLabel(int x, int y);

private:
    Ui::ScreenShotForm *ui;
    QWidget* m_parent;

    bool m_bScreenShot;
    bool m_bDrawRect;
    bool m_bPopMenu;
    bool m_bMoving;
    bool m_bScaling;

    QImage screenImage;
    QPoint m_bPt;
    QPoint m_ePt;
    QPoint m_oldPt;
    QPoint m_oldPt2;
    QRect m_rect;

    CornerType m_curCornerType;

    QRect m_popMenuDefautRect;
    QRect m_labelDefautRect;
    QRect m_scaledscreenDefautRect;

    QCursor m_arrowCursor;
public:
    bool IsShotting() {return m_bScreenShot;}
    void setShotting(bool b) {m_bScreenShot = b;}
    void setMainWindow(QWidget* parent) {m_parent = parent;}

    void beginShotting();
};

#endif // SCREENSHOTFORM_H
