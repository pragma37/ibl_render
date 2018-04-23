#version 330 core
in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 camPos;
uniform vec3 color;


void main()
{	
	float d = dot(normalize(camPos - WorldPos), Normal);
    FragColor = vec4(color * d, 1.0);
}
