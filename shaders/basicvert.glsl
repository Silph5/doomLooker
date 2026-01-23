#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 3) in int vertexTexID;

out vec2 UV;
flat out int fragTexID;

uniform mat4 mvp;

void main() {
    gl_Position = mvp * vec4(vertexPosition_modelspace, 1.0);
    UV = vertexUV;
    fragTexID = vertexTexID;
}