#include "openglwindow.h"

openglWindow::openglWindow(QWidget *parent) {
    generateSphere();
}

openglWindow::~openglWindow()
{
    makeCurrent();
    Sphere_VAO.destroy();
    Sphere_VBO.destroy();
    Sphere_EBO.destroy();
    Trace_VAO.destroy();
    Trace_VBO.destroy();
    Axis_VAO.destroy();
    Axis_VBO.destroy();
    doneCurrent();
    delete Body_data;
}

void openglWindow::setBody_data(QVector<body *> *Body_data)
{

    this->Body_data=Body_data;
}

void openglWindow::generateSphere()
{
    Sphere_Vertices.clear();
    Sphere_indices.clear();

    int stacks=8;  //phi方向顶点数
    int slices=16;  //theta方向顶点数

    // 生成顶点数据
    for (int i = 0; i <= stacks; ++i) {
        float phi = static_cast<float>(i) / stacks * M_PI;

        for (int j = 0; j <= slices; ++j) {
            float theta = static_cast<float>(j) / slices * 2 * M_PI;
            float x = sin(phi) * cos(theta);
            float y = cos(phi);
            float z = sin(phi) * sin(theta);
            Sphere_Vertices.append(QVector3D(x, y, z));
        }
    }
    // 生成索引数据
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = i * (slices + 1) + j;
            int second = first + slices + 1;
            Sphere_indices.append(first);
            Sphere_indices.append(second);
            Sphere_indices.append(first + 1);
            Sphere_indices.append(second);
            Sphere_indices.append(second + 1);
            Sphere_indices.append(first + 1);
        }
    }
}

float openglWindow::getFarthestDis()
{
    float FarthestDis=0;
    float maxVal=0;
    foreach (auto &item, *Body_data) {
        maxVal = qMax(  qMax( qAbs(item->position.x()),qAbs(item->position.y()) ), qAbs(item->position.z()));
        if (FarthestDis<maxVal)FarthestDis=maxVal;
    }
    return FarthestDis;
}

float openglWindow::getFastestVel()
{
    float FastestVel=0;
    float maxVal=0;
    foreach (auto &item, *Body_data) {
        maxVal = qMax(  qMax( qAbs(item->velocity.x()),qAbs(item->velocity.y()) ), qAbs(item->velocity.z()));
        if (FastestVel<maxVal)FastestVel=maxVal;
    }
    return FastestVel;
}

void openglWindow::initializeGL()
{
    this->initializeOpenGLFunctions(); //初始化OpenGL
    glEnable(GL_DEPTH_TEST);           //启用深度测试
    //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); //线框模式

    /**************    球体：着色器程序     ***************/
    Sphere_shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/shader/body.vert"); //添加顶点着色器文件
    Sphere_shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/shader/body.frag"); //添加片段着色器文件
    bool isSucess = Sphere_shaderProgram.link(); //链接着色器程序
    if (!isSucess) {qDebug() << Sphere_shaderProgram.log();}
    /**************   球体：VAO、VBO和EBO  ***************/
    Sphere_VAO.create();
    Sphere_VAO.bind();
    Sphere_VBO=QOpenGLBuffer(QOpenGLBuffer::VertexBuffer); //指定缓冲区对象类型—顶点缓冲区对象
    Sphere_VBO.create();
    Sphere_VBO.bind();
    Sphere_VBO.allocate(Sphere_Vertices.constData(),Sphere_Vertices.size()*sizeof(QVector3D)); //向GPU传入顶点数据
    Sphere_EBO=QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);  //指定缓冲区对象类型—索引缓冲区对象
    Sphere_EBO.create();
    Sphere_EBO.bind();
    Sphere_EBO.allocate(Sphere_indices.constData(),Sphere_indices.size()*sizeof(GLushort)); //向GPU传入索引数据
    Sphere_shaderProgram.bind();
    Sphere_shaderProgram.enableAttributeArray(0);
    Sphere_shaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));
    Sphere_VAO.release();

    /**************    轨迹线：着色器程序     ***************/
    Trace_shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/shader/body.vert"); //添加顶点着色器文件
    Trace_shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/shader/body.frag"); //添加片段着色器文件
    isSucess = Trace_shaderProgram.link(); //链接着色器程序
    if (!isSucess) {qDebug() << Trace_shaderProgram.log();}
    /**************   轨迹线：VAO、VBO  ***************/
    Trace_VAO.create();
    Trace_VAO.bind();
    Trace_VBO=QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    Trace_VBO.create();
    Trace_VBO.bind();
    Trace_VBO.allocate(TraceLine.constData(),TraceLine.size()*sizeof(QVector3D));
    Trace_shaderProgram.bind();
    Trace_shaderProgram.enableAttributeArray(0);
    Trace_shaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));
    Trace_VAO.release();

    /**************    坐标轴:着色器程序     ***************/
    Axis_shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/shader/axis.vert"); //添加顶点着色器文件
    Axis_shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/shader/axis.frag"); //添加片段着色器文件
    isSucess = Axis_shaderProgram.link(); //链接着色器程序
    if (!isSucess) {qDebug() << Axis_shaderProgram.log();}
    /**************   坐标轴:VAO、VBO  ***************/
    Axis_VAO.create();
    Axis_VAO.bind();
    Axis_VBO=QOpenGLBuffer(QOpenGLBuffer::VertexBuffer); //指定缓冲区对象类型—顶点缓冲区对象
    Axis_VBO.create();
    Axis_VBO.bind();
    Axis_VBO.allocate(axisData.constData(),axisData.length()*sizeof(float)); //向GPU传入顶点数据
    Axis_shaderProgram.bind();
    Axis_shaderProgram.enableAttributeArray(0);
    Axis_shaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(float));
    Axis_shaderProgram.enableAttributeArray(1);
    Axis_shaderProgram.setAttributeBuffer(1, GL_FLOAT,3 * sizeof(float), 3, 6 * sizeof(float));
    Axis_VAO.release();

}

void openglWindow::resizeGL(int w, int h)
{

}

void openglWindow::paintGL()
{
    glClearColor(0.0f,0.1f,0.2f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //在每一帧渲染开始时,清除颜色缓冲区和深度缓冲区：


    /**************   绘制天体  ***************/
    Sphere_shaderProgram.bind();
    Sphere_VAO.bind();
    M_view.setToIdentity();
    M_projection.setToIdentity();
    float Now_farthestDis=getFarthestDis();
    if(Last_farthestDis<Now_farthestDis | Last_farthestDis/Now_farthestDis>4){  //如果天体位置超出显示区域，就重新计算缩放系数
        Last_farthestDis=Now_farthestDis*1.5;
        if(Last_farthestDis==0)Last_farthestDis=100.0;
        Dis_factor=viewLength/Last_farthestDis;  //计算屏幕缩放系数
        //qDebug()<<"画面缩放系数:"<<Dis_factor;
    }
    M_view.lookAt(cameraPos,cameraPos+cameraFront,QVector3D(0.0f,1.0f,0.0f));//调整摄影机位置
    M_projection.perspective(45.0f,(float)1.0,0.1f,1000.0f);//调整视角
    Sphere_shaderProgram.setUniformValue("M_view",M_view);
    Sphere_shaderProgram.setUniformValue("M_projection",M_projection);
    if(Body_data!=nullptr){
        foreach (auto & item, *Body_data) {
            M_model.setToIdentity();
            M_model.translate(item->position*Dis_factor);  //平移模型
            M_model.scale(item->radius);     //缩放模型
            Sphere_shaderProgram.setUniformValue("M_model",M_model);
            Sphere_shaderProgram.setUniformValue("Sphere_Color",item->Color);

            glDrawElements(GL_TRIANGLES, Sphere_indices.size(), GL_UNSIGNED_SHORT, 0); //绘制天体
        }
    }
    Sphere_VAO.release();
    Sphere_shaderProgram.release();


    /**************   绘制轨迹线  ***************/
    Trace_shaderProgram.bind();
    Trace_VAO.bind();
    M_model.setToIdentity();
    M_model.scale(Dis_factor);
    Trace_shaderProgram.setUniformValue("M_model",M_model);
    Trace_shaderProgram.setUniformValue("M_view",M_view);
    Trace_shaderProgram.setUniformValue("M_projection",M_projection);
    if(Body_data!=nullptr){
        foreach (auto & item, *Body_data){
            Trace_VBO.bind();
            Trace_VBO.allocate(item->TraceLine.constData(),item->TraceLine.size()*sizeof(QVector3D)); //向GPU传入轨迹数据
            Trace_VBO.release();

            Trace_shaderProgram.setUniformValue("Sphere_Color",item->Color);

            glLineWidth(2.0f);
            glDrawArrays(GL_LINE_STRIP, 0, item->TraceLine.size());
        }
    }
    Trace_VAO.release();
    Trace_shaderProgram.release();


    /**************   绘制坐标轴  ***************/
    Axis_shaderProgram.bind();
    Axis_VAO.bind();
    M_model.setToIdentity();
    M_model.scale(Dis_factor);
    Axis_shaderProgram.setUniformValue("M_model",M_model);
    Axis_shaderProgram.setUniformValue("M_view",M_view);
    Axis_shaderProgram.setUniformValue("M_projection",M_projection);
    glLineWidth(3.0f);
    glDrawArrays(GL_LINES, 0, 6);
    glDrawArrays(GL_TRIANGLES, 6, 9);
    Axis_VAO.release();
    Axis_shaderProgram.release();

}

void openglWindow::paint_N_body()
{
    update();
}

void openglWindow::refreshAxis()
{
    axisLength = getFarthestDis()*1.5;    //新的坐标轴长度
    if(axisLength==0)axisLength=getFastestVel()*timeStep*5;
    if(axisLength==0)axisLength=viewLength;
    arrowSize = 0.05f*axisLength;
    axisData=QVector<float>{            //新的坐标轴数据
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

    /**************   重新绑定坐标轴VAO  ***************/
    makeCurrent();// 必须确保在当前上下文
    Axis_VAO.bind();
    Axis_VBO.bind();
    Axis_VBO.allocate(axisData.constData(),axisData.length()*sizeof(float)); //向GPU传入顶点数据
    Axis_shaderProgram.bind();
    Axis_shaderProgram.enableAttributeArray(0);
    Axis_shaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(float));
    Axis_shaderProgram.enableAttributeArray(1);
    Axis_shaderProgram.setAttributeBuffer(1, GL_FLOAT,3 * sizeof(float), 3, 6 * sizeof(float));
    Axis_VAO.release();
    doneCurrent();
}

double openglWindow::getAxisLength()
{
    return this->axisLength;
}

