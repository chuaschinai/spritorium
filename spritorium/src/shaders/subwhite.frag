#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTex0;
uniform sampler2D uTex1;

void main() {
    vec4 c1 = texture(uTex1, TexCoord);
    vec4 final_col = texture(uTex0, TexCoord);
    if (c1 == vec4(1.0))
        final_col = vec4(0.0);
    FragColor = final_col;
}