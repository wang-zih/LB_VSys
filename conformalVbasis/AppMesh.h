#ifndef APPMESH_H
#define APPMESH_H

#include<QtWidgets\QWidget>
#include<QtWidgets\QMessageBox>
#include<QtWidgets\QFileDialog>
#include<QtCore\QString>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include<OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>
#include<Eigen\Sparse>
#include"MeshViewerWidgetT.h"
//#include"BasisViewerWidge.h"

using namespace OpenMesh::Attributes;

struct MyTraits :public OpenMesh::DefaultTraits
{
	//typedef OpenMesh::Vec3d Point;//double-value points
	VertexAttributes(OpenMesh::Attributes::Normal |
					 OpenMesh::Attributes::Color);
	//FaceAttributes(OpenMesh::Attributes::Normal);
	//VertexAttributes(OpenMesh::Attributes::Color);
	HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
};

typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits> BaseMesh;

class AppMesh :public MeshViewerWidgetT<BaseMesh>
{
	Q_OBJECT
public:
	typedef Eigen::SparseMatrix<float> SpMatf;

	explicit AppMesh(QWidget* parent = 0) : MeshViewerWidgetT<BaseMesh>(parent) {}
	OpenMesh::IO::Options& options() { return _options; }
	void setOptions(const OpenMesh::IO::Options& opts) { _options = opts; }
	QString modelName() { return fileName; }
	void setFilename(QString &name) { fileName = name; }

	void open_mesh_gui(QString fname)
	{
		OpenMesh::Utils::Timer t;
		t.start();
		if (fname.isEmpty() || !open_mesh(fname.toLocal8Bit(), _options))
		{
			QString msg = "Cannot read mesh from file:\n '";
			msg += fname;
			msg += "'";
			QMessageBox::critical(NULL, windowTitle(), msg);
		}
		t.stop();
		std::cout << "Loaded mesh in ~" << t.as_string() << std::endl;
	}

	void ComputeWeightedGraphLP(SpMatf& Laplace);

	void ColorMapping(float* data, float minval, float maxval);

	void ConstSphere(BaseMesh& basisobject,int N);
	void sphericalPara(BaseMesh&);
	
	//friend class BasisViewerWidget;
private:
	OpenMesh::IO::Options _options;
	QString fileName;
};

#endif