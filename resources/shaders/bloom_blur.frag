#version 330 core
uniform sampler2D uScene;
uniform sampler2D uBloom;
uniform float uIntensity;
in vec2 vTexCoord;
out vec4 fragColor;
void main() {
    vec4 scene = texture(uScene, vTexCoord);
    vec4 bloom = texture(uBloom, vTexCoord) * uIntensity;
    fragColor = scene + bloom;
}
