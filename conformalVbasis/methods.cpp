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

//0<=m<=l
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
}

double sphericalFunc(int l, int m, double phi, double theta) {
	double Const = sqrt((2 * l + 1) *fac(l - m) / (4 * M_PI*fac(l + m)));
	std::complex<double> i(0, 1);

	Const*exp(m*phi*i);


	return 0;
}
