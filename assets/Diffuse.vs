varying vec3 Normal;
varying vec4 VertexPos;
varying vec4 RawVertexPos;
varying vec4 LightVec;

//uniform mat4 ModelMatrix;

void main() 
{
	Normal = gl_NormalMatrix * gl_Normal;

	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;

	VertexPos = gl_ModelViewMatrix * gl_Vertex;

	LightVec = gl_LightSource[0].position - VertexPos;

	RawVertexPos = gl_Vertex;

	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
}