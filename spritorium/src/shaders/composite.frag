#version 330 core

// composite srcover porterduff

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTex0;
uniform sampler2D uTex1;

void main() {
    vec4 c0 = texture(uTex0, TexCoord);
    vec4 c1 = texture(uTex1, TexCoord);
    // FragColor = c1 + c0 * (1.0 - c1.a);
    vec4 col;
    float ma = (1.0 - c1.a);
    float oa = (c1.a + c0.a * ma);
    if (oa <= 0.0) {
        col = vec4(0.0);
    }
    float inv_oa = 1.0 / oa;
    col = vec4((c1.a * c1.rgb + c0.a * c0.rgb * ma) * inv_oa, oa);
    FragColor = col;
}