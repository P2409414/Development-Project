#version 330 core

layout(points) in ;
layout(line_strip, max_vertices = 7) out ;

void main()
{
	gl_Position = gl_in[0].gl_Position ;
	EmitVertex() ;
	gl_Position = gl_in[0].gl_Position + vec4(0.0f, 10.0f, 0.0f, 0.0f);
	EmitVertex() ;
	gl_Position = gl_in[0].gl_Position + vec4(10.0f, 10.0f, 0.0f, 0.0f);
	EmitVertex() ;
	gl_Position = gl_in[0].gl_Position + vec4(10.0f, 0.0f, 0.0f, 0.0f);
	EmitVertex() ;
	gl_Position = gl_in[0].gl_Position + vec4(0.0f, 0.0f, 0.0f, 0.0f);
	EmitVertex() ;
	gl_Position = gl_in[0].gl_Position + vec4(0.0f, -10.0f, 0.0f, 0.0f);
	EmitVertex() ;
	gl_Position = gl_in[0].gl_Position + vec4(10.0f, 0.0f, 0.0f, 0.0f);
	EmitVertex() ;
	EndPrimitive() ;
}