#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aRadius;
layout (location = 2) in vec3 aColor;
out vec3 vColor;
uniform mat4 view;
uniform mat4 proj;
void main() {
    gl_Position = proj * view * vec4(aPos, 1.0);
    // Use radius in world units, scale by perspective
    float baseSize = 1200.0; // Tune for your screen DPI and FOV
    gl_PointSize = max(2.0, aRadius * baseSize / gl_Position.w);
    vColor = aColor;
}
