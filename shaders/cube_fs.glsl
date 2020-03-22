#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{   
    // To add a color modify the vec4
    FragColor = texture(texture1, TexCoords)+vec4(0.0,0.0,0.0,1.0);
}