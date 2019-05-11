#ifndef METHODS_H
#define METHODS_H
#include<Eigen\Sparse>
#include<OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>
typedef Eigen::SparseMatrix<float> SpMatf;
void EigenDecomposition(SpMatf &M, int nev, float sigma, Eigen::VectorXf &evalues, Eigen::MatrixXf &evecs);


#endif