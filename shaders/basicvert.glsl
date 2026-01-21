#version 330 core

#define MAX_SUBTEXTURES 500

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 3) in int vertexTexID;

out vec2 UV;

uniform mat4 mvp;
uniform vec4 atlasRects[MAX_SUBTEXTURES];

void main() {
    gl_Position = mvp * vec4(vertexPosition_modelspace, 1.0);

    vec4 rect = atlasRects[vertexTexID];

    UV = mix(rect.xy, rect.zw, vertexUV);
}