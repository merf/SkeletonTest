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

Vec4f light_pos;
ColorA light_color = ColorA::white();
ColorA diffuse_color = ColorA::white();

std::vector<Vec3f> verts;
std::vector<Vec3f> normals;
std::vector<GLint> indices;
std::vector<Vec4i> bone_indices;

GLuint vertex_buffer_id;
GLuint bone_indices_buffer_id;


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
	
	
	m_Cam.lookAt(Vec3f(0,0,10), Vec3f::zero());
	
	verts.push_back(Vec3f(-1, -1, 0));
	verts.push_back(Vec3f(1, -1, 0));
	verts.push_back(Vec3f(-1, 1, 0));
	verts.push_back(Vec3f(1, 1, 0));
	
	normals.push_back(Vec3f(0, 0, 1));
	normals.push_back(Vec3f(0, 0, 1));
	normals.push_back(Vec3f(0, 0, 1));
	normals.push_back(Vec3f(0, 0, 1));
	
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	
	indices.push_back(2);
	indices.push_back(1);
	indices.push_back(3);
	
	bone_indices.push_back(Vec4i(0,0,0,0));
	bone_indices.push_back(Vec4i(1,1,1,1));
	bone_indices.push_back(Vec4i(2,2,2,2));
	bone_indices.push_back(Vec4i(3,3,3,3));
	
	//m_VboMesh = gl::VboMesh(m_TriMesh);
	
	glGenBuffers(1, &vertex_buffer_id);
	glGenBuffers(1, &bone_indices_buffer_id);
	
	//upload data into GL buffers
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vec3f), verts.data()->ptr(), GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, bone_indices_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vec4i), bone_indices.data()->ptr(), GL_STATIC_DRAW);
	
	
	gl::enableDepthRead();
	gl::enableDepthWrite();
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
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
	
	Matrix44f bone_transforms[10];
	
	for(int i=0; i<10; ++i)
	{
		bone_transforms[i] = ci::Matrix44f::identity();
	}
	
	bone_transforms[0] = ci::Matrix44f::createTranslation(Vec3f(0, sin(getElapsedSeconds()), 0));
	
	//gl::rotate(Vec3f(0, getElapsedSeconds() * 60, 0));
	
	m_Shader.bind();
	
	m_Shader.uniform("BoneTransforms", bone_transforms[0]);
	
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(Vec3f), 0);
	
	GLint bone_indices_id = m_Shader.getAttribLocation("BoneIndices");
													 
	glBindBuffer(GL_ARRAY_BUFFER, bone_indices_buffer_id);
	glEnableVertexAttribArray(bone_indices_id);
	
	glVertexAttribPointer(bone_indices_id, 4, GL_INT, GL_FALSE, 0, 0);
	
	
	//gl::drawSphere(Vec3f::zero(), 1.0f);
	//gl::draw(m_VboMesh);
		
	//GLint bone_indices_id = m_Shader.getAttribLocation("BoneIndices");
	//GLint verts_id = m_Shader.getAttribLocation("InVertexPos");
	
	//float* p_verts = verts.data()->ptr();
	
	//glVertexAttrib4iv(bone_indices_id, bone_indices.data()->ptr());
	//glVertexAttrib3fv(verts_id, p_verts);
	
	//glEnableVertexAttribArray(bone_indices_id);
	//glEnableVertexAttribArray(verts_id);
	
	glDrawArrays(GL_TRIANGLES, 0, 3);
	
	//glDisableVertexAttribArray(bone_indices_id);
	//glDisableVertexAttribArray(verts_id);
	
	//glBindAttribLocation(m_Shader.getHandle(), 0, <#const GLchar *name#>)
	//glBindAttribLocation(m_Shader, 0,"myVertexPos");
	
	//glVertex3fv(0, verts.data());
	//glVertexAttrib4iv(0, bone_indices.data());
	
	//glDrawArrays(, <#GLint first#>, <#GLsizei count#>)
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableVertexAttribArray(bone_indices_id);
	
	gl::popModelView();
	
	m_Shader.unbind();

	
	glDisable(GL_LIGHT0);
	
	glEnable(GL_COLOR_MATERIAL);
	gl::color(light_color);
	gl::drawSphere(light_pos.xyz(), 0.1f);
	
	gl::popMatrices();
}


CINDER_APP_BASIC( SkeltonTestApp, RendererGl )
