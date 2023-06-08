#version 330 core
uniform sampler2D uTexture;
uniform int uOperator; // 0=Reinhard, 1=ACES, 2=Filmic
uniform float uExposure;
in vec2 vTexCoord;
out vec4 fragColor;
vec3 reinhard(vec3 c) { return c / (c + vec3(1.0)); }
vec3 aces(vec3 c) {
    float a = 2.51, b = 0.03, c_ = 2.43, d = 0.59, e = 0.14;
    return clamp((c * (a * c + b)) / (c * (c_ * c + d) + e), 0.0, 1.0);
}
vec3 filmic(vec3 c) {
    vec3 x = max(vec3(0.0), c - vec3(0.004));
    return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
}
void main() {
    vec3 color = texture(uTexture, vTexCoord).rgb * uExposure;
    if(uOperator == 0) color = reinhard(color);
    else if(uOperator == 1) color = aces(color);
    else color = filmic(color);
    fragColor = vec4(color, 1.0);
}
