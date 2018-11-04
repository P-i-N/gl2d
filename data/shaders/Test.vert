layout(location = 0) vec3 vertex_Position;
layout(location = 3) vec4 vertex_Color;

// Vertex program include test
void main()
{
	gl_Position = vec4(vertex_Position, 1);
}
