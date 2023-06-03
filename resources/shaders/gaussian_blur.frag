#version 330 core
uniform sampler2D uTexture;
uniform vec2 uDirection;
uniform float uRadius;
in vec2 vTexCoord;
out vec4 fragColor;
const float weights[7] = float[](0.006, 0.061, 0.242, 0.383, 0.242, 0.061, 0.006);
void main() {
    vec2 texelSize = 1.0 / textureSize(uTexture, 0);
    vec4 color = vec4(0.0);
    for(int i = -3; i <= 3; i++) {
        vec2 offset = vec2(float(i)) * uDirection * texelSize * uRadius;
        color += texture(uTexture, vTexCoord + offset) * weights[i + 3];
    }
    fragColor = color;
}
