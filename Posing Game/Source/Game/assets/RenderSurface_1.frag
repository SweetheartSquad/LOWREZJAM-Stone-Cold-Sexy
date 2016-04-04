#version 150

in vec2 Texcoord;

out vec4 outColor;

uniform sampler2D texFramebuffer;
uniform float time = 0;
uniform float whiteout = 0;

void main() {
    outColor = mix(vec4(texture(texFramebuffer, Texcoord)), vec4(1), whiteout);
}
