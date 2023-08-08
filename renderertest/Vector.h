#pragma once

typedef struct {
	float x, y;
} vec2_t;

typedef struct {
	float x, y, z;
} vec3_t;

typedef struct {
	float x, y, z, w;
} vec4_t;

float vec2_length(vec2_t v);
vec2_t vec2_add(vec2_t a, vec2_t b);
vec2_t vec2_sub(vec2_t a, vec2_t b);
vec2_t vec2_multiply_s(vec2_t v, float scalar);
vec2_t vec2_div_s(vec2_t v, float scalar);
vec2_t vec2_normalize_copy(vec2_t v);
void vec2_normalize_ref(vec2_t* v);
float vec2_dot(vec2_t a, vec2_t b);


float vec3_length(vec3_t v);
vec3_t vec3_add(vec3_t a, vec3_t b);
vec3_t vec3_sub(vec3_t a, vec3_t b);
vec3_t vec3_multiply_s(vec3_t v, float scalar);
vec3_t vec3_div_s(vec3_t v, float scalar);
vec3_t vec3_cross(vec3_t a, vec3_t b);
float vec3_dot(vec3_t a, vec3_t b);
vec3_t vec3_normalize_copy(vec3_t v);
void vec3_normalize_ref(vec3_t* v);

vec3_t vec3_rotate_x(vec3_t v, float angle);
vec3_t vec3_rotate_y(vec3_t v, float angle);
vec3_t vec3_rotate_z(vec3_t v, float angle);
// Todo: Add functions to manipulate and operate vectors

vec4_t vec4_from_vec3(vec3_t v);
vec3_t vec3_from_vec4(vec4_t v); 
vec2_t vec2_from_vec4(vec4_t v);