#include "Matrix.h"
#include <math.h>

mat4_t mat4_identity(void) {
	mat4_t m = { {
		{ 1, 0, 0, 0},
		{ 0, 1, 0, 0},
		{ 0, 0, 1, 0},
		{ 0, 0, 0, 1}
	} };
	return m;
}

mat4_t mat4_make_scale(float sx, float sy, float sz) {
	mat4_t m_s = mat4_identity();
	m_s.m[0][0] = sx;
	m_s.m[1][1] = sy;
	m_s.m[2][2] = sz;
	return m_s;
}

mat4_t mat4_make_translation(float tx, float ty, float tz) {
	mat4_t m_trans = mat4_identity();
	m_trans.m[0][3] = tx;
	m_trans.m[1][3] = ty;
	m_trans.m[2][3] = tz;
	return m_trans;
}

mat4_t mat4_make_rotation_x(float angle) {
	float c = cos(angle);
	float s = sin(angle);
	// |  1  0  0  0  |
	// |  0  c -s  0  |
	// |  0  s  c  0  |
	// |  0  0  0  1  |
	mat4_t m_r = mat4_identity();
	m_r.m[1][1] = c;
	m_r.m[1][2] = -s;
	m_r.m[2][1] = s;
	m_r.m[2][2] = c;
	return m_r;
}

mat4_t mat4_make_rotation_y(float angle) {
	float c = cos(angle);
	float s = sin(angle);
	// |  c  0  s  0  |
	// |  0  1  0  0  |
	// | -s  0  c  0  |
	// |  0  0  0  1  |
	mat4_t m_r = mat4_identity();
	m_r.m[0][0] = c;
	m_r.m[0][2] = s;
	m_r.m[2][0] = -s;
	m_r.m[2][2] = c;
	return m_r;
}

mat4_t mat4_make_rotation_z(float angle) {
	float c = cos(angle);
	float s = sin(angle);
	// |  c -s  0  0  |
	// |  s  c  0  0  |
	// |  0  0  1  0  |
	// |  0  0  0  1  |
	mat4_t m_r = mat4_identity();
	m_r.m[0][0] = c;
	m_r.m[0][1] = -s;
	m_r.m[1][0] = s;
	m_r.m[1][1] = c;
	return m_r;
}

mat4_t mat4_make_perspective(float fov, float aspect, float znear, float zfar) {
	// | (h/w)*1/tan(fov/2)             0              0                 0 |
	// |                  0  1/tan(fov/2)              0                 0 |
	// |                  0             0     zf/(zf-zn)  (-zf*zn)/(zf-zn) |
	// |                  0             0              1                 0 |
	mat4_t m = { {{ 0 }} };
	m.m[0][0] = aspect * (1 / tan(fov / 2));
	m.m[1][1] = 1 / tan(fov / 2);
	m.m[2][2] = zfar / (zfar - znear);
	m.m[2][3] = (-zfar * znear) / (zfar - znear);
	m.m[3][2] = 1.0;
	return m;
}

vec4_t mat4_mul_vec4_project(mat4_t mat_proj, vec4_t v) {
	// multiply the projection matrix by our original vector
	vec4_t result = mat4_mul_vec4(mat_proj, v);

	// perform perspective divide with original z-value that is now stored in w
	if (result.w != 0.0) {
		result.x /= result.w;
		result.y /= result.w;
		result.z /= result.w;
	}
	return result;
}

vec4_t mat4_mul_vec4(mat4_t m, vec4_t v) {
	vec4_t result;
	result.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w;
	result.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w;
	result.z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w;
	result.w = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w;
	return result;
}

mat4_t mat4_mul_mat4(mat4_t a, mat4_t b) {
	mat4_t result;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] + a.m[i][2] * b.m[2][j] + a.m[i][3] * b.m[3][j];
		}
	}
	return result;
}

mat4_t mat4_look_at(vec3_t eye, vec3_t target, vec3_t up) {
	vec3_t z = vec3_normalize_copy(vec3_sub(target, eye));		// forward (z) vector
	vec3_t x = vec3_normalize_copy(vec3_cross(up, z));			// right (x) vector
	vec3_t y = vec3_cross(z, x);								// up (y) vector

	// | x.x	 x.y	 x.z	-dot(x, eye) |
	// | y.x	 y.y	 y.z	-dot(y, eye) |
	// | z.x	 z.y	 z.z	-dot(z, eye) |
	// |   0	   0	   0 			   1 |

	mat4_t view_matrix = { {
		{x.x, x.y, x.z, -vec3_dot(x, eye)},
		{y.x, y.y, y.z, -vec3_dot(y, eye)},
		{z.x, z.y, z.z, -vec3_dot(z, eye)},
		{  0,	0,	 0,					1},
	} };
	/*mat4_t view_matrix = { {
		{x.x, y.x, z.x, -vec3_dot(x, eye)},
		{x.y, y.y, z.y, -vec3_dot(y, eye)},
		{x.z, y.z, z.z, -vec3_dot(z, eye)},
		{  0,	0,	 0,					1},
	} };*/
	return view_matrix;
}