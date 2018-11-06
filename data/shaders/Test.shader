#vertex

layout (location = 0) in vec3 vertex_Position;
layout (location = 3) in vec4 vertex_Color;

out vec4 color;

// Komentik
void main()
{
	gl_Position = vec4(vertex_Position, 1);
	color = vertex_Color;
}

#fragment

in vec4 color;

out vec4 out_Color;

void main()
{
	out_Color = vec4(color);
}
