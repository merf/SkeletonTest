varying vec3 Normal;
varying vec4 VertexPos;
//varying vec4 RawVertexPos;
varying vec4 LightVec;

varying vec4 Color;

float Lambert(vec3 v1, vec3 v2, float f)
{
	return max(dot(v1, v2), 0.0) * f + (1.0 - f);
}

void main() 
{
	vec4 light_dir = normalize(LightVec);

	float d = Lambert(Normal, light_dir.xyz, 0.5);

	vec4 color = vec4(d) * Color;

	gl_FragColor = vec4(color);
}