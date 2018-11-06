layout (location = 0) in vec3 vertex_Position;
layout (location = 3) in vec4 vertex_Color;

// Vertex program include test
void main()
{
	gl_Position = vec4(vertex_Position, 1);
}
