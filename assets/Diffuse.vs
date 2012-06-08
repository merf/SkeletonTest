
varying vec3 Normal;
out vec4 VertexPos;
//varying vec4 VertexPos;
varying vec4 RawVertexPos;
varying vec4 LightVec;

in vec4	inVertex;

uniform mat4 ModelMatrix;

void main() 
{
	Normal = gl_NormalMatrix * gl_Normal;

	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * inVertex;

	VertexPos = gl_ModelViewMatrix * inVertex;

	vec4 light_pos = gl_ModelViewMatrix * gl_LightSource[0].position;
	//light_pos = gl_LightSource[0].position;

	LightVec = gl_LightSource[0].position - VertexPos;

	RawVertexPos = gl_Vertex;

	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
}