#ifndef METHODS_H
#define METHODS_H
#include<Eigen\Sparse>
#include<OpenMesh\Core\IO\MeshIO.hh>
#include<OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>
typedef Eigen::SparseMatrix<float> SpMatf;
void EigenDecomposition(SpMatf &M, int nev, float sigma, Eigen::VectorXf &evalues, Eigen::MatrixXf &evecs);
float isSymmetry(SpMatf &M);


using namespace OpenMesh::Attributes;

struct MyTraits :public OpenMesh::DefaultTraits
{
	VertexAttributes(OpenMesh::Attributes::Normal |
		OpenMesh::Attributes::Color);
	//FaceAttributes(OpenMesh::Attributes::Normal);
	//VertexAttributes(OpenMesh::Attributes::Color);
	HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
};

typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits> BaseMesh;

OpenMesh::Vec3uc convert_color(float& val, float& maxval, float& minval, OpenMesh::Vec3uc& minColor, OpenMesh::Vec3uc& maxColor);


#endif