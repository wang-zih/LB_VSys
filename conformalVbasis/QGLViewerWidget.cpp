#include<GL\glut.h>
#include<QtWidgets\QAction>
#include<QtWidgets\QMenu>
#include<QtGui\QMouseEvent>
#include"QGLViewerWidget.h"
const double TRACKBALL_RADIUS = 0.6;

using namespace Qt;
using namespace OpenMesh;

QGLViewerWidget::QGLViewerWidget(QWidget* _parent)
	:QGLWidget(_parent)
{
	init();
}

QGLViewerWidget::QGLViewerWidget(QGLFormat& _fmt, QWidget* _parent)
	: QGLWidget(_fmt,_parent)
{
	init();
}

QGLViewerWidget::~QGLViewerWidget()
{

}

void QGLViewerWidget::add_draw_mode(const std::string& s) {
	n_draw_modes++;
	draw_mode_names.push_back(s);
}

void QGLViewerWidget::set_draw_mode(const int i) {
	//_mode->data().toInt();
	draw_mode = i;//1 ("Points"); 2("Wireframe"); 3("Solid Flat"); 4("Solid Smooth");
}

void QGLViewerWidget::init()
{
	setAttribute(Qt::WA_NoSystemBackground, true);
	setFocusPolicy(Qt::StrongFocus);
	setAcceptDrops(true);
	setCursor(Qt::PointingHandCursor);

	n_draw_modes = 0;
	add_draw_mode("Points");
	add_draw_mode("Wireframe");
	add_draw_mode("Solid Flat");
	add_draw_mode("Solid Smooth");
	//add_draw_mode("Colored Vertices");

	set_draw_mode(4);
	updateGL();
}

void QGLViewerWidget::setDefaultLight() {
	GLfloat pos1[] = { 0.1f,  0.1f, -0.02f, 0.0f };
	GLfloat pos2[] = { -0.1f,  0.1f, -0.02f, 0.0f };
	GLfloat pos3[] = { 0.0f,  0.0f,  0.1f,  0.0f };
	GLfloat col1[] = { 0.7f,  0.7f,  0.8f,  1.0f };
	GLfloat col2[] = { 0.8f,  0.7f,  0.7f,  1.0f };
	GLfloat col3[] = { 1.0f,  1.0f,  1.0f,  1.0f };

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, pos1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, col1);
	glLightfv(GL_LIGHT0, GL_SPECULAR, col1);
	/**/
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_POSITION, pos2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, col2);
	glLightfv(GL_LIGHT1, GL_SPECULAR, col2);

	glEnable(GL_LIGHT2);
	glLightfv(GL_LIGHT2, GL_POSITION, pos3);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, col3);
	glLightfv(GL_LIGHT2, GL_SPECULAR, col3);
}

void QGLViewerWidget::setDefaultMaterial()
{
	GLfloat mat_a[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat mat_d[] = { 0.7f, 0.7f, 0.5f, 1.0f };
	GLfloat mat_s[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat shine[] = { 120.0f };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_a);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_d);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_s);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
}

void QGLViewerWidget::initializeGL()
{
	// OpenGL state
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glDisable(GL_DITHER);
	glEnable(GL_DEPTH_TEST);
	// Material
	//setDefaultMaterial();
	// Lighting
	glLoadIdentity();
	setDefaultLight();

	// Fog
	GLfloat fogColor[4] = { 0.3f, 0.3f, 0.4f, 1.0f };
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, 0.35f);
	glHint(GL_FOG_HINT, GL_DONT_CARE);
	glFogf(GL_FOG_START, 5.0f);
	glFogf(GL_FOG_END, 25.0f);

	// scene pos and size
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix_);
	set_scene_pos(Vec3f(0.0, 0.0, 0.0), 1.0);
}


void QGLViewerWidget::resizeGL(int w,int h)
{
	update_projection_matrix();
	glViewport(0, 0, w, h);
	updateGL();
}

void QGLViewerWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(projection_matrix_);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(modelview_matrix_);

	if (draw_mode)
	{
		assert(draw_mode <= n_draw_modes);
		draw_scene(draw_mode_names[draw_mode - 1]);
	}
}

void QGLViewerWidget::draw_scene(const std::string& draw_mode) {
	if (draw_mode == "Wireframe") {
		glDisable(GL_LIGHTING);
	}
	else if (draw_mode == "Solid Flat") {
		glEnable(GL_LIGHTING);
		glShadeModel(GL_FLAT);

	}
	else if (draw_mode == "Solid Smooth") {
		glEnable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);
	}
}

void QGLViewerWidget::mouseMoveEvent(QMouseEvent* _event)
{
	QPoint newPoint2D = _event->pos();

	// Left button: rotate around center_
	// Right button: translate object

	Vec3f  newPoint3D;
	bool   newPoint_hitSphere = map_to_sphere(newPoint2D, newPoint3D);

	float dx = newPoint2D.x() - last_point_2D_.x();
	float dy = newPoint2D.y() - last_point_2D_.y();

	float w = width();
	float h = height();


	// switch to current GL context
	makeCurrent();

	if (_event->buttons() == LeftButton) {

		if (last_point_ok_) {
			if ((newPoint_hitSphere = map_to_sphere(newPoint2D, newPoint3D))) {
				Vec3f axis = last_point_3D_ % newPoint3D;
				if (axis.sqrnorm() < 1e-7) {
					axis = Vec3f(1, 0, 0);
				}
				else {
					axis.normalize();
				}
				// find the amount of rotation
				Vec3f d = last_point_3D_ - newPoint3D;
				float t = 0.5 * d.norm() / TRACKBALL_RADIUS;
				if (t < -1.0)
					t = -1.0;
				else if (t > 1.0)
					t = 1.0;
				float phi = 2.0 * asin(t);
				float angle = phi * 180.0 / M_PI;
				rotate(axis, angle);
			}
		}

	}

	// move in x,y direction
	else if (_event->buttons() == RightButton)
	{
		float z = -(modelview_matrix_[2] * center[0] +
			modelview_matrix_[6] * center[1] +
			modelview_matrix_[10] * center[2] +
			modelview_matrix_[14]) /
			(modelview_matrix_[3] * center[0] +
				modelview_matrix_[7] * center[1] +
				modelview_matrix_[11] * center[2] +
				modelview_matrix_[15]);

		float aspect = w / h;
		float near_plane = 0.01 * radius;
		float top = tan(fovy() / 2.0f*M_PI / 180.0f) * near_plane;
		float right = aspect*top;

		translate(Vec3f(2.0*dx / w*right / near_plane*z,
			-2.0*dy / h*top / near_plane*z,
			0.0f));
	}

	// remember this point
	last_point_2D_ = newPoint2D;
	last_point_3D_ = newPoint3D;
	last_point_ok_ = newPoint_hitSphere;

	// trigger redraw
	updateGL();
}

void QGLViewerWidget::mouseReleaseEvent(QMouseEvent* /* _event */)
{
	last_point_ok_ = false;
}


void QGLViewerWidget::wheelEvent(QWheelEvent* _event)
{
	float d = -(float)_event->delta() / 120.0 * 0.2 * radius;
	translate(Vec3f(0.0, 0.0, d));
	updateGL();
	_event->accept();
}

void QGLViewerWidget::translate(const OpenMesh::Vec3f& _trans)
{
	// Translate the object by _trans
	// Update modelview_matrix_
	makeCurrent();
	glLoadIdentity();
	glTranslated(_trans[0], _trans[1], _trans[2]);
	glMultMatrixd(modelview_matrix_);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix_);
}

void QGLViewerWidget::rotate(const OpenMesh::Vec3f& _axis, float _angle)
{
	// Rotate around center center_, axis _axis, by angle _angle
	// Update modelview_matrix_

	Vec3f t(modelview_matrix_[0] * center[0] +
		modelview_matrix_[4] * center[1] +
		modelview_matrix_[8] * center[2] +
		modelview_matrix_[12],
		modelview_matrix_[1] * center[0] +
		modelview_matrix_[5] * center[1] +
		modelview_matrix_[9] * center[2] +
		modelview_matrix_[13],
		modelview_matrix_[2] * center[0] +
		modelview_matrix_[6] * center[1] +
		modelview_matrix_[10] * center[2] +
		modelview_matrix_[14]);

	makeCurrent();
	glLoadIdentity();
	glTranslatef(t[0], t[1], t[2]);
	glRotated(_angle, _axis[0], _axis[1], _axis[2]);
	glTranslatef(-t[0], -t[1], -t[2]);
	glMultMatrixd(modelview_matrix_);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix_);
}

bool QGLViewerWidget::map_to_sphere(const QPoint& _v2D, OpenMesh::Vec3f& _v3D)
{
	// This is actually doing the Sphere/Hyperbolic sheet hybrid thing,
	// based on Ken Shoemake's ArcBall in Graphics Gems IV, 1993.
	double x = (2.0*_v2D.x() - width()) / width();
	double y = -(2.0*_v2D.y() - height()) / height();
	double xval = x;
	double yval = y;
	double x2y2 = xval*xval + yval*yval;

	const double rsqr = TRACKBALL_RADIUS*TRACKBALL_RADIUS;
	_v3D[0] = xval;
	_v3D[1] = yval;
	if (x2y2 < 0.5*rsqr) {
		_v3D[2] = sqrt(rsqr - x2y2);
	}
	else {
		_v3D[2] = 0.5*rsqr / sqrt(x2y2);
	}

	return true;
}

void QGLViewerWidget::update_projection_matrix()
{
	makeCurrent();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (GLfloat)width() / (GLfloat)height(),
		0.01*radius, 100.0*radius);
	glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix_);
	glMatrixMode(GL_MODELVIEW);
}

void QGLViewerWidget::view_all()
{
	translate(OpenMesh::Vec3f(-(modelview_matrix_[0] * center[0] +
		modelview_matrix_[4] * center[1] +
		modelview_matrix_[8] * center[2] +
		modelview_matrix_[12]),
		-(modelview_matrix_[1] * center[0] +
			modelview_matrix_[5] * center[1] +
			modelview_matrix_[9] * center[2] +
			modelview_matrix_[13]),
		-(modelview_matrix_[2] * center[0] +
			modelview_matrix_[6] * center[1] +
			modelview_matrix_[10] * center[2] +
			modelview_matrix_[14] +
			3.0*radius)));
}

void QGLViewerWidget::set_scene_pos(const OpenMesh::Vec3f& _cog, float _radius)
{
	center = _cog;
	radius = _radius;
	glFogf(GL_FOG_START, 1.5*_radius);
	glFogf(GL_FOG_END, 3.0*_radius);

	update_projection_matrix();
	view_all();
}