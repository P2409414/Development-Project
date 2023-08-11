#version 330 core

layout(triangles) in ;
layout(line_strip, max_vertices = 2) out ;

uniform vec2 normLength;

void main()
{
	//for(int i = 0 ; i < 2 ; i++)
	//{
		//gl_Position = gl_in[i].gl_Position;
		//EmitVertex();
		//gl_Position = (gl_in[i].gl_Position+vec4(normal_in[i]*normLength, 0.0));
		//EmitVertex();
		//EndPrimitive();
	//}
}