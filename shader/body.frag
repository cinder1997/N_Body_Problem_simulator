//片段着色器

#version 450 core
out vec4 FragColor;        //最终输出的片段颜色
uniform vec3 Sphere_Color;

void main()
{
    FragColor=vec4(Sphere_Color,1.0f);

}
