#ifndef MESH_H
#define MESH_H

#include "Texture.h"
#include "Triangle.h"
#include "Vector.h"

#define N_CUBE_VERTICES 8 // 8 vertices per cube
#define N_CUBE_FACES (6 * 2) // 6 cube faces, 2 triangles per face

// Order of vertices matter... A clock-wise orientation determines the front and back faces
extern vec3_t cube_vertices[N_CUBE_VERTICES];
extern face_t cube_faces[N_CUBE_FACES];

/// <summary>
/// Defines a struct for dynamic size meshs, with array of vertices and faces
/// </summary>
typedef struct {
	vec3_t* vertices; // Dynamic array of vertices
	face_t* faces;    // Dynamic array of faces
	vec3_t rotation;  // Rotation with x, y, and z values Euler angles
	vec3_t scale;
	vec3_t translation;
} mesh_t;

extern mesh_t mesh;

void load_cube_mesh_data(void);

// Read contents of the .obj file
// and load the vertices and faces in
// our mesh.vertices and mesh.faces
void load_obj_file_data(char* filename);

#endif // !MESH_H