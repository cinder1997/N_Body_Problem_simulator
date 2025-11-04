#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QDesktopServices>
#include "openglwindow.h"
#include "n_body_simulation.h"
#include "body.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWidget;
}
QT_END_NAMESPACE

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();
    void addBody(double mass, QVector3D pos, QVector3D vel, QVector3D color,float radius);

private slots:
    void startSim();
    void pause_continue();
    void openDataFile();
    void handle_bodyCollision(int i,int j);

private:
    Ui::MainWidget *ui;
    QHBoxLayout *Mainlayout;       // 主界面布局
    QVBoxLayout *scrollAreaLayout; // 滑动窗口
    QWidget *RightWidget;
    QGridLayout *gridLayout;
    openglWindow *GL_window;       // openGL窗口
    QLabel *time_axis_Label;       // 时间、坐标轴长度显示

    QVector<body *> Body_data;     // 储存所有天体数据和窗口
    double G = 6.67430e-11;        // 万有引力常数
    //double timeStep=3e-2;        // 时长步长
    double RKF45Tol=1e-6;          // RKF45算法容差

    N_body_Simulation *Simulator;  // 核心计算程序
    QRandomGenerator *RandomNum = QRandomGenerator::global(); //随机数生成器

    bool isStart=false;            // 表示仿真是否开始
    bool isPause=false;            // 表示仿真是否暂停
};
#endif // MAINWIDGET_H
