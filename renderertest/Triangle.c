#include "Triangle.h"

void draw_triangle_pixel(int x, int y, uint32_t color, vec4_t point_a, vec4_t point_b, vec4_t point_c) {
	
	vec2_t p = { x, y };
	vec2_t a = vec2_from_vec4(point_a);
	vec2_t b = vec2_from_vec4(point_b);
	vec2_t c = vec2_from_vec4(point_c);

	vec3_t weights = barycentric_weights(a, b, c, p);

	float alpha = weights.x;
	float beta = weights.y;
	float gamma = weights.z;

	float interpolated_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

	// adjust 1/w so the pixels tat are closer to the camera have smaller values
	interpolated_w = 1.0 - interpolated_w;

	if (interpolated_w < z_buffer[(window_width * y) + x]) {
		draw_pixel(x, y, color);

		//update z buffer value with 1/w of current pixel
		z_buffer[(window_width * y) + x] = interpolated_w;
	}
}

void draw_filled_triangle(int x0, int y0, float z0, float w0, int x1, int y1, float z1, float w1, int x2, int y2, float z2, float w2, uint32_t color) {
	// Loop all pixels of the triangle to render them based on the color that comes from the texture
	if (y0 > y1) {
		int_swap(&x0, &x1);
		int_swap(&y0, &y1);
		float_swap(&z0, &z1);
		float_swap(&w0, &w1);
	}
	if (y1 > y2) {
		int_swap(&x1, &x2);
		int_swap(&y1, &y2);
		float_swap(&z1, &z2);
		float_swap(&w1, &w2);
	}
	if (y0 > y1) {
		int_swap(&y0, &y1);
		int_swap(&x0, &x1);
		float_swap(&z0, &z1);
		float_swap(&w0, &w1);
	}

	// Create vector points after we sort the vertices
	vec4_t point_a = { x0, y0, z0, w0 };
	vec4_t point_b = { x1, y1, z1, w1 };
	vec4_t point_c = { x2, y2, z2, w2 };

	//////////////////////////////////////////////////
	// Render upper part of the triangle (flat-bottom)
	//////////////////////////////////////////////////
	float inv_slope_1 = 0;
	float inv_slope_2 = 0;

	

	if ((y1 - y0) != 0)
		inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
	if ((y2 - y0) != 0)
		inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);
	if (y1 - y0 != 0) {
		for (int y = y0; y <= y1; y++) {
			int x_start = x1 + (y - y1) * inv_slope_1;
			int x_end = x0 + (y - y0) * inv_slope_2;
			// swap if x_start is to the right of x_end
			if (x_end < x_start)
				int_swap(&x_start, &x_end);

			for (int x = x_start; x < x_end; x++) {
				draw_triangle_pixel(x, y, color, point_a, point_b, point_c);
			}
		}
	}

	///////////////////////////////////////////////
	// Render Lower part of the triangle (flat-top)
	///////////////////////////////////////////////
	inv_slope_1 = 0;
	inv_slope_2 = 0;

	if ((y2 - y1) != 0)
		inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
	if ((y2 - y0) != 0)
		inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);
	if (y2 - y1 != 0) {
		for (int y = y1; y <= y2; y++) {
			int x_start = x1 + (y - y1) * inv_slope_1;
			int x_end = x0 + (y - y0) * inv_slope_2;
			// swap if x_start is to the right of x_end
			if (x_end < x_start)
				int_swap(&x_start, &x_end);

			for (int x = x_start; x < x_end; x++) {
				draw_triangle_pixel(x, y, color, point_a, point_b, point_c);
			}
		}
	}
}

void draw_texel(int x, int y, vec4_t point_a, vec4_t point_b, vec4_t point_c,
	tex2_t a_uv, tex2_t b_uv, tex2_t c_uv,
	uint32_t* texture)
{
	vec2_t p = { x, y };
	vec2_t a = vec2_from_vec4(point_a);
	vec2_t b = vec2_from_vec4(point_b);
	vec2_t c = vec2_from_vec4(point_c);

	vec3_t weights = barycentric_weights(a, b, c, p);

	float alpha = weights.x;
	float beta = weights.y;
	float gamma = weights.z;

	// Variables store the interpolated values of U,V and also W for the current pixel
	float interpolated_u;
	float interpolated_v;
	float interpolated_reciprocal_w;

	// Perform the interpolation of all U/w and V/w values using barycentric weights and a factor of 1/w
	interpolated_u = (a_uv.u / point_a.w) * alpha + (b_uv.u / point_b.w) * beta + (c_uv.u / point_c.w) * gamma;
	interpolated_v = (a_uv.v / point_a.w) * alpha + (b_uv.v / point_b.w) * beta + (c_uv.v / point_c.w) * gamma;

	// interpolate the value of 1/w for the current pixel
	interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

	// Now divide back both interpolated values by the 1/w
	interpolated_u /= interpolated_reciprocal_w;
	interpolated_v /= interpolated_reciprocal_w;

	// Map the UV coordinate to the full texture width and height
	int texel_x = abs((int)(interpolated_u * texture_width)) % texture_width;
	int texel_y = abs((int)(interpolated_v * texture_height)) % texture_height;

	// adjust 1/w so the pixels that are closer to the camera have smaller values
	interpolated_reciprocal_w = 1.0 - interpolated_reciprocal_w;

	if (interpolated_reciprocal_w < z_buffer[(window_width * y) + x]) {
		draw_pixel(x, y, texture[(texture_width * texel_y) + texel_x]);

		//update z buffer value with 1/w of current pixel
		z_buffer[(window_width * y) + x] = interpolated_reciprocal_w;
	}
}


vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p) {
	vec2_t ac = vec2_sub(c, a);
	vec2_t ab = vec2_sub(b, a);
	vec2_t pc = vec2_sub(c, p);
	vec2_t pb = vec2_sub(b, p);
	vec2_t ap = vec2_sub(p, a);

	float area_of_parallelogram_abc = (ac.x * ab.y - ac.y * ab.x);

	float alpha = (pc.x * pb.y - pc.y * pb.x) / area_of_parallelogram_abc;

	float beta = (ac.x * ap.y - ac.y * ap.x) / area_of_parallelogram_abc;

	float gamma = (1 - alpha - beta);

	vec3_t weights = {
		.x = alpha,
		.y = beta,
		.z = gamma
	};

	return weights;
}

void draw_textured_triangle(int x0, int y0, float z0, float w0, float u0, float v0, 
							int x1, int y1, float z1, float w1, float u1, float v1,
							int x2, int y2, float z2, float w2, float u2, float v2,
							uint32_t* texture) 
{
	// Loop all pixels of the triangle to render them based on the color that comes from the texture
	if (y0 > y1) {
		int_swap(&x0, &x1);
		int_swap(&y0, &y1);
		float_swap(&z0, &z1);
		float_swap(&w0, &w1);
		float_swap(&u0, &u1);
		float_swap(&v0, &v1);
	}
	if (y1 > y2) {
		int_swap(&x1, &x2);
		int_swap(&y1, &y2);
		float_swap(&z1, &z2);
		float_swap(&w1, &w2);
		float_swap(&u1, &u2);
		float_swap(&v1, &v2);
	}
	if (y0 > y1) {
		int_swap(&y0, &y1);
		int_swap(&x0, &x1);
		float_swap(&z0, &z1);
		float_swap(&w0, &w1);
		float_swap(&u0, &u1);
		float_swap(&v0, &v1);
	}

	// flip v coordinates for proper texture mapping
	v0 = 1.0 - v0;
	v1 = 1.0 - v1;
	v2 = 1.0 - v2;
	// Create vector points after we sort the vertices
	vec4_t point_a = { x0, y0, z0, w0 };
	vec4_t point_b = { x1, y1, z1, w1 };
	vec4_t point_c = { x2, y2, z2, w2 };
	tex2_t a_uv = { u0, v0 };
	tex2_t b_uv = { u1, v1 };
	tex2_t c_uv = { u2, v2 };

	//////////////////////////////////////////////////
	// Render upper part of the triangle (flat-bottom)
	//////////////////////////////////////////////////
	float inv_slope_1 = 0;
	float inv_slope_2 = 0;

	if( (y1 - y0) != 0)
		inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
	if ((y2 - y0) != 0)
		inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);
	if (y1 - y0 != 0) {
		for (int y = y0; y <= y1; y++) {
			int x_start = x1 + (y - y1) * inv_slope_1;
			int x_end = x0 + (y - y0) * inv_slope_2;
			// swap if x_start is to the right of x_end
			if (x_end < x_start)
				int_swap(&x_start, &x_end);

			for (int x = x_start; x < x_end; x++) {
				// Draw our pixel with the color that comes from the texture
				// Test draw call
				draw_texel(x, y, point_a, point_b, point_c,
						   a_uv, b_uv, c_uv,
						   texture);
			}
		}
	}

	///////////////////////////////////////////////
	// Render Lower part of the triangle (flat-top)
	///////////////////////////////////////////////
	inv_slope_1 = 0;
	inv_slope_2 = 0;

	if ((y2 - y1) != 0)
		inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
	if ((y2 - y0) != 0)
		inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);
	if (y2 - y1 != 0) {
		for (int y = y1; y <= y2; y++) {
			int x_start = x1 + (y - y1) * inv_slope_1;
			int x_end = x0 + (y - y0) * inv_slope_2;
			// swap if x_start is to the right of x_end
			if (x_end < x_start)
				int_swap(&x_start, &x_end);

			for (int x = x_start; x < x_end; x++) {
				// Draw our pixel with the color that comes from the texture
				// Test draw call
				draw_texel(x, y, point_a, point_b, point_c,
					a_uv, b_uv, c_uv,
					texture);
			}
		}
	}
}