#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{    
    // Quick and dirty vignette
    
    vec2 st= gl_FragCoord.xy; 
    st = TexCoords;
    vec4 colorx=mix(vec4(1.,1.,1.,1.),vec4(.0,.0,.0,1.0), 2 *length(st-vec2(.5,.5)));
    FragColor = texture(texture1, TexCoords)*colorx;
}