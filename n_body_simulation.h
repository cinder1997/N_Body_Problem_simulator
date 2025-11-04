#ifndef N_BODY_SIMULATION_H
#define N_BODY_SIMULATION_H

#include <QVector>
#include <QTimer>
#include <QFile>
#include <QApplication>
#include <QDir>
#include "body.h"
#include "openglwindow.h"

class N_body_Simulation : public QObject
{
Q_OBJECT  // 必须包含Q_OBJECT宏
public:
    N_body_Simulation(openglWindow *GL_window);
    ~N_body_Simulation();

    void startSimulation(double G,double RKF45Tol); //开始N体问题仿真
    void Pause();
    void Continue();
    void setBody_data(QVector<body*> *Body_data);   //获取天体数据
    void setLabel(QLabel *time_axis_Label);         //刷新当前时间和坐标轴长度

    bool isCollision=false;       //碰撞发生标志位
    QString CollisionMessage="";  //碰撞信息

private slots:
    void TimerProcess();       //定时器中断函数，进行单步计算并在openGL窗口中重新绘制

signals:  // 自定义信号声明区域
    void bodyCollision(int i,int j);  //两天体发生碰撞信号

private:
    QVector<body*> *Body_data; // 天体数据指针
    double G;                  // 万有引力常数
    double RKF45Tol;           // RKF45算法容差
    double timeStep;           // 时长步长
    static constexpr double timeStep0=1e5; // 初始时间步长
    double CurrentTime=0;      // 当前总时长

    openglWindow *GL_window;
    QLabel *time_axis_Label;
    QTimer *timer= new QTimer(this);
    static constexpr int TimeInterval=100; //定时器间隔(ms)，调整它可以改变帧率

    QString filename;
    QFile file;
    QTextStream out;

    // RKF45系数
    static constexpr double a2 = 1.0/4.0, a3 = 3.0/8.0, a4 = 12.0/13.0, a5 = 1.0, a6 = 1.0/2.0;
    static constexpr double b21 = 1.0/4.0;
    static constexpr double b31 = 3.0/32.0, b32 = 9.0/32.0;
    static constexpr double b41 = 1932.0/2197.0, b42 = -7200.0/2197.0, b43 = 7296.0/2197.0;
    static constexpr double b51 = 439.0/216.0, b52 = -8.0, b53 = 3680.0/513.0, b54 = -845.0/4104.0;
    static constexpr double b61 = -8.0/27.0, b62 = 2.0, b63 = -3544.0/2565.0, b64 = 1859.0/4104.0, b65 = -11.0/40.0;
    static constexpr double c1 = 16.0/135.0, c3 = 6656.0/12825.0, c4 = 28561.0/56430.0, c5 = -9.0/50.0, c6 = 2.0/55.0;
    static constexpr double d1 = 25.0/216.0, d3 = 1408.0/2565.0, d4 = 2197.0/4104.0, d5 = -1.0/5.0;

    void rungeKuttaFehlberg45(); // 变步长龙格-库塔法(RKF45)
    QVector<QVector3D> dydt(QVector<double>& bodies_mass,QVector<QVector3D>& State); //计算各天体位置和速度的增量(斜率)，同时检测碰撞
    QVector<QVector3D> Q3D_add(QVector<QVector3D> a,QVector<QVector3D> b);
    QVector<QVector3D> Q3D_multiply(QVector<QVector3D> a,double num);


    // void rungeKuttaStep();       // 定步长龙格-库塔法(RK4)，已不再使用！
    // QVector<QVector3D> computeAccelerations(const QVector<double> &mass,const QVector<QVector3D> &pos,const QVector<QVector3D> &vel) const;
    // QVector3D computeGravity(const double &mass1,const QVector3D &pos1,const QVector3D &vel1, const double &mass2,const QVector3D &pos2) const;
};

#endif // N_BODY_SIMULATION_H
