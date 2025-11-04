 #include "mainwidget.h"
#include "./ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    //界面初始化
    ui->setupUi(this);

    GL_window=new openglWindow();  //创建openGL窗口
    Mainlayout=new QHBoxLayout(this);
    RightWidget=new QWidget();
    gridLayout=new QGridLayout();
    time_axis_Label=new QLabel();
    Mainlayout->addWidget(ui->LeftWidget);
    Mainlayout->addWidget(RightWidget);
    gridLayout->addWidget(GL_window,0,0);
    gridLayout->addWidget(time_axis_Label,0,0);
    time_axis_Label->setText("当前时间:\n"
                             "当前步长:\n"
                             "坐标轴长度:");
    time_axis_Label->setAlignment(Qt::AlignBottom);
    time_axis_Label->setStyleSheet("background-color: rgba(0,0,0,0);color: rgba(255,255,255,255);margin: 10px;");
    gridLayout->setContentsMargins(0, 0, 0, 0);
    Mainlayout->setStretch(0,1);
    Mainlayout->setStretch(1,3);  //设置左右窗口比例
    Mainlayout->setContentsMargins(0, 0, 0, 0);
    RightWidget->setLayout(gridLayout);
    this->setLayout(Mainlayout);

    ui->scrollArea->setWidget(ui->scrollAreaContents);
    scrollAreaLayout=new QVBoxLayout();
    ui->scrollAreaContents->setLayout(scrollAreaLayout);

    ui->lineEdit_G->setText(QString::number(G,'e',4));
    ui->lineEdit_G->setPlaceholderText("单位：​​N·m²/kg²");
    ui->lineEdit_RKF45Tol->setText(QString::number(RKF45Tol,'e',2));

    //计算程序初始化，将天体数据绑定到计算程序和openGL渲染管线中
    Simulator=new N_body_Simulation(GL_window); //创建计算程序
    Simulator->setLabel(time_axis_Label);
    Simulator->setBody_data(&Body_data);
    GL_window->setBody_data(&Body_data);

    //绑定信号与槽
    connect(ui->StartButton,&QPushButton::clicked,this,&MainWidget::startSim);
    connect(ui->pauseButton,&QPushButton::clicked,this,&MainWidget::pause_continue);
    connect(Simulator,&N_body_Simulation::bodyCollision,this,&MainWidget::handle_bodyCollision);
    connect(ui->openDataButton,&QPushButton::clicked,this,&MainWidget::openDataFile);
    connect(ui->addButton,&QPushButton::clicked,this,[this]() {
        QVector3D pos=QVector3D(RandomNum->bounded(2e11),RandomNum->bounded(2e11),RandomNum->bounded(2e11)); //生成随机位置
        QVector3D vel=QVector3D(RandomNum->bounded(1e4),RandomNum->bounded(1e4),RandomNum->bounded(1e4)); //生成随机速度
        QVector3D color=QVector3D(RandomNum->bounded(1.0f),RandomNum->bounded(1.0f),RandomNum->bounded(1.0f)); //生成随机颜色
        addBody(2.0e30,pos,vel,color,2.0f);
    });

    //添加初始三个天体:质量、位置、速度、颜色、半径
    //日-地-月系统
    addBody(1.989e30,QVector3D(0.0f,0.0f,0.0f),QVector3D(0.0f,0.0f,0.0f),QVector3D(0.8f,0.2f,0.2f),5.0f);
    addBody(5.9722e24,QVector3D(1.496e11,0.0f,0.0f),QVector3D(0.0f,0.0f,3e4),QVector3D(0.1f,0.1f,1.0f),2.0f);
    addBody(7.349e22,QVector3D(1.496e11+3.844e8,0.0f,0.0f),QVector3D(0.0f,1.023e3,3e4),QVector3D(0.2f,0.8f,0.2f),1.0f);
    // 经典三体问题 - 混沌系统
    // addBody(2.0e30,QVector3D(0.0f,0.0f,0.0f),QVector3D(0.0f,0.0f,0.0f),QVector3D(0.8f,0.2f,0.2f),2.0f);
    // addBody(2.0e30,QVector3D(1.5e11,0.0f,0.0f),QVector3D(0.0, 3.0e4, 0.0),QVector3D(0.2f,0.8f,0.2f),2.0f);
    // addBody(2.0e30,QVector3D(0.0f,1.5e11,0.0f),QVector3D(-2.5e4, 0.0, 0.0),QVector3D(0.2f,0.2f,0.8f),2.0f);

}

MainWidget::~MainWidget()
{
    delete ui;
    delete GL_window;
    delete Mainlayout;
    delete Simulator;
    delete scrollAreaLayout;
    delete RightWidget;
    delete gridLayout;
    delete time_axis_Label;
    foreach (auto & item, Body_data) {
        delete item;
    }
}


void MainWidget::openDataFile()
{
    QDir dir(QDir::currentPath()+"/output_data");
    if(!dir.exists()){
        dir=QDir(QDir::currentPath());
        dir.mkpath("output_data");
    };
    QUrl folderUrl = QUrl::fromLocalFile(QDir::currentPath()+"/output_data");
    QDesktopServices::openUrl(folderUrl); //打开数据存放文件夹
}


void MainWidget::addBody(double mass, QVector3D pos, QVector3D vel, QVector3D color,float radius)           //添加天体
{
    body *newBody=new body(mass,pos,vel,color,radius);
    Body_data.append(newBody);
    Body_data.last()->Body_Num=Body_data.size();
    //根据天体颜色设置背景颜色
    int r = static_cast<int>(color.x() * 255);
    int g = static_cast<int>(color.y() * 255);
    int b = static_cast<int>(color.z() * 255);
    QString Newcolor=QString::number(r)+","+QString::number(g)+","+QString::number(b);
    QString str=Body_data.last()->thisWidget.styleSheet()+
                "QWidget{background-color: rgba(255,255,255,200);border-radius: 10px;border: 1px solid rgb("+Newcolor+");"+"}";
    Body_data.last()->thisWidget.setStyleSheet(str);
    str="background-color: rgb("+Newcolor+");";
    Body_data.last()->deleteButton.setStyleSheet(str);
    str="QSlider::sub-page:horizontal {background-color: rgb("+Newcolor+");}";
    Body_data.last()->radis_slider.setStyleSheet(str);
    Body_data.last()->radis_slider.setFocusPolicy(Qt::NoFocus);
    scrollAreaLayout->addWidget(&Body_data.last()->thisWidget, Qt::AlignBottom);
    GL_window->paint_N_body();
    //绑定删除按钮槽函数
    connect(&Body_data.last()->deleteButton,&QPushButton::clicked,this,[=](){
        if(newBody){
            //解除connect
            newBody->thisWidget.hide(); //删除界面
            Body_data.remove(Body_data.indexOf(newBody));//删除列表元素
            delete newBody;             //释放内存
            GL_window->paint_N_body();
            qDebug()<<"当前天体数量:"<<Body_data.size();
        }
    });

    qDebug()<<"当前天体数量:"<<Body_data.size();
}


void MainWidget::startSim()
{
    Simulator->Pause();
    isStart=false;
    ui->pauseButton->setText("停止");

    bool success;
    G=ui->lineEdit_G->text().toDouble(&success);
    if(!success){QMessageBox::warning(this, "警告", "引力常数有误，请重新输入");return;};
    RKF45Tol=ui->lineEdit_RKF45Tol->text().toDouble(&success);
    if(success==false | RKF45Tol<=0){QMessageBox::warning(this, "警告", "时间步长有误，请重新输入");return;};
    foreach (auto &item, Body_data) {
        double newMass=item->m_edit.text().toDouble(&success);
        if(success==false | newMass<=0){QMessageBox::warning(this, "警告", "天体质量有误，请重新输入");return;};
        QVector3D newPos=body::QstringToQVector3D(item->p_edit.text(),&success);
        if(!success){QMessageBox::warning(this, "警告", "天体位置有误，请重新输入");return;};
        QVector3D newVel=body::QstringToQVector3D(item->v_edit.text(),&success);
        if(!success){QMessageBox::warning(this, "警告", "天体速度有误，请重新输入");return;};
        float newRadius=item->radis_slider.value();

        item->mass=newMass;
        item->position=newPos;
        item->velocity=newVel;
        item->radius=newRadius;
        item->TraceLine.clear();
    };

    GL_window->timeStep=this->RKF45Tol;
    GL_window->refreshAxis();
    Simulator->CollisionMessage="";
    Simulator->startSimulation(G,RKF45Tol);  //开始N体问题仿真
    isStart=true;isPause=false;
}

void MainWidget::pause_continue()
{
    if(!isStart)return;

    isPause=!isPause;
    if(isPause){
        Simulator->Pause();
        ui->pauseButton->setText("继续");
    }
    else{
        Simulator->Continue();
        ui->pauseButton->setText("停止");
    }
}


void MainWidget::handle_bodyCollision(int i, int j)  //如天体发生碰撞，则将两天体按动量守恒定理合并为一个天体
{
    Simulator->Pause();
    Simulator->CollisionMessage="天体"+QString::number(i+1)+"和天体"+QString::number(j+1)+"发生碰撞";
    qDebug()<<Simulator->CollisionMessage;
    body *body1=Body_data[i];
    body *body2=Body_data[j];

    //添加新天体
    double newMass=body1->mass+body2->mass;
    QVector3D newPos=body1->position;
    QVector3D newVel=(body1->velocity*body1->mass + body2->velocity*body2->mass)/newMass;
    QVector3D newColor=body1->Color;
    float newRadius=body1->radius;
    addBody(newMass,newPos,newVel,newColor,newRadius);
    //删除原有天体
    body1->thisWidget.hide();
    body2->thisWidget.hide();
    Body_data.remove(Body_data.indexOf(body1));
    Body_data.remove(Body_data.indexOf(body2));
    delete body1;     //释放内存
    delete body2;     //释放内存

    GL_window->paint_N_body();
    qDebug()<<"碰撞合并后，当前天体数量:"<<Body_data.size();
    Simulator->Continue();
}
