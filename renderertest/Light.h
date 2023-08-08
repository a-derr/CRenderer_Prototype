#pragma once
#include <stdint.h>
#include <math.h>
#include "Vector.h"
typedef struct {
	vec3_t direction;
} light_t;

extern light_t light;

uint32_t light_apply_intensity(uint32_t original_color, float percentage_factor);