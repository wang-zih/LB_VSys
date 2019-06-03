#include"methods.h"
//#include<SymEigsShiftSolver.h>
#include<GenEigsRealShiftSolver.h>
#include<MatOp\SparseGenRealShiftSolve.h>
#include<iostream>
#include<complex>

using namespace Spectra;

void EigenDecomposition(SpMatf &M, int nev, float sigma, Eigen::VectorXf &evalues, Eigen::MatrixXf &evecs) {

	int ncv = 12 * nev;

	SparseGenRealShiftSolve<float> op(M);
	GenEigsRealShiftSolver<float, LARGEST_REAL, SparseGenRealShiftSolve<float>>
		eigs(&op,nev,ncv,sigma);

	eigs.init();
	int nconv = eigs.compute();// maxit = 300 to reduce running time for failed cases
	int niter = eigs.num_iterations();
	int nops = eigs.num_operations();

	if (eigs.info() != SUCCESSFUL)
	{
		std::cout <<"FAILED on this test"<<std::endl;
		std::cout << "nconv = " << nconv << std::endl;
		std::cout << "niter = " << niter << std::endl;
		std::cout << "nops  = " << nops << std::endl;
		return;
	}

	evalues=eigs.eigenvalues().real();
	evecs = eigs.eigenvectors().real();

	Eigen::MatrixXf resid = M * evecs - evecs * evalues.asDiagonal();
	const double err = resid.array().abs().maxCoeff();
	std::cout << "||AU - UD||_inf = " <<err<< std::endl;
}

float isSymmetry(SpMatf& M) {
	return (M - (SpMatf)(M.transpose())).cwiseAbs().sum();
}

OpenMesh::Vec3uc convert_color(float& val, float& maxval, float& minval, OpenMesh::Vec3uc& minColor, OpenMesh::Vec3uc& maxColor) {

	if (abs(maxval - minval) < 1e-6) {
		return minColor;
	}

	float t = (val - minval) / (maxval - minval);
	OpenMesh::Vec3uc middColor(255, 255, 255);

	if (t < 0.5)
		return minColor + 2 * t*(middColor - minColor);
	else if (t >= 0.5)
		return maxColor + 2 * (1 - t)*(middColor - maxColor);

}


//(x,y,z)-centroid   ->(r,phi,theta)
//range: theta in (0,pi);  phi in (0,2pi)
void ToSphericalCoordinate(Eigen::Vector3f& dir, double& r, double& phi, double& theta) {
	r = dir.norm();
	theta = acosf(dir[2] / r);    //  cos(theta)=z/r
	phi = atan2(dir[1], dir[0]);//  tan(phi)=y/x
}

//(r,phi,theta)-> (x,y,z)-centroid
Eigen::Vector3f ToVector(double& r, double& phi, double& theta) {
	return Eigen::Vector3f(r*sin(theta)*cos(phi), r*sin(theta)*sin(phi), r*cos(theta));
}

int fac(int n) {
	if (n == 1) { return 1; }
	else {
		return n*fac(n - 1);
	}
}

/*
Input: 
n: number of degree l and order m;
x: value of the spherical func.
Output: 
lower triangle matrix.
each row indicate one degree l; 
each column indicate one order m.
*/
Eigen::MatrixXd Plm(int n,double x)
{
	Eigen::MatrixXd P(n, n);
	const double y = sqrt(1 - x*x);

	int l, m;

	//diagonal recurrences
	for (l = 1; l < n; ++l)
		P(l, l) = -(2*l-1)*y*P(l - 1, l - 1);

	//triangular recurrences
	for (m = 1; m < n; ++m) {
		P(m + 1, m) = x*(2 * m + m)*P(m, m);

		for (l = m + 2; l < n; ++l)
			P(l, m) = (x*(2*l-1)*P(l - 1, m) - (l+m-1)*P(l - 2, m))/(l-m);
	}
	return P;
}

double sphericalFunc(int l, int m, double phi, double theta) {
	double Const = sqrt((2 * l + 1) *fac(l - m) / (4 * M_PI*fac(l + m)));
	std::complex<double> i(0, 1);

	Const*exp(m*phi*i);


	return 0;
}


BaseMesh::Point sphericalPoint(double theta, double phi) {
	double x = sin(theta)*cos(phi);
	double y = sin(theta)*sin(phi);
	double z = cos(theta);
	return BaseMesh::Point(x, y, z);
}

// The mesh vertex corresponding to spherical_point(¦È, ¦Õ)
//BaseMesh::VertexHandle sphericalPointHandle(double theta, double phi) {
//
//}
//Spherical Function
//vId=(i - 1)*n2 + j + 1;
void ConstSphere(BaseMesh& VBasisMesh, int N, bool cnstTopo) {

	VBasisMesh.request_face_normals();
	VBasisMesh.request_face_colors();
	VBasisMesh.request_vertex_normals();

	int n1 = N, n2 = N, vId;
	const int t = n1*(n2 - 1) + 2;
	BaseMesh::VertexHandle *vhandle = new BaseMesh::VertexHandle[t];
	vhandle[0] = VBasisMesh.add_vertex(BaseMesh::Point(0.0, 0.0, 1.0));
	for (int i = 1; i < n1; ++i) {//(i*pi/n1, j*2pi/n2)
		for (int j = 0; j < n2; ++j) {
			vId = (i - 1)*n2 + j + 1;
			vhandle[vId] = VBasisMesh.add_vertex(sphericalPoint(i*M_PI / n1, j * 2 * M_PI / n2));
		}
	}
	vhandle[n1*n2 - n2 + 1] = VBasisMesh.add_vertex(BaseMesh::Point(0.0, 0.0, -1.0));

	if (cnstTopo) {
		std::vector<BaseMesh::VertexHandle> face_vhandles;
		for (int j = 0; j < n2 - 1; ++j) {
			face_vhandles.clear();
			face_vhandles.push_back(vhandle[0]); face_vhandles.push_back(vhandle[j + 1]); face_vhandles.push_back(vhandle[j + 2]);
			VBasisMesh.add_face(face_vhandles);
		}
		face_vhandles.clear();
		face_vhandles.push_back(vhandle[0]); face_vhandles.push_back(vhandle[n2]); face_vhandles.push_back(vhandle[1]);
		VBasisMesh.add_face(face_vhandles);
		for (int i = 1; i < n1 - 1; ++i) {
			for (int j = 0; j < n2 - 1; ++j) {
				//(i,j)-(i+1,j)-(i+1,j+1)
				//(i+1,j+1)-(i,j+1)-(i,j)
				face_vhandles.clear();
				face_vhandles.push_back(vhandle[(i - 1)*n2 + j + 1]); face_vhandles.push_back(vhandle[i*n2 + j + 1]);
				face_vhandles.push_back(vhandle[i*n2 + j + 2]);
				VBasisMesh.add_face(face_vhandles);
				face_vhandles.clear();
				face_vhandles.push_back(vhandle[i*n2 + j + 2]); face_vhandles.push_back(vhandle[(i - 1)*n2 + j + 2]);
				face_vhandles.push_back(vhandle[(i - 1)*n2 + j + 1]);
				VBasisMesh.add_face(face_vhandles);
			}
			//j=n2-1 (i,n2-1)-(i+1,n2-1)-(i+1,0)
			//(i+1,0)-(i,0)-(i,n2-1)
			//(i - 1)*n2 + j + 1
			face_vhandles.clear();
			face_vhandles.push_back(vhandle[(i - 1)*n2 + n2]); face_vhandles.push_back(vhandle[i*n2 + n2]);
			face_vhandles.push_back(vhandle[i*n2 + 1]);
			VBasisMesh.add_face(face_vhandles);
			face_vhandles.clear();
			face_vhandles.push_back(vhandle[i*n2 + 1]); face_vhandles.push_back(vhandle[(i - 1)*n2 + 1]);
			face_vhandles.push_back(vhandle[(i - 1)*n2 + n2]);
			VBasisMesh.add_face(face_vhandles);
		}
		for (int j = 0; j < n2 - 1; ++j) {
			//(n1 - 2)*n2 + j + 1
			face_vhandles.clear();
			face_vhandles.push_back(vhandle[n1*n2 - n2 + 1]); face_vhandles.push_back(vhandle[(n1 - 2)*n2 + j + 2]);
			face_vhandles.push_back(vhandle[(n1 - 2)*n2 + j + 1]);
			VBasisMesh.add_face(face_vhandles);
		}
		face_vhandles.clear();
		face_vhandles.push_back(vhandle[n1*n2 - n2 + 1]);
		face_vhandles.push_back(vhandle[n1*n2 - 2 * n2 + 1]); face_vhandles.push_back(vhandle[n1*n2 - n2]);

		VBasisMesh.add_face(face_vhandles);
		VBasisMesh.update_face_normals();
		VBasisMesh.update_vertex_normals();
	}
	delete[] vhandle;
}