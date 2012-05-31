#version 130

out vec3 Normal;
out vec4 VertexPos;
out vec4 RawVertexPos;
out vec4 LightVec;

out vec4 Color;

in ivec4 BoneIndices;
in vec4 BoneWeights;
in vec4	inVertex;

uniform mat4 BoneTransforms[10];
uniform vec4 BoneColors[10];


//uniform mat4 ModelMatrix;

void main() 
{
	//Normal = gl_NormalMatrix * gl_Normal;

	int i0 = int(BoneIndices.x);
	int i1 = int(BoneIndices.y);
	int i2 = int(BoneIndices.z);
	int i3 = int(BoneIndices.w);
	
	vec4 vertex_pos = vec4(0);
	Color = vec4(0);
	

/*
	int index = int(BoneWeights[1]);
	vertex_pos += BoneTransforms[index] * inVertex * BoneWeights[0];
	Color += BoneColors[index] * BoneWeights[0];
	*/

	float w = 1.0;

	//for(int i=0; i<1; ++i)
	{
		//int index = BoneIndices[i];

		int index = int(BoneWeights[1]);
		float weight = BoneWeights[0];

		vertex_pos += BoneTransforms[index] * inVertex * weight;
		Color += BoneColors[index] * weight;

		w -= weight;

		index = int(BoneWeights[3]);
		weight = BoneWeights[2];

		vertex_pos += BoneTransforms[index] * inVertex * weight;
		Color += BoneColors[index] * weight;

		w -= weight;
	}

	Color = vec4(BoneWeights.rgb, 1.0);

	vertex_pos += inVertex * w;

	RawVertexPos = vertex_pos;

	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vertex_pos;

	VertexPos = gl_ModelViewMatrix * vertex_pos;

	LightVec = gl_LightSource[0].position - VertexPos;
	

	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
}