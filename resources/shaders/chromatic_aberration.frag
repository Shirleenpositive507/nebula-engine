#version 330 core
uniform sampler2D uTexture;
uniform float uStrength;
in vec2 vTexCoord;
out vec4 fragColor;
void main() {
    vec2 center = vec2(0.5);
    vec2 dir = vTexCoord - center;
    float r = texture(uTexture, vTexCoord + dir * uStrength * 0.01).r;
    float g = texture(uTexture, vTexCoord).g;
    float b = texture(uTexture, vTexCoord - dir * uStrength * 0.01).b;
    fragColor = vec4(r, g, b, 1.0);
}
