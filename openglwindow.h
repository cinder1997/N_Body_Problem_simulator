#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QOpenGLWidget>
#include <qopenglfunctions_4_5_core.h>
#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QVector>
#include <QtMath>
#include "body.h"


class openglWindow: public QOpenGLWidget,QOpenGLFunctions_4_5_Core
{
public:
    openglWindow(QWidget *parent = nullptr);
    ~openglWindow();
    void setBody_data(QVector<body*> *Body_data);
    float getFarthestDis(); //获取天体中距离坐标轴最远距离
    float getFastestVel();  //获取天体中最快速度
    void paint_N_body();    //绘制当前天体位置
    void refreshAxis();     //重新计算并绑定坐标轴
    double getAxisLength();

    double timeStep=1;
protected:
    //实现QOpenGL虚函数
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();

private:
    QVector<body*> *Body_data=nullptr;  //天体数据

    QVector<QVector3D> Sphere_Vertices;  //球体顶点数据
    QVector<GLushort> Sphere_indices;    //球体索引数据
    void generateSphere();               //初始化球体数据
    QOpenGLShaderProgram Sphere_shaderProgram; //球体着色器程序
    QOpenGLVertexArrayObject Sphere_VAO;       //球体GPU缓冲对象
    QOpenGLBuffer Sphere_VBO;
    QOpenGLBuffer Sphere_EBO;

    QOpenGLShaderProgram Trace_shaderProgram; //轨迹线着色器程序
    QOpenGLVertexArrayObject Trace_VAO;       //轨迹线GPU缓冲对象
    QOpenGLBuffer Trace_VBO;
    QVector<QVector3D> TraceLine={
        QVector3D(0,0,0),
        QVector3D(0.5,0.5,0),
};

    QMatrix4x4 M_model;       //模型矩阵M，将模型坐标平移、旋转和缩放到世界坐标
    QMatrix4x4 M_view;        //透视矩阵V，表示摄像机的位置和朝向
    QMatrix4x4 M_projection;  //裁剪矩阵P，表示摄影机看到的范围

    float viewLength = 100.0f;    //可视区域长度
    QVector3D cameraPos=QVector3D{(float)1.6*viewLength,(float)1.6*viewLength,(float)1.6*viewLength};  //摄影机位置
    QVector3D cameraFront=QVector3D(-(float)1.6*viewLength,-(float)1.6*viewLength,-(float)1.6*viewLength);//摄影机的朝向、
    float Last_farthestDis=0;     //最远天体距离
    float Dis_factor=1.0f;        //缩放系数，用于绘制天体时会将其位置缩放到可视范围内

    float axisLength = viewLength;    //默认坐标轴长度
    float arrowSize = 0.05f*axisLength;
    QVector<float> axisData = {       //坐标轴数据
        // X轴 - 红色
        0.0f, 0.0f, 0.0f,       1.0f, 0.0f, 0.0f,  // 起点
        axisLength, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // 终点
        // Y轴 - 绿色
        0.0f, 0.0f, 0.0f,       0.0f, 1.0f, 0.0f,  // 起点
        0.0f, axisLength, 0.0f, 0.0f, 1.0f, 0.0f,  // 终点
        // Z轴 - 蓝色
        0.0f, 0.0f, 0.0f,       0.0f, 0.0f, 1.0f,  // 起点
        0.0f, 0.0f, axisLength, 0.0f, 0.0f, 1.0f,  // 终点
        // X轴箭头
        axisLength, 0.0f, 0.0f,                  1.0f, 0.0f, 0.0f,  // 箭头起点
        axisLength - arrowSize, 0.0f, arrowSize, 1.0f, 0.0f, 0.0f,  // 箭头端点1
        axisLength - arrowSize, 0.0f, -arrowSize,1.0f, 0.0f, 0.0f,  // 箭头端点2
        // Y轴箭头
        0.0f, axisLength, 0.0f,                  0.0f, 1.0f, 0.0f,  // 箭头起点
        0.0f, axisLength-arrowSize, arrowSize,   0.0f, 1.0f, 0.0f,  // 箭头端点1
        0.0f, axisLength-arrowSize, -arrowSize,  0.0f, 1.0f, 0.0f,  // 箭头端点2
        // Z轴箭头
        0.0f, 0.0f, axisLength,                  0.0f, 0.0f, 1.0f,  // 箭头起点
        arrowSize, 0.0f, axisLength-arrowSize,   0.0f, 0.0f, 1.0f,  // 箭头端点1
        -arrowSize, 0.0f, axisLength-arrowSize,  0.0f, 0.0f, 1.0f,  // 箭头端点2
    };
    QOpenGLShaderProgram Axis_shaderProgram;//坐标轴着色器程序
    QOpenGLVertexArrayObject Axis_VAO;      //坐标轴GPU缓冲对象
    QOpenGLBuffer Axis_VBO;

};

#endif // OPENGLWINDOW_H
