#include "n_body_simulation.h"


N_body_Simulation::N_body_Simulation(openglWindow *GL_window)
{
    this->GL_window=GL_window;
    connect(timer, &QTimer::timeout, this,&N_body_Simulation::TimerProcess);
}

N_body_Simulation::~N_body_Simulation()
{
    delete Body_data;
}


void N_body_Simulation::startSimulation(double G,double RKF45Tol)
{
    timer->stop();

    CurrentTime=0;
    this->G=G;
    this->RKF45Tol=RKF45Tol;
    timeStep=timeStep0;

    //创建文档
    QDir dir(QDir::currentPath()+"/output_data");
    if(!dir.exists()){
        dir=QDir(QDir::currentPath());
        dir.mkpath("output_data");
    };
    filename=QDir::currentPath()+"/output_data/N_body_simulation.txt";
    file.setFileName(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug()<<"无法打开文件" + filename;
    }
    out.setDevice(&file);
    out.setEncoding(QStringConverter::Utf8);    // 设置编码为UTF-8（支持中文）
    out.setRealNumberNotation(QTextStream::ScientificNotation); //以科学计数法保存数字
    out.setRealNumberPrecision(3);  // 设置数字的有效位数
    // 写入表头
    out <<"引力常数: "<<this->G<< "\n";
    foreach (auto &item, *Body_data) {
        out <<"天体"<<item->Body_Num<<"质量:"<<item->mass<<"kg   ,";
    }
    out<< "\n";
    out <<"当前时间s/当前步长s     ";
    foreach (auto &item, *Body_data) {
        out <<"天体"<<item->Body_Num<<"位置m,";
        out <<"天体"<<item->Body_Num<<"速度m/s,";
        out <<"天体"<<item->Body_Num<<"加速度m/s²,";out<< "      ";
    }
    out<< "\n\n";
    file.close();

    timer->setInterval(TimeInterval); //每100ms绘制一帧
    timer->start();          //启动定时器，开始仿真
}

void N_body_Simulation::Pause()
{
    timer->stop();
}

void N_body_Simulation::Continue()
{
    timer->start();
}

void N_body_Simulation::setBody_data(QVector<body *> *Body_data)
{
    this->Body_data=Body_data;
}

void N_body_Simulation::setLabel(QLabel *time_axis_Label)
{
    this->time_axis_Label=time_axis_Label;
}


void N_body_Simulation::TimerProcess()  //定时器中断函数，进行单步计算并在openGL窗口中重新绘制
{
    rungeKuttaFehlberg45(); //RKF45单步计算
    //rungeKuttaStep();

    if(isCollision){  //如发生碰撞则停止当前计算程序
        return;
    }

    //写入数据
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        qDebug()<<"无法打开文件" + filename;
    }
    else {
        QFileInfo fileInfo(filename);
        if(fileInfo.exists()){
            qint64 fileSize=fileInfo.size();
            if(fileSize/(1024.0 * 1024.0)<100){  //文档超过100M就不再写入了
                out <<CurrentTime<<","<<timeStep<<"     ";
                foreach (auto &item, *Body_data) {
                    out <<"("<<item->position.x()<<","<<item->position.y()<<","<<item->position.z()<<"),";
                    out <<"("<<item->velocity.x()<<","<<item->velocity.y()<<","<<item->velocity.z()<<"),";
                    out <<"("<<item->acceleration.x()<<","<<item->acceleration.y()<<","<<item->acceleration.z()<<"),     ";
                }
                out<< "\n";
            }
        }
        file.close();
    }

    GL_window->paint_N_body();
    time_axis_Label->setText(CollisionMessage+"\n"
                             "当前时间:"+QString::number(CurrentTime,'e',2)+" s\n"
                             "当前步长:"+QString::number(timeStep,'e',2)+" s\n"
                             "坐标轴长度:"+QString::number(GL_window->getAxisLength(),'e',2)+" m");

}


void N_body_Simulation::rungeKuttaFehlberg45()
{
    int n=Body_data->size();
    if (n == 0) return;

    for(int count=0;count<10;count++){ //最多迭代十次
        QVector<double> bodies_mass;
        QVector<QVector3D> origin_State;
        QVector<QVector3D> k1,k2,k3,k4,k5,k6;
        QVector<QVector3D> state1,state2,state3,state4,state5;
        QVector<QVector3D> newState_4ord,newState_5ord;
        isCollision=false;       //碰撞标志位
        //存储各天体当前时刻的位置和速度
        for (int i = 0; i < n; i++) {
            bodies_mass.append((*Body_data)[i]->mass);
            origin_State.append((*Body_data)[i]->position);
            origin_State.append((*Body_data)[i]->velocity);
        }

        //计算6个斜率
        k1=Q3D_multiply(dydt(bodies_mass,origin_State) ,timeStep);
        if(isCollision)return;

        state1=Q3D_add(origin_State,Q3D_multiply(k1,b21));
        k2=Q3D_multiply(dydt(bodies_mass,state1) ,timeStep) ;
        if(isCollision)return;

        state2=Q3D_add(Q3D_add(origin_State,Q3D_multiply(k1,b31)),Q3D_multiply(k2,b32));
        k3=Q3D_multiply(dydt(bodies_mass,state2) ,timeStep) ;
        if(isCollision)return;

        state3=Q3D_add(Q3D_add(Q3D_add(origin_State,Q3D_multiply(k1,b41)),Q3D_multiply(k2,b42)),Q3D_multiply(k3,b43));
        k4=Q3D_multiply(dydt(bodies_mass,state3) ,timeStep) ;
        if(isCollision)return;

        state4=Q3D_add(Q3D_add(Q3D_add(Q3D_add(origin_State,Q3D_multiply(k1,b51)),Q3D_multiply(k2,b52)),Q3D_multiply(k3,b53)),Q3D_multiply(k4,b54));
        k5=Q3D_multiply(dydt(bodies_mass,state4) ,timeStep) ;
        if(isCollision)return;

        state5=Q3D_add(Q3D_add(Q3D_add(Q3D_add(Q3D_add(origin_State,Q3D_multiply(k1,b61)),Q3D_multiply(k2,b62)),Q3D_multiply(k3,b63)),Q3D_multiply(k4,b64)),Q3D_multiply(k5,b65));
        k6=Q3D_multiply(dydt(bodies_mass,state5) ,timeStep) ;
        if(isCollision)return;


        //计算龙格库塔4阶和5阶估计
        newState_4ord=Q3D_add(Q3D_add(Q3D_add(Q3D_add(origin_State,Q3D_multiply(k1,d1)),Q3D_multiply(k3,d3)),Q3D_multiply(k4,d4)),Q3D_multiply(k5,d5));
        newState_5ord=Q3D_add(Q3D_add(Q3D_add(Q3D_add(Q3D_add(origin_State,Q3D_multiply(k1,c1)),Q3D_multiply(k3,c3)),Q3D_multiply(k4,c4)),Q3D_multiply(k5,c5)),Q3D_multiply(k6,c6));

        //计算误差,由于各个天体对时间步长的敏感程度不同，需要取其中最大的误差来控制时间步长
        double maxError=0;
        for(int i=0;i<n;i++){
            double error_pos=(newState_5ord[2*i]-newState_4ord[2*i]).length()/newState_5ord[2*i].length();       //位置误差
            //double error_vel=(newState_5ord[2*i+1]-newState_4ord[2*i+1]).length()/newState_5ord[2*i+1].length(); //速度误差
            if(maxError<error_pos)maxError=error_pos;
        }
        //qDebug()<<"maxError"<<maxError;

        //更新步长
         if(maxError<RKF45Tol){  //误差小于容差，则接受计算结果，并放大时间步长
            for (int i = 0; i < n; i++) {
                (*Body_data)[i]->setPosition(newState_5ord[2*i]);
                (*Body_data)[i]->velocity=newState_5ord[2*i+1];
            }
            CurrentTime+=timeStep;
            if(maxError!=0)timeStep=0.9*timeStep*qPow(RKF45Tol/maxError,0.2);
            return;
         }
         else{       //误差大于容差，则缩小时间步长重新计算
            timeStep=0.9*timeStep*qPow(RKF45Tol/maxError,0.25);
         }
    }
}


QVector<QVector3D> N_body_Simulation::dydt(QVector<double>& bodies_mass,QVector<QVector3D> &State)
{
    int n=bodies_mass.size();
    QVector<QVector3D> k;
    QVector3D r_vec;
    QVector3D TotalAcceleration;
    double distance;

    for (int i=0;i<n;i++){
        k.append(State[2*i+1]); //天体位置的导数为该天体的速度

        //计算加速度
        TotalAcceleration= QVector3D(0, 0, 0);
        for(int j=0;j<n;j++){
            if(i==j) continue;

            r_vec = State[2*j] - State[2*i];
            distance=r_vec.length();
            // 避免除零，添加软化长度
            double softenedDistance = sqrt(distance*distance + 1e-4);
            QVector3D Accel=G * bodies_mass[j] * r_vec / (softenedDistance * softenedDistance * softenedDistance);
            if(distance==0 | 0.5*Accel.length()*timeStep*timeStep>5*distance ){  //当两天体距离过近时，判断两者发生相撞
                isCollision=true;
                emit bodyCollision(i,j); //发出天体碰撞信号，让主程序将两天体合并为一个
                return k;
            }
            TotalAcceleration += Accel;
        }
        k.append(TotalAcceleration); //天体速度的导数为该天体的加速度，即受到的万有引力之和/质量
    }

    return k;
}

QVector<QVector3D> N_body_Simulation::Q3D_add(QVector<QVector3D> a, QVector<QVector3D> b)
{
    QVector<QVector3D> result;
    for(int i=0;i<a.size();i++){
        result.append(a[i]+b[i]);
    }
    return result;
}

QVector<QVector3D> N_body_Simulation::Q3D_multiply(QVector<QVector3D> a, double num)
{
    QVector<QVector3D> result;
    for(int i=0;i<a.size();i++){
        result.append(a[i]*num);
    }
    return result;
}


// void N_body_Simulation::rungeKuttaStep()
// {
//     int n = Body_data->size();
//     if (n == 0) return;

//     // 保存初始状态
//     QVector<double> state1_mass;
//     QVector<QVector3D> state1_pos, state1_vel;
//     foreach(auto & item,*Body_data){
//         state1_mass.append(item->mass);
//         state1_pos.append(item->position);
//         state1_vel.append(item->velocity);
//     }

//     // k1 = f(t, y)
//     QVector<QVector3D> k1_pos, k1_vel;  //位置和速度的增量
//     QVector<QVector3D> acc1 = computeAccelerations(state1_mass,state1_pos,state1_vel);
//     for (int i = 0; i < n; ++i) {
//         k1_pos.append(state1_vel[i]);   // 位置的导数是速度
//         k1_vel.append(acc1[i]);         // 速度的导数是加速度
//     }

//     // k2 = f(t + timeStep/2, y + timeStep/2 * k1)
//     QVector<QVector3D> state2_pos,state2_vel;
//     for (int i = 0; i < n; ++i) {
//         state2_pos.append(state1_pos[i] + k1_pos[i] * (timeStep/2.0));
//         state2_vel.append(state1_vel[i] + k1_vel[i] * (timeStep/2.0));
//     }
//     QVector<QVector3D> k2_pos, k2_vel;
//     QVector<QVector3D> acc2 = computeAccelerations(state1_mass,state2_pos,state2_vel);
//     for (int i = 0; i < n; ++i) {
//         k2_pos.append(state2_vel[i]);
//         k2_vel.append(acc2[i]);
//     }

//     // k3 = f(t + timeStep/2, y + timeStep/2 * k2)
//     QVector<QVector3D> state3_pos,state3_vel;
//     for (int i = 0; i < n; ++i) {
//         state3_pos.append(state1_pos[i] + k2_pos[i] * (timeStep/2.0));
//         state3_vel.append(state1_vel[i] + k2_vel[i] * (timeStep/2.0));
//     }
//     QVector<QVector3D> k3_pos, k3_vel;
//     QVector<QVector3D> acc3 = computeAccelerations(state1_mass,state3_pos,state3_vel);
//     for (int i = 0; i < n; ++i) {
//         k3_pos.append(state3_vel[i]);
//         k3_vel.append(acc3[i]);
//     }

//     // k4 = f(t + timeStep, y + timeStep * k3)
//     QVector<QVector3D> state4_pos,state4_vel;
//     for (int i = 0; i < n; ++i) {
//         state4_pos.append(state1_pos[i] + k3_pos[i] * timeStep);
//         state4_vel.append(state1_vel[i] + k3_vel[i] * timeStep);
//     }
//     QVector<QVector3D> k4_pos, k4_vel;
//     QVector<QVector3D> acc4 = computeAccelerations(state1_mass,state4_pos,state4_vel);
//     for (int i = 0; i < n; ++i) {
//         k4_pos.append(state4_vel[i]);
//         k4_vel.append(acc4[i]);
//     }

//     // 更新状态：y_{n+1} = y_n + timeStep/6 * (k1 + 2*k2 + 2*k3 + k4)
//     QVector<QVector3D> newPos,newVel;
//     for (int i = 0; i < n; ++i) {
//         QVector3D pos_increment = (k1_pos[i] + k2_pos[i]*2.0 + k3_pos[i]*2.0 + k4_pos[i]) * (timeStep/6.0);
//         QVector3D vel_increment = (k1_vel[i] + k2_vel[i]*2.0 + k3_vel[i]*2.0 + k4_vel[i]) * (timeStep/6.0);
//         newPos.append(state1_pos[i] + pos_increment);
//         newVel.append(state1_vel[i] + vel_increment);
//         (*Body_data)[i]->setPosition(newPos[i]);
//         (*Body_data)[i]->velocity=newVel[i];
//     }

//     // 更新加速度
//     QVector<QVector3D> newAccelerations = computeAccelerations(state1_mass,newPos,newVel);
//     for (int i = 0; i < n; ++i) {
//         (*Body_data)[i]->acceleration = newAccelerations[i];
//     }

// }

// QVector<QVector3D> N_body_Simulation::computeAccelerations(const QVector<double> &mass,const QVector<QVector3D> &pos,const QVector<QVector3D> &vel) const
// {
//     int n = mass.size();
//     QVector<QVector3D> accelerations(n, QVector3D());

//     for (int i = 0; i < n; ++i) {
//         QVector3D totalForce(0.0, 0.0, 0.0);

//         for (int j = 0; j < n; ++j) {
//             if (i != j) {
//                 totalForce = totalForce + computeGravity(mass[i],pos[i],vel[i],mass[j],pos[j]);
//             }
//         }
//         accelerations[i] = totalForce / mass[i];
//     }

//     return accelerations;
// }

// QVector3D N_body_Simulation::computeGravity(const double &mass1,const QVector3D &pos1,const QVector3D &vel1, const double &mass2,const QVector3D &pos2) const
// {
//     QVector3D r_vec = pos2 - pos1;
//     double distance = r_vec.length();

//     // 避免除零错误
//     if(distance==0){
//         return QVector3D(0.0,0.0,0.0);
//     }

//     // 避免引力奇点造成的数值不稳定
//     QVector3D accelerate=G*mass2/(distance*distance) *r_vec.normalized();
//     //double acc_dis=(vel1*timeStep+0.5*accelerate*(timeStep*timeStep)).length(); //计算下一个时间步长内的位移
//     double acc_dis=(0.5*accelerate*(timeStep*timeStep)).length(); //计算下一个时间步长内由加速度引起的位移
//     if (distance < 1.5*acc_dis) {
//         distance=qSqrt(distance * distance+acc_dis*acc_dis);      //软化长度
//     }

//     double force_magnitude = G * mass1 * mass2 / (distance * distance);
//     QVector3D force_direction = r_vec.normalized();

//     return force_direction * force_magnitude;
// }
