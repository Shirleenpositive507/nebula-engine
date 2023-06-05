#version 330 core
uniform sampler2D uTexture;
uniform float uIntensity;
uniform float uRadius;
uniform vec4 uColor;
in vec2 vTexCoord;
out vec4 fragColor;
void main() {
    vec4 c = texture(uTexture, vTexCoord);
    vec2 center = vec2(0.5);
    float dist = distance(vTexCoord, center);
    float vignette = 1.0 - smoothstep(uRadius, 1.0, dist * 1.414);
    vignette = mix(1.0, vignette, uIntensity);
    fragColor = mix(uColor, c, vignette);
}
