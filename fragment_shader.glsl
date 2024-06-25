#version 330 core
out vec4 FragColor;
in vec2 texCoord;

uniform int width ;
uniform int height;

vec3 frag(vec2 co)
{
    return vec3(co.x, co.y, 1);
}

void main()
{
    int x = int(texCoord.x * float(width));
    int y = int(texCoord.y * float(height));

    FragColor = vec4(frag(texCoord), 1.0);
}
