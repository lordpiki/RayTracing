#version 330 core
out vec4 FragColor;
in vec2 texCoord;
uniform int width;
uniform int height;

vec3 rayTrace(int x, int y, int width, int height) {
    return vec3(float(x) / float(width), float(y) / float(height), 0.0f);
}

void main() 
{
    int x = int(texCoord.x * float(width));
    int y = int(texCoord.y * float(height));
    vec3 color = rayTrace(x, y, width, height);
    FragColor = vec4(color, 1.0);
}
