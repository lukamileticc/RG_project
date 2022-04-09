#version 330 core

out vec4 FragColor;

in vec2 TexCords;

uniform sampler2D tex0;
uniform sampler2D tex1;

void main(){

    FragColor = mix(texture(tex0,TexCords),texture(tex1,TexCords),0.2);
}