#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTex0;
uniform float uScale;
uniform vec2 uTexSize;
uniform vec2 uResolution;
uniform vec2 uPos;

uniform float uBillboardSize;
uniform vec3 uBillboardColor;

uniform sampler2D uTexBrush;
uniform vec2 uBrushSize;
uniform vec2 uBrushPos;
uniform vec2 uBrushAlignment;

void main() {
    vec2 tex_uv = uTexSize / uResolution;
    vec2 canvas_offset = uPos / uResolution;
    vec2 uv = TexCoord;
    vec3 color;

    // billboard grid
    float bb_scale = uBillboardSize * uScale;
    vec2 n = ceil((TexCoord - canvas_offset) * uResolution / bb_scale);
    float r = mod(n.x + n.y, 2.0) + 1.0;
    color = vec3(r) * uBillboardColor;

    uv = (uv - canvas_offset) / (tex_uv * uScale);
    
    if (uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0)
    {
        vec4 tex = texture(uTex0, uv);
        if (tex.a > 0.0)
            color = tex.rgb;
    }
    else
    {
        color = vec3(0.1);
    }

    // brush
    vec2 b_tex_uv = uBrushSize / uResolution * uScale;
    vec2 b_offset = (uBrushPos - uBrushAlignment) / uResolution * uScale + canvas_offset;
    vec2 b_uv = (TexCoord - b_offset) / b_tex_uv;

    vec4 tex_brush = vec4(0.0);
    if (b_uv.x >= 0.0 && b_uv.x <= 1.0 && b_uv.y >= 0.0 && b_uv.y <= 1.0)
    {
        tex_brush = texture(uTexBrush, b_uv);
        if (uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0)
        {
            color = mix(color, tex_brush.rgb, tex_brush.a);
        }
    }

    // outline
    if (tex_brush.a <= 0.0)
    {
        vec2 texel_radius = 1.0 / (b_tex_uv * uResolution);

        const int SAMPLES = 8;
        bool near_edge = false;
        for (int i = 0; i < SAMPLES; ++i)
        {
            float angle = 6.2831853 * float(i) / float(SAMPLES);
            vec2 offset = vec2(cos(angle), sin(angle)) * texel_radius;
            vec2 sample_uv = b_uv + offset;

            if (sample_uv.x < 0.0 || sample_uv.x > 1.0 || sample_uv.y < 0.0 || sample_uv.y > 1.0)
                continue;

            if (texture(uTexBrush, sample_uv).a > 0.0)
            {
                near_edge = true;
                break;
            }
        }

        if (near_edge)
            color = 1.0 - color;
    }

    FragColor = vec4(color, 1.0);
}