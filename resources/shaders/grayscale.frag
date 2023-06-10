#version 330 core
uniform sampler2D uTexture;
uniform float uAmount;
in vec2 vTexCoord;
out vec4 fragColor;
void main() {
    vec4 c = texture(uTexture, vTexCoord);
    float gray = dot(c.rgb, vec3(0.299, 0.587, 0.114));
    fragColor = vec4(mix(c.rgb, vec3(gray), uAmount), c.a);
}
