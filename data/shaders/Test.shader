#vertex

layout (location = 0) in vec3 vertex_Position;
layout (location = 3) in vec4 vertex_Color;

layout (std140, binding = 0) uniform FrameData
{
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
};

out vec4 color;

void main()
{
	gl_Position = ProjectionMatrix * ViewMatrix * vec4(vertex_Position, 1);
	color = vertex_Color;
}

#fragment

in vec4 color;

out vec4 out_Color;

void main()
{
	out_Color = vec4(color);
}
