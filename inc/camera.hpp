#pragma once
#include <cmath>

struct Camera3D {
    float distance = 2.5f;
    float yaw = 0.0f;    // Horizontal angle (radians)
    float pitch = 0.0f;  // Vertical angle (radians)
    float targetX = 0.0f, targetY = 0.0f, targetZ = 0.0f;
    float fov = 45.0f;
    float minDistance = 0.5f, maxDistance = 10.0f;

    // Returns camera position in world space
    void getPosition(float& x, float& y, float& z) const {
        x = targetX + distance * cosf(pitch) * sinf(yaw);
        y = targetY + distance * sinf(pitch);
        z = targetZ + distance * cosf(pitch) * cosf(yaw);
    }
};
