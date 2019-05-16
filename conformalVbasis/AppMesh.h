#ifndef APPMESH_H
#define APPMESH_H

#include<QtWidgets\QWidget>
#include<QtWidgets\QMessageBox>
#include<QtWidgets\QFileDialog>
#include<QtCore\QString>
#include <OpenMesh/Tools/Utils/Timer.hh>

#include<Eigen\Sparse>
#include"MeshViewerWidgetT.h"
#include"methods.h"


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
	
private:
	OpenMesh::IO::Options _options;
	QString fileName;
};

#endif