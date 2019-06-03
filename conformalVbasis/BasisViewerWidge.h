#ifndef BASISVIEWERWIDGE_H
#define BASISVIEWERWIDGE_H
#include<OpenMesh\Core\IO\MeshIO.hh>
#include<OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>
#include"QGLViewerWidget.h"
using namespace OpenMesh::Attributes;

struct MyTraitsBasis :public OpenMesh::DefaultTraits
{
	//FaceAttributes(OpenMesh::Attributes::Normal);
	HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
};
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraitsBasis> MeshBasis;

class BasisViewerWidget :public QGLViewerWidget
{
public:
	BasisViewerWidget(QWidget* _parent = 0)
		:QGLViewerWidget(_parent){}
	OpenMesh::IO::Options& options() { return opt_; }
	void setOptions(const OpenMesh::IO::Options& opts) { opt_ = opts; }

	void glVertex(const typename MeshBasis::VertexHandle _vh)
	{
		glVertex3fv(&mesh_.point(_vh)[0]);
	}

	void glVertex(const typename MeshBasis::Point& _p)
	{
		glVertex3fv(&_p[0]);
	}
	//friend void AppMesh::sphericalPara(BaseMesh&);
private:
	OpenMesh::IO::Options  opt_;

	MeshBasis              mesh_;
};




#endif