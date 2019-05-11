#ifndef TRIMESHVIEWERWIDGET_H
#define TRIMESHVIEWERWIDGET_H

#include<QtWidgets\QWidget>
#include<QtWidgets\QMessageBox>
#include<QtWidgets\QFileDialog>
#include<QtCore\QString>
#include"MeshViewerWidgetT.h"
#include <OpenMesh/Tools/Utils/Timer.hh>
#include<OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>

using namespace OpenMesh::Attributes;

struct MyTraits2 :public OpenMesh::DefaultTraits
{
	//typedef OpenMesh::Vec3d Point;//double-value points
	//VertexAttributes(OpenMesh::Attributes::Normal |
	//				 OpenMesh::Attributes::Color);
	//FaceAttributes(OpenMesh::Attributes::Normal);
	HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
};

typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits2> MyMesh;

class TriMeshViewer:public MeshViewerWidgetT<MyMesh>
{
	Q_OBJECT
public:
	explicit TriMeshViewer(QWidget* parent = 0) : MeshViewerWidgetT<MyMesh>(parent){}
	OpenMesh::IO::Options& options() { return _options; }
	void setOptions(const OpenMesh::IO::Options& opts) { _options = opts; }

	
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
	
private:
	OpenMesh::IO::Options _options;
};

#endif