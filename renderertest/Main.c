#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include "Array.h"
#include "Camera.h"
#include "Display.h"
#include "Light.h"
#include "Mesh.h"
#include "Matrix.h"
#include "Texture.h"
#include "Triangle.h"
#include "upng.h"
#include "Vector.h"

#define MAX_TRIANGLES_PER_MESH 10000
triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];

int num_triangles_to_render = 0;

float delta_time = 0.0;

mat4_t proj_matrix; 
mat4_t view_matrix;
mat4_t world_matrix;

bool is_running = false;
int previous_frame_time = 0;

 void setup(void) {
	 render_method = RENDER_WIRE;
	 cull_method = CULL_BACKFACE;
	 
	 // Allocate memory for color buffer in bytes
	 color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);
	 z_buffer = (float*)malloc(sizeof(float) * window_width * window_height);
	 // color_buffer[(window_width * 10) + 20] = 0xFFFF0000;
	 if (!color_buffer) {
		 fprintf(stderr, "Color Buffer Allocation failed");
	 }
	 // Create an SDL texture that is used to display the color buffer
	 color_buffer_texture = SDL_CreateTexture(
		 renderer,
		 SDL_PIXELFORMAT_RGBA32,
		 SDL_TEXTUREACCESS_STREAMING,
		 window_width,
		 window_height
	 );

	 if(!color_buffer_texture){
		 fprintf(stderr, "Error Creating SDL texture");
	 }

	 // Initialize perspective projection matrix
	 float fov = M_PI/3.0; // Same as 180/3 or 60degrees
	 float aspect_ratio = (float)window_height / (float)window_width;
	 float znear = 0.1;
	 float zfar = 100.0;
	 proj_matrix = mat4_make_perspective(fov, aspect_ratio, znear, zfar);

	 //load_cube_mesh_data();
	 //load_obj_file_data("./assets/cube.obj");
	 //load_obj_file_data("./assets/f22.obj");
	 //load_obj_file_data("./assets/crab.obj");
	 //load_obj_file_data("./assets/f117.obj");
	 load_obj_file_data("./assets/efa.obj");
	 //load_obj_file_data("./assets/drone.obj");

	 //load_png_texture_data("./assets/cube.png");
	 //load_png_texture_data("./assets/f22.png");
	 //load_png_texture_data("./assets/f117.png");
	 load_png_texture_data("./assets/efa.png");
	 //load_png_texture_data("./assets/drone.png");
	 //load_png_texture_data("./assets/crab.png");
	 
 }

 void process_input(void) {
	 SDL_Event event;
	 SDL_PollEvent(&event);

	 switch (event.type) {
		case SDL_QUIT:
			is_running = false;
			break;
		case SDL_KEYDOWN:
			if(event.key.keysym.sym == SDLK_ESCAPE)
				is_running = false;
			if (event.key.keysym.sym == SDLK_1)
				render_method = RENDER_WIRE_VERTEX;
			if (event.key.keysym.sym == SDLK_2)
				render_method = RENDER_WIRE;
			if (event.key.keysym.sym == SDLK_3)
				render_method = RENDER_FILL_TRIANGLE;
			if (event.key.keysym.sym == SDLK_4)
				render_method = RENDER_FILL_TRIANGLE_WIRE;
			if (event.key.keysym.sym == SDLK_5)
				render_method = RENDER_TEXTURED;
			if (event.key.keysym.sym == SDLK_6)
				render_method = RENDER_TEXTURED_WIRED;
			if (event.key.keysym.sym == SDLK_c)
				cull_method = CULL_BACKFACE;
			if (event.key.keysym.sym == SDLK_v)
				cull_method = CULL_NONE;
			if (event.key.keysym.sym == SDLK_UP)
				camera.position.y += 3.0 * delta_time;
			if (event.key.keysym.sym == SDLK_DOWN)
				camera.position.y -= 3.0 * delta_time;
			if (event.key.keysym.sym == SDLK_a)
				camera.yaw += 1.0 * delta_time;
			if (event.key.keysym.sym == SDLK_d)
				camera.yaw -= 1.0 * delta_time;
			if (event.key.keysym.sym == SDLK_w) {
				camera.forward_velocity = vec3_multiply_s(camera.direction, 5.0 * delta_time);
				camera.position = vec3_add(camera.position, camera.forward_velocity);
			}
			if (event.key.keysym.sym == SDLK_s) {
				camera.forward_velocity = vec3_multiply_s(camera.direction, 5.0 * delta_time);
				camera.position = vec3_sub(camera.position, camera.forward_velocity);
			}
			break;
	 }
 }

 void update(void) {
	 int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
	 if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
		SDL_Delay(time_to_wait);

	 //factor converted to seconds to update our game objects
	 delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0;

	 previous_frame_time = SDL_GetTicks();

	 num_triangles_to_render = 0;
	 
	 mesh.rotation.x += (float)0.5 * delta_time;
	 /*mesh.rotation.y += (float)0.6 * delta_time;
	 mesh.rotation.z += (float)0.6 * delta_time;*/
	 // Translate the vertex away from the camera

	 mesh.translation.z = 4.0;

	 // Compute the new camera rotation and tranlation for the FPS camera
	 vec3_t target = { 0, 0, 1 };
	 mat4_t camera_yaw_rotation = mat4_make_rotation_y(camera.yaw);
	 camera.direction =  vec3_from_vec4(mat4_mul_vec4(camera_yaw_rotation, vec4_from_vec3(target)));

	 // offset camera position in the direction where the camera is pointing at
	 target = vec3_add(camera.position, camera.direction);
	 vec3_t up_direction = { 0, 1, 0 };

	 // Create the view matrix
	 view_matrix = mat4_look_at(camera.position, target, up_direction);

	 // Create scale, translation, and rotation matrix 
	 mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
	 mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
	 mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
	 mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);
	 mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);

	 int num_faces = array_length(mesh.faces);
	 for (int i = 0; i < num_faces; i++) {
		 face_t mesh_face = mesh.faces[i];
		 
		 vec3_t face_vertices[3];
		 face_vertices[0] = mesh.vertices[mesh_face.a];
		 face_vertices[1] = mesh.vertices[mesh_face.b];
		 face_vertices[2] = mesh.vertices[mesh_face.c];

		 vec4_t transformed_vertices[3];

		 // Loop all 3 vertices of current face and apply transformations
		 for (int j = 0; j < 3; j++) {
			 vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);
			 
			 // Create a world matrix combining scale, rotation and translation matrices
			 world_matrix = mat4_identity();

			 // Multiply all matrices to load the world_matrix ---------------    [t]*[r]*[s]*vertex
			 world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
			 world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
			 world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
			 world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
			 world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

			 // Multiply the world matrix by the original vector to get transformation
			 transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

			 // Multiply the view matrix by the original vector to transform the scene to camera space
			 transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

			 transformed_vertices[j] = transformed_vertex;
		 }

		 vec3_t vec_a = vec3_from_vec4(transformed_vertices[0]);
		 vec3_t vec_b = vec3_from_vec4(transformed_vertices[1]);
		 vec3_t vec_c = vec3_from_vec4(transformed_vertices[2]);

		 vec3_t a_b = vec3_sub(vec_b, vec_a);
		 vec3_t a_c = vec3_sub(vec_c, vec_a);
		 vec3_normalize_ref(&a_b);
		 vec3_normalize_ref(&a_c);

		 // After transformations we want to do a back face culling check
		 vec3_t face_normal = vec3_cross(a_b, a_c);
		 vec3_normalize_ref(&face_normal);

		 vec3_t origin = { 0, 0, 0 };
		 vec3_t camera_ray = vec3_sub(origin, vec_a);

		 float camera_dot_normal = vec3_dot(face_normal, camera_ray);

		 if (cull_method == CULL_BACKFACE) {

			 if (camera_dot_normal < 0) {
				 continue;
			 }			
		 }

		 vec4_t projected_points[3];

		 // After cull check, loop through vertices to perform projection
		 for (int j = 0; j < 3; j++) {
			 
			 // Project the current vertex
			 projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);
			 
			 // Scale into the view
			 projected_points[j].x *= (window_width / 2.0);
			 projected_points[j].y *= (window_height / 2.0);
			 
			 // invert y value to account for flipped screen y coordinates
			 projected_points[j].y *= -1;

			 // translate projected point to middle of screen
			 projected_points[j].x += (window_width / 2.0);
			 projected_points[j].y += (window_height / 2.0);
		 }

		 uint32_t face_color = 0xFFFF0000;

		 // Calculate color shade intensity based off of face normal and light direction
		 vec3_t normalized_light = vec3_normalize_copy(light.direction);
		 float face_dot_light = -vec3_dot(face_normal, normalized_light);
		 face_color = light_apply_intensity(mesh_face.color, face_dot_light);

		 triangle_t projected_triangle = {
			.points = {
				{ projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w },
				{ projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w },
				{ projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w },
			},
			.texcoords = {
				{ mesh_face.a_uv.u, mesh_face.a_uv.v },
				{ mesh_face.b_uv.u, mesh_face.b_uv.v },
				{ mesh_face.c_uv.u, mesh_face.c_uv.v }
			 },
			.color = face_color
		 };

		 if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH) {
			 // Save projected triangle to array of triangles to render
			triangles_to_render[num_triangles_to_render] = projected_triangle;
			num_triangles_to_render++;
		 }
	 }
 }

 void render(void) {
	 // Draws grid in background
	 draw_grid();

	 for (int i = 0; i < num_triangles_to_render; i++) {
		 triangle_t triangle = triangles_to_render[i];

		 // draw filled triangle
		 if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE) {
			 draw_filled_triangle(
				 triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w,
				 triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w,
				 triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w,
				 triangle.color
			 );
		 }
		 //Draw textured triangle
		 if (render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRED) {
			 draw_textured_triangle(
				 triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v,
				 triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v,
				 triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v,
				mesh_texture
			 );
		 }

		 // draw wireframe
		 if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE || render_method == RENDER_TEXTURED_WIRED) {
			 draw_triangle(
				 triangle.points[0].x, triangle.points[0].y,
				 triangle.points[1].x, triangle.points[1].y,
				 triangle.points[2].x, triangle.points[2].y,
				 0xFFFF3333
			 );
		 }

		 //draw vertices
		 if (render_method == RENDER_WIRE_VERTEX) {
			 draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFF0000FF);
			 draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFF0000FF);
			 draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFF0000FF);
		 }
	 }

	 render_color_buffer();

	 clear_color_buffer(0xFF000000);
	 clear_z_buffer();

	 SDL_RenderPresent(renderer);
 }

 void free_resources(void) {
	 free(color_buffer);
	 free(z_buffer);
	 array_free(mesh.vertices);
	 array_free(mesh.faces);
	 upng_free(png_texture);
 }

int main(int argc, char* args[]) {
	
	is_running = initialize_window();
	
	setup();

	while (is_running) {
		process_input();
		update();
		render();
	}
	destroy_window();
	free_resources();
	return 0;
}