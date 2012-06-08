#version 130
#extension GL_EXT_gpu_shader4 : enable

out vec3 Normal;
out vec4 VertexPos;
out vec4 RawVertexPos;
out vec4 LightVec;

out vec4 Color;

in vec4 BoneIndices;
in vec4 BoneWeights;
in vec4	inVertex;
in vec3	inNormal;

uniform mat4 BoneTransforms[10];
uniform vec4 BoneColors[10];


//uniform mat4 ModelMatrix;

void main() 
{
//	Normal = gl_NormalMatrix * gl_Normal;

	vec4 normal4 = vec4(gl_Normal, 1.0);
	vec4 final_mormal = vec4(0.0);

	int i0 = int(BoneIndices.x);
	int i1 = int(BoneIndices.y);
	int i2 = int(BoneIndices.z);
	int i3 = int(BoneIndices.w);
	
	vec4 vertex_pos = vec4(0);
	Color = vec4(0);

	mat4 transform = mat4(1.0);
	
	float w = 1.0;

	for(int i=0; i<2; ++i)
	{
		int index = int(BoneIndices[i]);
		float weight = BoneWeights[i];

		vertex_pos += BoneTransforms[index] * inVertex * weight;
		final_mormal += BoneTransforms[index] * normal4 * weight;
		Color += BoneColors[index] * weight;
		w -= weight;
	}

	vertex_pos += inVertex * w;

	RawVertexPos = vertex_pos;

	Normal = inNormal;

	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vertex_pos;

	VertexPos = gl_ModelViewMatrix * vertex_pos;

	LightVec = gl_LightSource[0].position - VertexPos;
	
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
}