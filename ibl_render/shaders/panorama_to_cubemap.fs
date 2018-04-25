#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform sampler2D panorama;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.x, v.y), asin(v.z));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(WorldPos));
    vec3 color = texture(panorama, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}
