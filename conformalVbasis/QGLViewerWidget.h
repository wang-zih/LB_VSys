#ifndef QGLVIEWERWIDGETT_H
#define QGLVIEWERWIDGETT_H

#include<QtOpenGL\QGLWidget>
#include <OpenMesh/Core/Geometry/VectorT.hh>

//=== Forward declarations =======


class QGLViewerWidget :public QGLWidget
{
	Q_OBJECT

public:
	QGLViewerWidget(QWidget* _parent = 0);
	QGLViewerWidget(QGLFormat& _fmt, QWidget* _parent = 0);

	virtual ~QGLViewerWidget();
private:
	void init(void);

public:

	/* Sets the center and size of the whole scene.
	The _center is used as fixpoint for rotations and for adjusting
	the camera/viewer (see view_all()). */
	void set_scene_pos(const OpenMesh::Vec3f& _center, float _radius);

	/* view the whole scene: the eye point is moved far enough from the
	center so that the whole scene is visible. */
	void view_all();

	void add_draw_mode(const std::string& _s);
	void set_draw_mode(const int i);
	float fovy() const { return 45.0f; }

protected:

	virtual void draw_scene(const std::string& _draw_mode);

	void setDefaultLight();
	void setDefaultMaterial();

	virtual void mouseReleaseEvent(QMouseEvent*);
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void wheelEvent(QWheelEvent*);

private:
	void initializeGL();
	void paintGL();
	void resizeGL(int w, int h);


	void update_projection_matrix();
	void translate(const OpenMesh::Vec3f& _trans);
	void rotate(const OpenMesh::Vec3f& _axis, float _angle);
	

	GLdouble    projection_matrix_[16],
		modelview_matrix_[16];

	float radius;
	OpenMesh::Vec3f center;

	// virtual trackball: map 2D screen point to unit sphere
	bool map_to_sphere(const QPoint& _point, OpenMesh::Vec3f& _result);

	QPoint           last_point_2D_;
	OpenMesh::Vec3f  last_point_3D_;
	bool             last_point_ok_;

	int draw_mode;
	int n_draw_modes;
	std::vector<std::string> draw_mode_names;

};



#endif