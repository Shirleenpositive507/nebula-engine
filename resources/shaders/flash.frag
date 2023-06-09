#version 330 core
uniform sampler2D uTexture;
uniform vec4 uFlashColor;
uniform float uFlashAmount;
in vec2 vTexCoord;
out vec4 fragColor;
void main() {
    vec4 c = texture(uTexture, vTexCoord);
    fragColor = mix(c, uFlashColor, uFlashAmount);
}
