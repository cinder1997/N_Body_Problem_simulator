//顶点着色器

#version 450 core
layout (location = 0) in vec3 aPos;      //模型顶点坐标，location用于区分输入量的属性
// layout (location = 1) in vec3 aColor;    //顶点颜色
// layout (location = 2) in vec2 aTexCoord; //纹理坐标
// layout (location = 3) in vec3 aNormal;   //法线方向
uniform mat4 M_model;      //模型矩阵M,将顶点从模型空间转换到世界空间
uniform mat4 M_view;       //透视矩阵V，将顶点从世界空间转换到观察空间 (相机空间)
uniform mat4 M_projection; //裁剪矩阵P，将顶点从观察空间转换到裁剪空间


void main()
{
    gl_Position = M_projection * M_view * M_model * vec4(aPos, 1.0);

}
