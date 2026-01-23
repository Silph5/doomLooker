#version 330 core
#define MAX_SUBTEXTURES 500

in vec2 UV;
flat in int fragTexID;

out vec4 color;

uniform sampler2D atlasTexture;
uniform vec4 atlasRects[MAX_SUBTEXTURES];

void main() {
    vec4 rect = atlasRects[fragTexID];
    vec2 subSize = rect.zw - rect.xy;
    vec2 tiledUV = rect.xy + (UV - floor(UV)) * subSize;

    color = texture(atlasTexture, tiledUV);
}