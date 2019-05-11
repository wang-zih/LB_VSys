#define MESHVIEWERWIDGET_CPP

#include"MeshViewerWidgetT.h"
using namespace OpenMesh;

template <typename M>
bool MeshViewerWidgetT<M>::open_mesh(const char* _filename, OpenMesh::IO::Options _opt)
{
	mesh_.request_face_normals();
	mesh_.request_face_colors();
	mesh_.request_vertex_normals();
	mesh_.request_vertex_colors();
	std::cout << "Loading from file '" << _filename << "'\n";

	if (OpenMesh::IO::read_mesh(mesh_, _filename, _opt))
	{
		opt_ = _opt;
		if(!opt_.check(OpenMesh::IO::Options::FaceNormal))
			mesh_.update_face_normals();
		else
			std::cout << "File provides face normals\n";

		if (!opt_.check(IO::Options::VertexNormal))
			mesh_.update_vertex_normals();
		else
			std::cout << "File provides vertex normals\n";

		// check for possible color information
		if (opt_.check(IO::Options::VertexColor))
		{
			std::cout << "File provides vertex colors\n";
			add_draw_mode("Colored Vertices");
		}
		else
			mesh_.release_vertex_colors();

		if (_opt.check(IO::Options::FaceColor))
		{
			std::cout << "File provides face colors\n";
			add_draw_mode("Solid Colored Faces");
			add_draw_mode("Smooth Colored Faces");
		}
		else
			mesh_.release_face_colors();


		// bounding box
		typename Mesh::ConstVertexIter vIt(mesh_.vertices_begin());
		typename Mesh::ConstVertexIter vEnd(mesh_.vertices_end());

		typedef typename Mesh::Point Point;
		using OpenMesh::Vec3f;

		Vec3f bbMin, bbMax;

		bbMin = bbMax = OpenMesh::vector_cast<Vec3f>(mesh_.point(*vIt));

		for (size_t count = 0; vIt != vEnd; ++vIt, ++count)
		{
			bbMin.minimize(OpenMesh::vector_cast<Vec3f>(mesh_.point(*vIt)));
			bbMax.maximize(OpenMesh::vector_cast<Vec3f>(mesh_.point(*vIt)));
		}

		// set center and radius
		set_scene_pos((bbMin + bbMax)*0.5, (bbMin - bbMax).norm()*0.5);

		// for normal display
		normal_scale_ = (bbMax - bbMin).min()*0.05f;

		// info
		std::clog << mesh_.n_vertices() << " vertices, "
			<< mesh_.n_edges() << " edge, "
			<< mesh_.n_faces() << " faces\n";

		// base point for displaying face normals
		{
			OpenMesh::Utils::Timer t;
			t.start();
			mesh_.add_property(fp_normal_base_);
			typename M::FaceIter f_it = mesh_.faces_begin();
			typename M::FaceVertexIter fv_it;
			for (; f_it != mesh_.faces_end(); ++f_it)
			{
				typename Mesh::Point v(0, 0, 0);
				for (fv_it = mesh_.fv_iter(*f_it); fv_it.is_valid(); ++fv_it)
					v += OpenMesh::vector_cast<typename Mesh::Normal>(mesh_.point(*fv_it));
				v *= 1.0f / 3.0f;
				mesh_.property(fp_normal_base_, *f_it) = v;
			}
			t.stop();
			std::clog << "Computed base point for displaying face normals ["
				<< t.as_string() << "]" << std::endl;
		}

		setWindowTitle(QFileInfo(_filename).fileName());


		return true;
	}
	return false;
}


template <typename M>
void MeshViewerWidgetT<M>::draw_openmesh(const std::string& _draw_mode)
{
	typename Mesh::ConstFaceIter    fIt(mesh_.faces_begin()),
		fEnd(mesh_.faces_end());

	typename Mesh::ConstFaceVertexIter fvIt;

		if (_draw_mode == "Wireframe") // -------------------------------------------
		{
			glBegin(GL_TRIANGLES);
			for (; fIt != fEnd; ++fIt)
			{
				fvIt = mesh_.cfv_iter(*fIt);
				glVertex3fv(&mesh_.point(*fvIt)[0]);
				++fvIt;
				glVertex3fv(&mesh_.point(*fvIt)[0]);
				++fvIt;
				glVertex3fv(&mesh_.point(*fvIt)[0]);
			}
			glEnd();
		}

		else if (_draw_mode == "Solid Flat") // -------------------------------------
		{
			glBegin(GL_TRIANGLES);
			for (; fIt != fEnd; ++fIt)
			{
				glNormal3fv(&mesh_.normal(*fIt)[0]);

				fvIt = mesh_.cfv_iter(*fIt);
				glVertex3fv(&mesh_.point(*fvIt)[0]);
				++fvIt;
				glVertex3fv(&mesh_.point(*fvIt)[0]);
				++fvIt;
				glVertex3fv(&mesh_.point(*fvIt)[0]);
			}
			glEnd();

		}


		else if (_draw_mode == "Solid Smooth") // -----------------------------------
		{
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, mesh_.points());

			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());

			glBegin(GL_TRIANGLES);
			for (; fIt != fEnd; ++fIt)
			{
				fvIt = mesh_.cfv_iter(*fIt);
				glArrayElement(fvIt->idx());
				++fvIt;
				glArrayElement(fvIt->idx());
				++fvIt;
				glArrayElement(fvIt->idx());
			}
			glEnd();

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);

		}

		else if (_draw_mode == "Colored Vertices") // --------------------------------
		{
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, mesh_.points());

			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());

			if (mesh_.has_vertex_colors())
			{
				glEnableClientState(GL_COLOR_ARRAY);
				glColorPointer(3, GL_UNSIGNED_BYTE, 0, mesh_.vertex_colors());
			}

			glBegin(GL_TRIANGLES);
			for (; fIt != fEnd; ++fIt)
			{
				fvIt = mesh_.cfv_iter(*fIt); 
				glArrayElement(fvIt->idx());
				++fvIt;
				glArrayElement(fvIt->idx());
				++fvIt;
				glArrayElement(fvIt->idx());
			}
			glEnd();

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);
		}


		else if (_draw_mode == "Solid Colored Faces") // -----------------------------
		{
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, mesh_.points());

			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());

			glBegin(GL_TRIANGLES);
			for (; fIt != fEnd; ++fIt)
			{
				glColor(*fIt);

				fvIt = mesh_.cfv_iter(*fIt);
				glArrayElement(fvIt->idx());
				++fvIt;
				glArrayElement(fvIt->idx());
				++fvIt;
				glArrayElement(fvIt->idx());
			}
			glEnd();

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
		}


		else if (_draw_mode == "Smooth Colored Faces") // ---------------------------
		{
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, mesh_.points());

			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());

			glBegin(GL_TRIANGLES);
			for (; fIt != fEnd; ++fIt)
			{
				glMaterial(*fIt);

				fvIt = mesh_.cfv_iter(*fIt);
				glArrayElement(fvIt->idx());
				++fvIt;
				glArrayElement(fvIt->idx());
				++fvIt;
				glArrayElement(fvIt->idx());
			}
			glEnd();

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
		}

		else if (_draw_mode == "Points") // -----------------------------------------
		{
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, mesh_.points());

			if (mesh_.has_vertex_colors() && use_color_)
			{
				glEnableClientState(GL_COLOR_ARRAY);
				glColorPointer(3, GL_UNSIGNED_BYTE, 0, mesh_.vertex_colors());
			}

			glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(mesh_.n_vertices()));
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);
		}


}

template <typename M>
void MeshViewerWidgetT<M>::draw_scene(const std::string& _draw_mode)
{
	if (!mesh_.n_vertices())
		return;

		if (_draw_mode == "Points")
		{
			glDisable(GL_LIGHTING);
			draw_openmesh(_draw_mode);
		}
		else if (_draw_mode == "Wireframe")
		{
			glDisable(GL_LIGHTING);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			draw_openmesh(_draw_mode);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		else if (_draw_mode == "Hidden-Line")
		{
			glDisable(GL_LIGHTING);
			glShadeModel(GL_FLAT);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
			glDepthRange(0.01, 1.0);
			draw_openmesh("Wireframe");

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glDepthRange(0.0, 1.0);
			draw_openmesh("Wireframe");

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		else if (_draw_mode == "Solid Flat")
		{
			glEnable(GL_LIGHTING);
			glShadeModel(GL_FLAT);
			draw_openmesh(_draw_mode);
		}

		else if (_draw_mode == "Solid Smooth" )
		{
			glEnable(GL_LIGHTING);
			glShadeModel(GL_SMOOTH);
			draw_openmesh(_draw_mode);
		}

		else if (_draw_mode == "Colored Vertices")
		{
			glEnable(GL_LIGHTING);
			glEnable(GL_COLOR_MATERIAL);
			glShadeModel(GL_SMOOTH);
			draw_openmesh(_draw_mode); 
		}

		else if (_draw_mode == "Solid Colored Faces")
		{
			glDisable(GL_LIGHTING);
			glShadeModel(GL_FLAT);
			draw_openmesh(_draw_mode);
			setDefaultMaterial();
		}

		else if (_draw_mode == "Smooth Colored Faces")
		{
			glEnable(GL_LIGHTING);
			glShadeModel(GL_SMOOTH);
			draw_openmesh(_draw_mode);
			setDefaultMaterial();
		}

	if (show_vnormals_)
	{
		typename Mesh::VertexIter vit;
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glColor3f(1.000f, 0.803f, 0.027f); // orange
		for (vit = mesh_.vertices_begin(); vit != mesh_.vertices_end(); ++vit)
		{
			glVertex(*vit);
			glVertex(mesh_.point(*vit) + normal_scale_*mesh_.normal(*vit));
		}
		glEnd();
	}

	if (show_fnormals_)
	{
		typename Mesh::FaceIter fit;
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glColor3f(0.705f, 0.976f, 0.270f); // greenish
		for (fit = mesh_.faces_begin(); fit != mesh_.faces_end(); ++fit)
		{
			glVertex(mesh_.property(fp_normal_base_, *fit));
			glVertex(mesh_.property(fp_normal_base_, *fit) +
				normal_scale_*mesh_.normal(*fit));
		}
		glEnd();
	}
}