#version 330 core

uniform vec2 iResolution;
out vec2 fragCoord;

void main()
{
	vec2 vertices[3] = vec2[3](vec2(-1, -1), vec2(3, -1), vec2(-1, 3)); // define a triangle in clip space
	gl_Position = vec4(vertices[gl_VertexID], 0, 1); // render area will be [-1, 1] x [-1, 1]
	fragCoord = (0.5 * gl_Position.xy + vec2(0.5)) * iResolution; // convert to screen space [0, iResolution.x] x [0, iResolution.y]
}