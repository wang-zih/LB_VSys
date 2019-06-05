#include"AppMesh.h"

//openmesh的delete 会删除对应的边和面，这里会存在问题
int AppMesh::RemoveDumplicateVertex() {

	std::vector<VertexHandle> perm(mesh_.n_vertices());
	std::map<VertexHandle, VertexHandle> mp;
	int k = 0, deleted = 0;
	BaseMesh::VertexIter vIt, vEnd(mesh_.vertices_end());	
	for (vIt = mesh_.vertices_begin(); vIt != vEnd; ++vIt,++k) {
		perm[k]=*vIt;
	}

	std::sort(perm.begin(), perm.end(),[=](VertexHandle a, VertexHandle b) {return mesh_.point(a)<mesh_.point(b); });

	int j = 0, i = 1;
	mp[perm[0]] = perm[0];
	for (; i != mesh_.n_vertices();)
	{
		if (mesh_.point(perm[i]) == mesh_.point(perm[j])) {//dumplicate
			mp[perm[i]] = perm[j];//perm[i] 重复的元素为perm[j]
			//mesh_.delete_vertex(perm[i]);//openmesh的delete 会删除对应的边和面，这里会存在问题
			mesh_.status(perm[i]).set_deleted(true);
			++i;		
			deleted++;
		}
		else {//new vertex
			j = i;
			i++;
		}
	}

	//====================update 
	/**/for (BaseMesh::HalfedgeIter heIt = mesh_.halfedges_begin(); heIt != mesh_.halfedges_end(); ++heIt)
	{
		VertexHandle vhd = mesh_.to_vertex_handle(*heIt);

		if (mp.find(vhd) != mp.end())
			mesh_.set_vertex_handle(*heIt, mp[vhd]);
	}
	return deleted;
	
}

class TripleElement {
	
public:
	TripleElement(unsigned int v0, unsigned int v1, unsigned int v2, FaceHandle _fh) {
		v[0] = v0; v[1] = v1; v[2] = v2;
		fh = _fh;
		//std::sort(v, v + 3);
	}

	bool operator < (const TripleElement &p) const
	{
		return (v[2] != p.v[2]) ? (v[2]<p.v[2]) :
			(v[1] != p.v[1]) ? (v[1]<p.v[1]) :
			(v[0]<p.v[0]);
	}

	bool operator == (const TripleElement &p) const
	{
		if ((v[0] == p.v[0]) && (v[1] == p.v[1]) && (v[2] == p.v[2])) 
			return true;
		return false;
	}

	unsigned int v[3];
	FaceHandle fh;
};

int AppMesh::RemoveDumplicateFace()
{

	std::vector<TripleElement> faceList;
	BaseMesh::FaceIter fIt, fEnd(mesh_.faces_end());
	BaseMesh::FaceVertexIter fvIt;

	int i;
	for (fIt = mesh_.faces_begin(); fIt != fEnd; ++fIt) {
		TripleElement trie(0, 0, 0, *fIt);
		i = 0;
		for (auto fv : mesh_.fv_range(*fIt)) {
			trie.v[i] = fv.idx();
			i++;
		}
		std::sort(trie.v, trie.v + 3);
		faceList.push_back(trie);
	}
	std::sort(faceList.begin(), faceList.end());

	int dumplicated = 0;
	for (int i = 0; i < mesh_.n_faces()-1; ++i)
	{
		if (faceList[i]==faceList[i+1]) {
			//mesh_.delete_face(faceList[i].fh);
			mesh_.status(faceList[i].fh).set_deleted(true);
			dumplicated++;
		}
	}
	return dumplicated;
}

void AppMesh::RemoveDumplicateItems()
{
	if (!mesh_.has_vertex_status())
		mesh_.request_vertex_status();
	if (!mesh_.has_face_status())
		mesh_.request_face_status();
	if (!mesh_.has_edge_status())
		mesh_.request_edge_status();

	int deletedVt=RemoveDumplicateVertex();
	std::cout << deletedVt << std::endl;

	int deletedF=RemoveDumplicateFace();
	std::cout << deletedF << std::endl;

	mesh_.garbage_collection();
	if(mesh_.has_vertex_status())
		mesh_.release_vertex_status();
	if (mesh_.has_face_status())
		mesh_.release_face_status();

	std::cout << "obj size: " << mesh_.n_vertices() << " " << mesh_.n_faces() << " " << mesh_.n_edges() << std::endl;
}

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

//the cot value of opposite angle of this halfedge 对角的cot值
float computeCot(BaseMesh& mesh, BaseMesh::HalfedgeHandle h) {

	OpenMesh::Vec3f in_vec, out_vec;
	mesh.calc_sector_vectors(mesh.next_halfedge_handle(h), in_vec, out_vec);
	return dot(in_vec, out_vec) / norm(cross(in_vec, out_vec));
}


float computeCot2(BaseMesh& mesh, BaseMesh::HalfedgeHandle h) {
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
		
			//cot_alpha = computeCot2(mesh_, mesh_.next_halfedge_handle(*vohIt));
			//cot_beta = computeCot2(mesh_, mesh_.next_halfedge_handle(mesh_.opposite_halfedge_handle(*vohIt)));
			//cot_tmp = computeCot2(mesh_, *vohIt);
			cot_alpha = computeCot(mesh_, *vohIt);
			cot_beta = computeCot(mesh_, mesh_.opposite_halfedge_handle(*vohIt));						
			cot_tmp= computeCot(mesh_, mesh_.prev_halfedge_handle(*vohIt));
			weight = (cot_alpha + cot_beta) / 2;
			
			sum_weight += weight;
			voronoi_area[vc] +=computeVoroArea(mesh_, *vohIt, cot_tmp, cot_alpha);
			if (isnan<float>(weight+cot_tmp) || isinf<float>(weight + cot_tmp)) {
				std::cout << "error LP weight:" << vc << " " << vp << std::endl;
			}
			Laplace.insert(vc, vp) = -weight;
		}
		//std::cout << "voro area:" << vc << " " << voronoi_area[vc] << std::endl;
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


void AppMesh::ColorMapping(float* data, float minval, float maxval) {
	std::cout << maxval - minval << std::endl;
	OpenMesh::Vec3uc maxColor(255, 0, 0), minColor(0, 0, 255);
	typename BaseMesh::ConstVertexIter vIt(mesh_.vertices_begin()), vEnd(mesh_.vertices_end());

	for (; vIt != vEnd; ++vIt) {
		mesh_.set_color(*vIt, convert_color(data[vIt->idx()], minval, maxval, minColor, maxColor));
	}
}

//computing conformal structures of surfaces
void AppMesh::sphereConformalMapping(VPropHandleT<Vec3f> &newPts) {
	//parameter
	double steplen = 1e-2;
	const double threshold1 = 1e-3, threshold2 = 1e-3;


	OpenMesh::VPropHandleT<Vec3f> Laplace;
	mesh_.add_property(newPts);
	//mesh_.property(newPts).set_persistent(true);
	mesh_.add_property(Laplace);

	BaseMesh::VertexIter vIt, vEnd(mesh_.vertices_end());
	BaseMesh::VertexVertexIter vvIt;
	BaseMesh::HalfedgeIter heIt,heEnd(mesh_.halfedges_end()); 

	//Gauss Map
	for (vIt= mesh_.vertices_begin(); vIt != vEnd; ++vIt) {
		mesh_.property(newPts, *vIt)=mesh_.normal(*vIt);
	}
	
	//The harmonic energy E(h) of Gauss Map
	double oldE=0,newE=0;
	Vec3f tmp;
	for (vIt = mesh_.vertices_begin(); vIt != vEnd; ++vIt)
		for (vvIt = mesh_.vv_begin(*vIt); vvIt.is_valid(); ++vvIt) {
			tmp = mesh_.property(newPts, *vIt) - mesh_.property(newPts, *vvIt);
			oldE += sqrnorm(tmp);
		}


	Vec3f Pt;
//===============================spherical barycentric embeddings
	while (1) {
		//update Laplace
		for (vIt = mesh_.vertices_begin(); vIt != vEnd; ++vIt) {
			mesh_.property(Laplace, *vIt).vectorize(0.0f);
			for (vvIt = mesh_.vv_begin(*vIt); vvIt.is_valid(); ++vvIt) {
				mesh_.property(Laplace, *vIt) += mesh_.property(newPts, *vIt) - mesh_.property(newPts, *vvIt);
			}
		}

		//Steepest Descendent 
		for (vIt = mesh_.vertices_begin(); vIt != vEnd; ++vIt) {
			Vec3f* newPoints = &mesh_.property(newPts, *vIt);
			Pt = (mesh_.property(Laplace, *vIt)- *newPoints*dot(mesh_.property(Laplace, *vIt),*newPoints))*steplen;
			*newPoints -= Pt;
			(*newPoints).normalize();
		}


		//The new harmonic energy E(h)=newE
		newE = 0;
		for (vIt = mesh_.vertices_begin(); vIt != vEnd; ++vIt)
			for (vvIt = mesh_.vv_begin(*vIt); vvIt.is_valid(); ++vvIt) {
				tmp = mesh_.property(newPts, *vIt) - mesh_.property(newPts, *vvIt);
				newE += sqrnorm(tmp);
			}
				
		//update
		//std::cout << "Energy difference: " <<newE-oldE<< std::endl;
		if (fabs(newE - oldE) < threshold1)
			break;
		
		oldE = newE;
	}
//============================ spherical conformal mapping  ====================================================
	std::cout << "Begin spherical conformal mapping" << std::endl;
	BaseMesh::VertexFaceIter vfIt; 
	BaseMesh::VertexOHalfedgeIter voheIt;
	Vec3f center;
	//using the spherical barycentric embeding as the initial
	oldE = newE;
	while (1) {
		for (vIt = mesh_.vertices_begin(); vIt != vEnd; ++vIt) {
			mesh_.property(Laplace, *vIt).vectorize(0.0f);
			for (vvIt = mesh_.vv_begin(*vIt); vvIt.is_valid(); ++vvIt) {
				mesh_.property(Laplace, *vIt) += mesh_.property(newPts, *vIt) - mesh_.property(newPts, *vvIt);
			}
		}
		//Steepest Descendent
		for (vIt = mesh_.vertices_begin(); vIt != vEnd; ++vIt) {
			Vec3f *newPoints = &mesh_.property(newPts, *vIt);
			Pt = (mesh_.property(Laplace, *vIt) - *newPoints*dot(mesh_.property(Laplace, *vIt), *newPoints))*steplen;
			*newPoints -= Pt;
			(*newPoints).normalize();
		}
		//harmonic mass Center
		center.vectorize(0.0f);
		
		float area = 0;
		for (vIt = mesh_.vertices_begin(); vIt != vEnd; ++vIt) {
			area = 0;
			for(voheIt=mesh_.voh_begin(*vIt);voheIt.is_valid();++voheIt){
				area += mesh_.calc_sector_area(*voheIt);
			}
			center += mesh_.point(*vIt)*area;
		}
		center /= mesh_.n_vertices();

		for (vIt = mesh_.vertices_begin(); vIt != vEnd; ++vIt) {
			mesh_.property(newPts, *vIt) -= center;
			mesh_.property(newPts, *vIt).normalize();
		}
		//harmonic energy
		newE = 0;
		for (vIt = mesh_.vertices_begin(); vIt != vEnd; ++vIt)
			for (vvIt = mesh_.vv_begin(*vIt); vvIt.is_valid(); ++vvIt) {
				tmp = mesh_.property(newPts, *vIt) - mesh_.property(newPts, *vvIt);
				newE += sqrnorm(tmp) * 1;
			}

		if (fabs(newE - oldE) < threshold2)
			break;
		oldE = newE;
	}
	
	mesh_.remove_property(Laplace);
	//==============================

	/*for (vIt = mesh_.vertices_begin(); vIt != vEnd; ++vIt)
	{
		mesh_.point(*vIt) = mesh_.property(newPts, *vIt);
	}*/

}


//p在fh中的面积坐标
float barycentricValue(BaseMesh &mesh, VPropHandleT<Vec3f> &newPts, float* vals, FaceHandle &fh,Vec3f p)
{
	float val[3];
	BaseMesh::ConstFaceVertexIter fv_it(mesh.cfv_iter(fh));
	const Vec3f& p0(mesh.property(newPts, (*fv_it))); val[0] = vals[fv_it->idx()]; ++fv_it;
	const Vec3f& p1(mesh.property(newPts, (*fv_it))); val[1] = vals[fv_it->idx()]; ++fv_it;
	const Vec3f& p2(mesh.property(newPts, (*fv_it))); val[2] = vals[fv_it->idx()];
	Vec3f p1p0(p1 - p0), p2p0(p2-p0);

	Vec3f facen= mesh.calc_face_normal(fh);
	float area2 = cross(p1p0, p2p0).norm();
	
	float w0 = dot(facen, cross(p1 - p, p2 - p)) / area2;
	float w1 = dot(facen, cross(p2 - p, p0 - p)) / area2;
	float w2 = dot(facen, cross(p0 - p, p1 - p)) / area2;

	
	//std::cout << "weight " << w0 + w1 + w2 << std::endl;
	return w0*val[0] + w1*val[1] + w2*val[2];
}

Vec3f calc_conformalCentroid(BaseMesh& mesh, VPropHandleT<Vec3f> &newPts, FaceHandle &fh)
{
	Vec3f pt;
	vectorize(pt, 0);
	float valence = 0.0f;
	for (BaseMesh::FaceVertexIter fvIt = mesh.fv_begin(fh); fvIt.is_valid(); ++fvIt, valence += 1.0f)
	{
		pt += mesh.property(newPts, fvIt.handle());
	}
	pt /= valence;
	return pt;
}

//blended intrinc mapping 看GMDS扩散是否有帮助
//Color mapping of the sphere by Inverse of the conformal mapping
void AppMesh::sphericalPara(VPropHandleT<Vec3f> &newPts, BaseMesh& VBasisMesh, float* vals, Eigen::VectorXf& sphereVal) {
	
	BaseMesh::VertexIter vIt;
	BaseMesh::FaceIter fIt;

	FPropHandleT<Vec3f> centroids;
	mesh_.add_property(centroids);

	for (fIt = mesh_.faces_begin(); fIt != mesh_.faces_end(); ++fIt) {//after mapping, the region of each tri
		mesh_.property(centroids, *fIt) = calc_conformalCentroid(mesh_, newPts, fIt.handle());//mesh_.calc_face_centroid(*fIt);
	}
	
	
	float min = 100000,tmp;
	BaseMesh::FaceHandle facebel;
	for (vIt = VBasisMesh.vertices_begin(); vIt != VBasisMesh.vertices_end(); ++vIt) {//the corresponding tri of each vertex
		min = 100000;
		for (fIt = mesh_.faces_begin(); fIt != mesh_.faces_end(); ++fIt) {
			tmp=sqrnorm(VBasisMesh.point(*vIt) - mesh_.property(centroids, *fIt));
			if (tmp < min){min = tmp; facebel = fIt.handle();}
		}
		//std::cout << "min " << min << " "<<facebel<<std::endl;
		sphereVal(vIt->idx())= barycentricValue(mesh_, newPts, vals, facebel, VBasisMesh.point(*vIt));
	}
	mesh_.remove_property(centroids);
	//mesh_.remove_property(newPts);
}
