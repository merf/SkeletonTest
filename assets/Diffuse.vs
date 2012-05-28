varying vec3 Normal;
varying vec4 VertexPos;
varying vec4 RawVertexPos;
varying vec4 LightVec;

varying vec4 Color;

attribute vec4 BoneIndices;
attribute vec4 InVertexPos;

uniform mat4 BoneTransforms;


//uniform mat4 ModelMatrix;

void main() 
{
	Normal = gl_NormalMatrix * gl_Normal;

	int i0 = int(BoneIndices.x);
	int i1 = int(BoneIndices.y);
	int i2 = int(BoneIndices.z);
	int i3 = int(BoneIndices.w);
	
	vec4 vertex_pos = gl_Vertex;
	
	if(BoneIndices.x == 0.0)
	{
		vertex_pos = BoneTransforms * gl_Vertex;
		//vertex_pos += vec4(0,1,0,0);
		Color = vec4(1,0,0,1);
	}
	else
	{
		Color = vec4(1);
	}

	RawVertexPos = vertex_pos;

	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vertex_pos;

	VertexPos = gl_ModelViewMatrix * vertex_pos;

	LightVec = gl_LightSource[0].position - VertexPos;
	

	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
}