#ifndef BODY_H
#define BODY_H
#include <QVector3D>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>

class body
{
public:
    body(const double newMass = 1.0,
         const QVector3D newPos=QVector3D{0.0f,0.0f,0.0f},
         const QVector3D newVel=QVector3D{0.0f,0.0f,0.0f},
         const QVector3D newColor=QVector3D{1.0f,1.0f,1.0f},
         const float newRadius=1.0f
         );
    ~body();
    static QString QVector3DtoQString(QVector3D in);
    static QVector3D QstringToQVector3D(const QString& str, bool* ok  = nullptr);
    void setPosition(QVector3D& newPos);

    int Body_Num=0; //序号

    double mass=1.0;                            // 质量 (kg)
    QVector3D position={0.0f,0.0f,0.0f};        // 位置 (m)
    QVector3D velocity={0.0f,0.0f,0.0f};        // 速度 (m/s)
    QVector3D acceleration={0.0f,0.0f,0.0f};    // 加速度 (m/s²)
    QVector<QVector3D> TraceLine;               // 保存历史数据，用于绘制轨迹线
    int trace_len=100;                           // 历史数据个数

    QVector3D Color=QVector3D(1.0f,1.0f,1.0f);  // 颜色
    float radius=1.0;                           // 半径

    QWidget thisWidget;                         // 天体子窗口
    QVBoxLayout WidgetLayout;
    QPushButton deleteButton;
    QHBoxLayout HLayout1;
    QLabel m_lable;
    QLineEdit m_edit;
    QHBoxLayout HLayout2;
    QLabel p_lable;
    QLineEdit p_edit;
    QHBoxLayout HLayout3;
    QLabel v_lable;
    QLineEdit v_edit;
    QSlider radis_slider;
};

inline body::body(const double newMass, const QVector3D newPos, const QVector3D newVel, const QVector3D newColor, const float newRadius)
{
    this->mass=newMass;
    this->position=newPos;
    this->velocity=newVel;
    this->Color=newColor;
    this->radius=newRadius;
    TraceLine.clear();
    TraceLine.append(newPos);

    deleteButton.setText("删除");
    m_lable.setText("质量:");
    m_edit.setText(QString::number(newMass));
    m_edit.setPlaceholderText("单位：kg");
    HLayout1.addWidget(&m_lable);
    HLayout1.addWidget(&m_edit);
    p_lable.setText("初始位置:");
    p_edit.setText(QVector3DtoQString(newPos));
    p_edit.setPlaceholderText("(Px,Py,Pz) 单位：m");
    HLayout2.addWidget(&p_lable);
    HLayout2.addWidget(&p_edit);
    v_lable.setText("初始速度");
    v_edit.setText(QVector3DtoQString(newVel));
    v_edit.setPlaceholderText("(Vx,Vy,Vz) 单位：m/s²");
    HLayout3.addWidget(&v_lable);
    HLayout3.addWidget(&v_edit);
    radis_slider.setOrientation(Qt::Horizontal);
    radis_slider.setRange(1,5);
    radis_slider.setValue(newRadius);

    WidgetLayout.addWidget(&deleteButton);
    WidgetLayout.addLayout(&HLayout1);
    WidgetLayout.addLayout(&HLayout2);
    WidgetLayout.addLayout(&HLayout3);
    WidgetLayout.addWidget(&radis_slider);

    thisWidget.setLayout(&WidgetLayout);

}

inline body::~body()
{

}

inline QString body::QVector3DtoQString(QVector3D in)
{
    return QString::number(in.x())+","+QString::number(in.y())+","+QString::number(in.z());
}


inline QVector3D body::QstringToQVector3D(const QString &str, bool *ok )
{
    if (ok) *ok  = false;

    QStringList parts = str.split(',');
    if (parts.size() != 3) {
        qWarning() << "字符串格式错误，需要3个数值，得到:" << parts.size();
        return QVector3D();
    }

    bool xOk, yOk, zOk;
    float x = parts[0].toFloat(&xOk);
    float y = parts[1].toFloat(&yOk);
    float z = parts[2].toFloat(&zOk);

    if (!xOk || !yOk || !zOk) {
        qWarning() << "数值转换失败";
        return QVector3D();
    }

    if (ok) *ok  = true;
    return QVector3D(x, y, z);
}

inline void body::setPosition(QVector3D &newPos)
{
    this->position=newPos;
    this->TraceLine.append(newPos);
    if(TraceLine.size()>trace_len){ //最多保存30个历史点位置
        TraceLine.pop_front();
    }
}


#endif // BODY_H
