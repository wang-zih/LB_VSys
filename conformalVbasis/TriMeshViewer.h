#ifndef TRIMESHVIEWERWIDGET_H
#define TRIMESHVIEWERWIDGET_H

#include<QtWidgets\QWidget>
#include<QtWidgets\QMessageBox>
#include<QtWidgets\QFileDialog>
#include<QtCore\QString>
#include"MeshViewerWidgetT.h"
#include <OpenMesh/Tools/Utils/Timer.hh>
//#include<OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>
#include"methods.h"

class TriMeshViewer:public MeshViewerWidgetT<BaseMesh>
{
	Q_OBJECT
public:
	explicit TriMeshViewer(QWidget* parent = 0) : MeshViewerWidgetT<BaseMesh>(parent){}
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