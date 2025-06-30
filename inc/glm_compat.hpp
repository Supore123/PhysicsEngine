#pragma once
// Minimal GLM-like math for OpenGL shaders (vec3, mat4, lookAt, perspective)
#include <cmath>

struct vec3 {
    float x, y, z;
    vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

struct mat4 {
    float m[16];
    float* operator[](int i) { return &m[i*4]; }
    const float* operator[](int i) const { return &m[i*4]; }
};

inline mat4 identity() {
    mat4 r = {};
    r.m[0]=r.m[5]=r.m[10]=r.m[15]=1.0f;
    return r;
}

inline mat4 perspective(float fovy, float aspect, float zNear, float zFar) {
    float f = 1.0f / tanf(fovy * 0.5f * 3.1415926f / 180.0f);
    mat4 r = {};
    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (zFar + zNear) / (zNear - zFar);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * zFar * zNear) / (zNear - zFar);
    return r;
}

inline mat4 lookAt(const vec3& eye, const vec3& center, const vec3& up) {
    vec3 f = {center.x-eye.x, center.y-eye.y, center.z-eye.z};
    float flen = sqrtf(f.x*f.x+f.y*f.y+f.z*f.z);
    f.x/=flen; f.y/=flen; f.z/=flen;
    vec3 s = {f.y*up.z-f.z*up.y, f.z*up.x-f.x*up.z, f.x*up.y-f.y*up.x};
    float slen = sqrtf(s.x*s.x+s.y*s.y+s.z*s.z);
    s.x/=slen; s.y/=slen; s.z/=slen;
    vec3 u = {s.y*f.z-s.z*f.y, s.z*f.x-s.x*f.z, s.x*f.y-s.y*f.x};
    mat4 r = identity();
    r.m[0]=s.x; r.m[4]=s.y; r.m[8]=s.z;
    r.m[1]=u.x; r.m[5]=u.y; r.m[9]=u.z;
    r.m[2]=-f.x; r.m[6]=-f.y; r.m[10]=-f.z;
    r.m[12]=-(s.x*eye.x+s.y*eye.y+s.z*eye.z);
    r.m[13]=-(u.x*eye.x+u.y*eye.y+u.z*eye.z);
    r.m[14]=f.x*eye.x+f.y*eye.y+f.z*eye.z;
    return r;
}
