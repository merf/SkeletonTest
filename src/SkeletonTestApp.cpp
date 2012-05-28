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
	
	for(int i=0; i<2; ++i)
	{
		m_TriMesh.appendVertex(Vec3f(-1, -1, 0));
		m_TriMesh.appendVertex(Vec3f(1, -1, 0));
		m_TriMesh.appendVertex(Vec3f(-1, 1, 0));
		m_TriMesh.appendVertex(Vec3f(1, 1, 0));
	}
	
	for(int i=0; i<2; ++i)
	{
		m_TriMesh.appendNormal(Vec3f(0, 0, i==0 ? 1 : -1));
		m_TriMesh.appendNormal(Vec3f(0, 0, i==0 ? 1 : -1));
		m_TriMesh.appendNormal(Vec3f(0, 0, i==0 ? 1 : -1));
		m_TriMesh.appendNormal(Vec3f(0, 0, i==0 ? 1 : -1));
	}
	
	m_TriMesh.appendTriangle(0, 1, 2);
	m_TriMesh.appendTriangle(2, 1, 3);
	
	m_TriMesh.appendTriangle(2, 1, 0);
	m_TriMesh.appendTriangle(3, 1, 2);
	
	m_VboMesh = gl::VboMesh(m_TriMesh);
	
	gl::enableDepthRead();
	gl::enableDepthWrite();
}

//////////////////////////////////////////////////////////////////////////
void SkeltonTestApp::update()
{
}

//////////////////////////////////////////////////////////////////////////
void SkeltonTestApp::draw()
{
	glEnable(GL_COLOR_MATERIAL);
	
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
	
	

	gl::rotate(Vec3f(0, getElapsedSeconds() * 60, 0));
	
	m_Shader.bind();
	
	gl::drawSphere(Vec3f::zero(), 1.0f);
	
	gl::popModelView();
	
	m_Shader.unbind();
	
	
	glDisable(GL_LIGHT0);
	
	gl::color(light_color);
	gl::drawSphere(light_pos.xyz(), 0.1f);

	gl::popMatrices();
}


CINDER_APP_BASIC( SkeltonTestApp, RendererGl )
