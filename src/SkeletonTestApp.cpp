#include "Resources.h"

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Camera.h"
#include "cinder/ImageIo.h"

#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;

using std::vector;

const int MAX_BONES = 3;
const int VERTS_PER_BONE = 2;
const int RADIUS_VERTS = 3;

Vec4f light_pos;
ColorA light_color = ColorA::white();
ColorA diffuse_color = ColorA::white();

std::vector<Vec4f> verts;
std::vector<Vec3f> normals;
std::vector<GLushort> indices;
std::vector<Vec4f> bone_indices;
std::vector<Vec4f> bone_weights;


Vec4f bone_colors[MAX_BONES];
Matrix44f bone_transforms_ms[MAX_BONES];
Matrix44f bone_transforms_ls[MAX_BONES];
Matrix44f ref_pose[MAX_BONES];
Matrix44f inv_ref_pose[MAX_BONES];

GLuint vertex_buffer_id;
GLuint bone_indices_buffer_id;
GLuint bone_weights_buffer_id;

float smoothstep(float t)
{
	return t * t * (3.0f - 2.0f * t);
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
		int i0 = (i)%MAX_BONES;
		int i1 = (i+1)%MAX_BONES;

		float y_f_bone = i/(float)(MAX_BONES-1) - 0.5f;
		float y_bone = y_f_bone * 10;
		Matrix44f m = Matrix44f::createTranslation(Vec3f(0,y_bone,0));
		ref_pose[i] = m;
		m.invert();
		inv_ref_pose[i] = m;
		
		for(int j=0; j<VERTS_PER_BONE; ++j)
		{
			float f = j/(float)VERTS_PER_BONE;
			float w1 = smoothstep(f);
			float w0 = 1.0f-w1;

			//w0 = 1.0f;
			//w1 = 0.0f;

			float y_f = (i+f)/(MAX_BONES-1) - 0.5f;
			float y = y_f * 10;

			for(int k=0; k<RADIUS_VERTS; ++k)
			{
				float angle0 = k/(float)RADIUS_VERTS*M_PI*2.0f;
				float angle1 = (k+1)/(float)RADIUS_VERTS*M_PI*2.0f;

				verts.push_back(Vec4f(cos(angle0), y, -sin(angle0), 1));
				//verts.push_back(Vec4f(cos(angle1), y, -sin(angle1), 1));

				//TODO - normals
				normals.push_back(Vec3f(0, 0, 1));
				//normals.push_back(Vec3f(0, 0, 1));

				bone_indices.push_back(Vec4f(i0, i1, 0, 0));
				//bone_indices.push_back(Vec4f(i0, i1, 0, 0));

				bone_weights.push_back(Vec4f(w0, w1, 0.0f, 0.0f));
				//bone_weights.push_back(Vec4f(w0, w1, 0.0f, 0.0f));
			}
		}
	}

	int num_verts = verts.size();
	
	for(int i=0; i<MAX_BONES-1; ++i)
	{
		for(int j=0; j<VERTS_PER_BONE; ++j)
		{
			for(int k=0; k<RADIUS_VERTS; ++k)
			{
				//indices.push_back(((k + j * RADIUS_VERTS + i * VERTS_PER_BONE * RADIUS_VERTS) + 0) % num_verts);
				//indices.push_back(((k + j * RADIUS_VERTS + i * VERTS_PER_BONE * RADIUS_VERTS) + 1) % num_verts);
				//indices.push_back(((k + (j+1) * RADIUS_VERTS + i * VERTS_PER_BONE * RADIUS_VERTS) + 0) % num_verts);

				indices.push_back( (k + j * RADIUS_VERTS + i * VERTS_PER_BONE * RADIUS_VERTS) + 1);
				indices.push_back( ((k + (j+1) * RADIUS_VERTS + i * VERTS_PER_BONE * RADIUS_VERTS) + 1) % (RADIUS_VERTS*2*(j+1)));
				indices.push_back( ((k + (j+1) * RADIUS_VERTS) + i * VERTS_PER_BONE * RADIUS_VERTS) + 0);


				//indices.push_back((k + j * RADIUS_VERTS + i * VERTS_PER_BONE * RADIUS_VERTS) * 2 + 1);
				//indices.push_back((1 + k + j * RADIUS_VERTS + i * VERTS_PER_BONE * RADIUS_VERTS) * 2 + 1);
				//indices.push_back((1 + k + j * RADIUS_VERTS + i * VERTS_PER_BONE * RADIUS_VERTS) * 2 + 0);
			}
		}
	}
	
	//m_VboMesh = gl::VboMesh(m_TriMesh);
	
	glGenBuffers(1, &vertex_buffer_id);
	glGenBuffers(1, &bone_indices_buffer_id);
	glGenBuffers(1, &bone_weights_buffer_id);
	
	int num = verts.size();
	
	//upload data into GL buffers
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, num * sizeof(Vec4f), verts.data()->ptr(), GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, bone_indices_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, num * sizeof(Vec4f), bone_indices.data()->ptr(), GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, bone_weights_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, num * sizeof(Vec4f), bone_weights.data()->ptr(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	gl::enableDepthRead();
	gl::enableDepthWrite();

	for(int i=0; i<MAX_BONES; ++i)
	{
		bone_colors[i] = Vec4f(hsvToRGB(Vec3f(ci::randFloat(0.0f, 1.0f), 1.0f, 1.0f)));
	}

	
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
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

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
	
	for(int i=0; i<MAX_BONES; ++i)
	{
		float f = i/(float)MAX_BONES;
		
		//bone_transforms_ms[i] = Matrix44f::createRotation(Vec3f(0, 0, 0.2f * (i%2==0 ? 1 : -1) * sin(getElapsedSeconds()))) * ref_pose[i];
		bone_transforms_ms[i] = Matrix44f::createTranslation(Vec3f((i%2==0 ? 1 : -1) * sin(getElapsedSeconds() * 5), 0, 0)) * ref_pose[i];
		bone_transforms_ls[i] = inv_ref_pose[i] * bone_transforms_ms[i];

		//bone_transforms[i] = Matrix44f::identity();
	}
	
	//gl::rotate(Vec3f(0, getElapsedSeconds() * 60, 0));
	
	m_Shader.bind();
	
	m_Shader.uniform("BoneTransforms", bone_transforms_ls, MAX_BONES);
	m_Shader.uniform("BoneColors", &bone_colors[0], MAX_BONES);
	
	//Vertices
	GLint vertex_loc = m_Shader.getAttribLocation("inVertex");
	if(vertex_loc != -1)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
		glEnableVertexAttribArray(vertex_loc);
		glVertexAttribPointer(vertex_loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	}

	//glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
	//glEnableClientState(GL_VERTEX_ARRAY);
	//glVertexPointer(3, GL_FLOAT, sizeof(Vec3f), 0);
	
	//Bone Indices
	GLint bone_indices_loc = m_Shader.getAttribLocation("BoneIndices");
	if(bone_indices_loc != -1)
	{
		glBindBuffer(GL_ARRAY_BUFFER, bone_indices_buffer_id);
		glEnableVertexAttribArray(bone_indices_loc);
		glVertexAttribPointer(bone_indices_loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
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

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	
	m_Shader.unbind();

	glDisable(GL_LIGHTING);
	
	for(int i=0; i<MAX_BONES; ++i)
	{
		glColor4f(bone_colors[i].x, bone_colors[i].y, bone_colors[i].z, bone_colors[i].w);

		Matrix44f m = bone_transforms_ms[i];
		//Matrix44f m = bone_transforms[i];
		Vec3f bone_pos = Vec3f::zero();
		bone_pos = m.transformPoint(bone_pos);
		gl::drawSphere(bone_pos, 0.25f);
	}



	gl::popModelView();

	//glEnable(GL_COLOR_MATERIAL);
	//gl::color(light_color);
	//gl::drawSphere(light_pos.xyz(), 0.1f);

	gl::popMatrices();
}


CINDER_APP_BASIC( SkeltonTestApp, RendererGl )
