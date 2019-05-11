#include"AppMesh.h"

float computeVoroArea(BaseMesh& mesh, BaseMesh::HalfedgeHandle h, float cot1,float cot2) {

	float edgelen1 = mesh.calc_edge_sqr_length(h);
	float edgelen2 = mesh.calc_edge_sqr_length(mesh.next_halfedge_handle(h));	
	float edgelen3 = mesh.calc_edge_sqr_length(mesh.prev_halfedge_handle(h));

	if (edgelen1 + edgelen3 < edgelen2)
		return mesh.calc_sector_area(h) / 2;
	else if ((edgelen1 + edgelen2 < edgelen3) || (edgelen2 + edgelen3 < edgelen1))
		return mesh.calc_sector_area(h) / 4;
	else {
		return (edgelen1*cot2+ edgelen3*cot1) / 8;
	}
	
}

float computeCot(BaseMesh& mesh, BaseMesh::HalfedgeHandle h) {
	OpenMesh::Vec3f in_vec, out_vec;
	mesh.calc_sector_vectors(h, in_vec, out_vec);

	float denom = norm(in_vec)*norm(out_vec);
	assert(denom!= 0);
	float cos = dot(in_vec, out_vec) / denom;
	return cos / sqrt(1 - cos*cos);
	
}

//http://www.openmesh.org/media/Documentations/OpenMesh-7.1-Documentation/a03944.html
//D:/install_package/coding/graphics/OpenMesh-7.1/Documentation/a03944.html
void AppMesh::ComputeWeightedGraphLP(SpMatf &Laplace)
{
	int v_num = mesh_.n_vertices();
	Laplace = SpMatf(v_num, v_num);

	typename BaseMesh::ConstVertexIter vIt(mesh_.vertices_begin()), vEnd(mesh_.vertices_end());
	BaseMesh::VertexOHalfedgeCWIter vohIt;
	int vc, vp;
	float weight, sum_weight, cot_alpha, cot_beta, cot_tmp;
	float* voronoi_area = (float*)malloc(sizeof(float)*v_num);

	for (; vIt != vEnd; ++vIt)
	{
		vc = vIt->idx();//center vertex
		sum_weight = 0;
		voronoi_area[vc] = 0;
		
		for (vohIt = mesh_.voh_cwbegin(*vIt); vohIt != mesh_.voh_cwend(*vIt); ++vohIt)
		{
			vp = mesh_.to_vertex_handle(*vohIt).idx();//oppisite vertex
		
			cot_alpha = computeCot(mesh_, mesh_.next_halfedge_handle(*vohIt));
			cot_beta = computeCot(mesh_, mesh_.next_halfedge_handle(mesh_.opposite_halfedge_handle(*vohIt)));
			cot_tmp = computeCot(mesh_, *vohIt);
			weight = (cot_alpha + cot_beta) / 2;
			
			sum_weight += weight;
			voronoi_area[vc] +=computeVoroArea(mesh_, *vohIt, cot_tmp, cot_alpha);

			Laplace.insert(vc, vp) = -weight;
		}
		Laplace.insert(vc, vc) = sum_weight;
	}
	for (int k = 0; k<Laplace.outerSize(); ++k)
		for (SpMatf::InnerIterator it(Laplace, k); it; ++it)
		{
			Laplace.coeffRef(it.row(), it.col()) = it.value() / voronoi_area[it.row()];
		}
	free(voronoi_area);
	Laplace.makeCompressed();
}

Vec3uc convert_color(float& val, float& maxval, float& minval, Vec3uc& minColor,Vec3uc& maxColor) {
	//float t=(val - minval) / (maxval - minval);
	//return minColor + t*(maxColor - minColor);

	if (abs(maxval - minval) < 1e-6) {
		return minColor;
	}
	float t;
	if (2 * val <= maxval + minval) {
		t = 2 * (val - minval) / (maxval - minval);
		return minColor + t*(Vec3uc(255, 255, 255) - minColor);
	}
	else {
		t = 2 * (maxval - val) / (maxval - minval);
		return maxColor + t*(Vec3uc(255, 255, 255) - maxColor);
	}
}

void AppMesh::ColorMapping(float* data, float minval, float maxval) {
	std::cout << maxval - minval << std::endl;
	Vec3uc maxColor(255, 0, 0), minColor(0, 0, 255);
	typename BaseMesh::ConstVertexIter vIt(mesh_.vertices_begin()), vEnd(mesh_.vertices_end());

	for (; vIt != vEnd; ++vIt) {
		mesh_.set_color(*vIt, convert_color(data[vIt->idx()], minval, maxval, minColor, maxColor));
	}
}

BaseMesh::Point sphericalPoint(double theta,double phi) {
	double x = sin(theta)*cos(phi);
	double y=sin(theta)*sin(phi);
	double z=cos(theta);
	return BaseMesh::Point(x, y, z);
}

//The mesh vertex corresponding to spherical_point(¦È,¦Õ)
BaseMesh::VertexHandle sphericalPointHandle(double theta, double phi) {
//
}
//Spherical Function
void AppMesh::ConstSphere(BaseMesh& VBasisMesh) {
	
	VBasisMesh.request_face_normals();
	VBasisMesh.request_vertex_normals();
	 
	int n1 = 10, n2 = 10, vId;
	const int t = n1*(n2-1)+2;
	BaseMesh::VertexHandle vhandle[92];
	vhandle[0] = VBasisMesh.add_vertex(BaseMesh::Point(0.0, 0.0, 1.0));
	for (int i = 1; i < n1; ++i) {//(i*pi/n1, j*2pi/n2)
		for (int j = 0; j < n2; ++j) {
			vId=(i - 1)*n2 + j + 1;
			vhandle[vId] = VBasisMesh.add_vertex(sphericalPoint(i*M_PI / n1, j * 2 * M_PI / n2));
		}	
	}
	vhandle[n1*n2-n2+1]= VBasisMesh.add_vertex(BaseMesh::Point(0.0, 0.0, -1.0));

	std::vector<BaseMesh::VertexHandle> face_vhandles;
	for (int j = 0; j < n2-1; ++j) {
		face_vhandles.clear();
		face_vhandles.push_back(vhandle[0]); face_vhandles.push_back(vhandle[j+1]); face_vhandles.push_back(vhandle[j+2]);
		VBasisMesh.add_face(face_vhandles);
	}
	face_vhandles.clear();
	face_vhandles.push_back(vhandle[0]); face_vhandles.push_back(vhandle[n2]); face_vhandles.push_back(vhandle[1]);
	VBasisMesh.add_face(face_vhandles);
	for (int i = 1; i < n1-1; ++i) {
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
	face_vhandles.push_back(vhandle[n1*n2 - 2 * n2 + 1]);face_vhandles.push_back(vhandle[n1*n2 - n2]); 

	VBasisMesh.add_face(face_vhandles);
	VBasisMesh.update_face_normals();
	VBasisMesh.update_vertex_normals();

}

void AppMesh::sphericalPara(BaseMesh& VBasisMesh) {

	BaseMesh::Point centroid(0, 0, 0);

	BaseMesh::VertexIter vIt, v_End(mesh_.vertices_end());
	for (vIt = mesh_.vertices_begin(); vIt != v_End; ++vIt) {
		centroid += mesh_.point(*vIt);
	}
	centroid /= mesh_.n_vertices();


	//std::function<double(double, double)> SphericalFunction;
}