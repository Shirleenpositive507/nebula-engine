#version 330 core
uniform sampler2D uTexture;
uniform vec4 uOutlineColor;
uniform vec2 uTexelSize;
in vec2 vTexCoord;
out vec4 fragColor;
void main() {
    vec4 c = texture(uTexture, vTexCoord);
    if(c.a > 0.0) { fragColor = c; return; }
    float o = 0.0;
    o += texture(uTexture, vTexCoord + vec2(-1,-1) * uTexelSize).a;
    o += texture(uTexture, vTexCoord + vec2(0,-1) * uTexelSize).a;
    o += texture(uTexture, vTexCoord + vec2(1,-1) * uTexelSize).a;
    o += texture(uTexture, vTexCoord + vec2(-1,0) * uTexelSize).a;
    o += texture(uTexture, vTexCoord + vec2(1,0) * uTexelSize).a;
    o += texture(uTexture, vTexCoord + vec2(-1,1) * uTexelSize).a;
    o += texture(uTexture, vTexCoord + vec2(0,1) * uTexelSize).a;
    o += texture(uTexture, vTexCoord + vec2(1,1) * uTexelSize).a;
    if(o > 0.0) fragColor = uOutlineColor;
    else fragColor = vec4(0.0);
}
