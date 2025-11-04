//片段着色器

#version 450 core
out vec4 FragColor;        //最终输出的片段颜色
in vec3 axis_Color;

void main()
{
    FragColor=vec4(axis_Color,1.0f);

}
