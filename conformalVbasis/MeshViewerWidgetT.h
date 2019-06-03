#ifndef MESHVIEWERWIDGETT_H
#define MESHVIEWERWIDGETT_H

#include<OpenMesh\Core\IO\MeshIO.hh>
#include<OpenMesh\Tools\Utils\StripifierT.hh>
#include"QGLViewerWidget.h"


template <typename M>
class MeshViewerWidgetT :public QGLViewerWidget
{
public:
	typedef M  Mesh;

public:
	MeshViewerWidgetT(QWidget* _parent = 0)
		:QGLViewerWidget(_parent),
		use_color_(true),
		show_vnormals_(false),
		show_fnormals_(false)
	{
		//add_draw_mode("Points");
		//add_draw_mode("Hidden-Line");
	}

	~MeshViewerWidgetT() {};

	bool open_mesh(const char* _filename, OpenMesh::IO::Options _opt);
	void set_mesh(Mesh& outermesh) { mesh_ = outermesh; };
	Mesh& get_mesh() { return mesh_; };

protected:

	OpenMesh::IO::Options  opt_; 

	Mesh                   mesh_;
	bool                   use_color_;
	bool                   show_vnormals_;
	bool                   show_fnormals_;
	float                  normal_scale_;
	OpenMesh::FPropHandleT< typename Mesh::Point > fp_normal_base_;
	/// inherited drawing method
	 void draw_scene(const std::string& _draw_mode);
	 void draw_openmesh(const std::string& _draw_mode);

	void glVertex(const typename Mesh::VertexHandle _vh)
	{
		glVertex3fv(&mesh_.point(_vh)[0]);
	}

	void glVertex(const typename Mesh::Point& _p)
	{
		glVertex3fv(&_p[0]);
	}

	void glNormal(const typename Mesh::VertexHandle _vh)
	{
		glNormal3fv(&mesh_.normal(_vh)[0]);
	}

	void glTexCoord(const typename Mesh::VertexHandle _vh)
	{
		glTexCoord2fv(&mesh_.texcoord(_vh)[0]);
	}

	void glColor(const typename Mesh::VertexHandle _vh)
	{
		glColor3ubv(&mesh_.color(_vh)[0]);
	}

	// face properties

	void glNormal(const typename Mesh::FaceHandle _fh)
	{
		glNormal3fv(&mesh_.normal(_fh)[0]);
	}

	void glColor(const typename Mesh::FaceHandle _fh)
	{
		glColor3ubv(&mesh_.color(_fh)[0]);
	}

	void glMaterial(const typename Mesh::FaceHandle _fh,
		int _f = GL_FRONT_AND_BACK, int _m = GL_DIFFUSE)
	{
		OpenMesh::Vec3f c = OpenMesh::color_cast<OpenMesh::Vec3f>(mesh_.color(_fh));
		OpenMesh::Vec4f m(c[0], c[1], c[2], 1.0f);

		glMaterialfv(_f, _m, &m[0]);
	}
};

//============================== Ä£°åÀàµÄ===============================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(MESHVIEWERWIDGET_CPP)
#  define MESHVIEWERWIDGET_TEMPLATES
#  include "MeshViewerWidgetT.cpp"
#endif
//=============================================================================
#endif