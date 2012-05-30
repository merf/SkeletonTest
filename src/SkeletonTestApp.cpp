#include "Resources.h"

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Camera.h"
#include "cinder/ImageIo.h"

using namespace ci;
using namespace ci::app;

using std::vector;

const int MAX_BONES = 10;
const int VERTS_PER_BONE = 10;

Vec4f light_pos;
ColorA light_color = ColorA::white();
ColorA diffuse_color = ColorA::white();

std::vector<Vec3f> verts;
std::vector<Vec3f> normals;
std::vector<GLushort> indices;
std::vector<Vec4i> bone_indices;
std::vector<Vec4f> bone_weights;

GLuint vertex_buffer_id;
GLuint bone_indices_buffer_id;
GLuint bone_weights_buffer_id;

float smoothstep(float t)
{
	return t * t * (3.0 - 2.0 * t);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class SkeltonTestApp : public AppBasic 
{
public:
	void setup();
	void update();
	void draw();
	
	TriMesh			m_TriMesh;
	gl::VboMesh		m_VboMesh;
	CameraPersp		m_Cam;
	
	gl::GlslProg	m_Shader;
};

//////////////////////////////////////////////////////////////////////////
void SkeltonTestApp::setup()
{
	try 
	{
		m_Shader = gl::GlslProg(loadAsset("Diffuse.vs"), loadAsset("Diffuse.fs"));
	}
	
	catch( gl::GlslProgCompileExc &exc ) 
	{
		std::cout << "Shader compile error: " << std::endl;
		std::cout << exc.what();
		quit();
	}
	catch( ... ) {
		std::cout << "Unable to load shader" << std::endl;
	}
	
	
	m_Cam.lookAt(Vec3f(0,0,25), Vec3f::zero());
	
	for(int i=0; i<MAX_BONES; ++i)
	{		
		int i0 = i;
		int i1 = (i+1)%MAX_BONES;
		
		for(int j=0; j<VERTS_PER_BONE; ++j)
		{
			float w1 = smoothstep(j/(float)VERTS_PER_BONE);
			float w0 = 1.0f-w1;
						
			w0 = 1.0f;
			w1 = 0.0f;
			float y = i-MAX_BONES/2 + w1;
			verts.push_back(Vec3f(-1, y, 0));
			verts.push_back(Vec3f(1, y, 0));
			
			normals.push_back(Vec3f(0, 0, 1));
			normals.push_back(Vec3f(0, 0, 1));
			
			bone_indices.push_back(Vec4i(i0, i1, i0, i0));
			bone_indices.push_back(Vec4i(i0, i1, i0, i0));
			
			bone_weights.push_back(Vec4f(w0, w1, 0.0f, 0.0f));
			bone_weights.push_back(Vec4f(w0, w1, 0.0f, 0.0f));
		}
	}
	
	for(int i=0; i<MAX_BONES-1; ++i)
	{
		for(int j=0; j<VERTS_PER_BONE; ++j)
		{
			indices.push_back((j + i * VERTS_PER_BONE) * 2 + 0);
			indices.push_back((j + i * VERTS_PER_BONE) * 2 + 1);
			indices.push_back((j + i * VERTS_PER_BONE) * 2 + 2);
			
			indices.push_back((j + i * VERTS_PER_BONE) * 2 + 2);
			indices.push_back((j + i * VERTS_PER_BONE) * 2 + 1);
			indices.push_back((j + i * VERTS_PER_BONE) * 2 + 3);
		}
	}
	
	//m_VboMesh = gl::VboMesh(m_TriMesh);
	
	glGenBuffers(1, &vertex_buffer_id);
	glGenBuffers(1, &bone_indices_buffer_id);
	glGenBuffers(1, &bone_weights_buffer_id);
	
	int num = verts.size();
	
	//upload data into GL buffers
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, num * sizeof(Vec3f), verts.data()->ptr(), GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, bone_indices_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, num * sizeof(Vec4i), bone_indices.data()->ptr(), GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, bone_weights_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, num * sizeof(Vec4f), bone_weights.data()->ptr(), GL_STATIC_DRAW);
	
	gl::enableDepthRead();
	gl::enableDepthWrite();
	
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
}

//////////////////////////////////////////////////////////////////////////
void SkeltonTestApp::update()
{
}

//////////////////////////////////////////////////////////////////////////
void SkeltonTestApp::draw()
{
	
	Vec4f light_pos = Vec4f(1 * sin(getElapsedSeconds()), -1 * cos(getElapsedSeconds()), 3.0f, 1.0f);
	
	gl::clear();
	
	gl::pushMatrices();
	
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_color.ptr());	
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_color.ptr());
	
	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	
	
	
	
	gl::setMatrices(m_Cam);
	
	gl::pushModelView();
	
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos.ptr() );
	
	Vec4f bone_colors[MAX_BONES];
	Matrix44f bone_transforms[MAX_BONES];
	
	for(int i=0; i<MAX_BONES; ++i)
	{
		float f = i/(float)MAX_BONES;
		
		bone_transforms[i] = ci::Matrix44f::createTranslation(Vec3f(sin(getElapsedSeconds() + f * M_PI * 2.0f), 0, 0));
		bone_transforms[i] = ci::Matrix44f::createRotation(Vec3f(0,1,0), getElapsedSeconds() + f * M_PI * 2.0f);
		
		bone_colors[i] = Vec4f(hsvToRGB(Vec3f(f, 1.0f, 1.0f)));
	}
	
	//bone_transforms[0] = ci::Matrix44f::createTranslation(Vec3f(0, sin(getElapsedSeconds()), 0));
	//bone_transforms[1] = ci::Matrix44f::createTranslation(Vec3f(sin(getElapsedSeconds()), 0, 0));
	
	//gl::rotate(Vec3f(0, getElapsedSeconds() * 60, 0));
	
	m_Shader.bind();
	
	m_Shader.uniform("BoneTransforms", bone_transforms, MAX_BONES);
	m_Shader.uniform("BoneColors", &bone_colors[0], MAX_BONES);
	
	//Vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(Vec3f), 0);
	
	//Bone Indices
	GLint bone_indices_loc = m_Shader.getAttribLocation("BoneIndices");
	if(bone_indices_loc != -1)
	{
		glBindBuffer(GL_ARRAY_BUFFER, bone_indices_buffer_id);
		glEnableVertexAttribArray(bone_indices_loc);
		glVertexAttribPointer(bone_indices_loc, 4, GL_INT, GL_FALSE, 0, 0);
	}
	
	//Bone Weights
	GLint bone_weights_loc = m_Shader.getAttribLocation("BoneWeights");
	if(bone_weights_loc != -1)
	{
		glBindBuffer(GL_ARRAY_BUFFER, bone_weights_buffer_id);
		glEnableVertexAttribArray(bone_weights_loc);
		glVertexAttribPointer(bone_weights_loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	}
	
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, indices.data());
	
	
	glDisableClientState(GL_VERTEX_ARRAY);
	
	if(bone_indices_loc != -1)
	{
		glDisableVertexAttribArray(bone_indices_loc);
	}
	
	if(bone_weights_loc != -1)
	{
		glDisableVertexAttribArray(bone_weights_loc);
	}
	
	
	
	
	gl::popModelView();
	
	m_Shader.unbind();
	
	
	glDisable(GL_LIGHT0);
	
	glEnable(GL_COLOR_MATERIAL);
	gl::color(light_color);
	gl::drawSphere(light_pos.xyz(), 0.1f);
	
	gl::popMatrices();
}


CINDER_APP_BASIC( SkeltonTestApp, RendererGl )
