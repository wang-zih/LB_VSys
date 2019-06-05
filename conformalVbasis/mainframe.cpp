#include"mainframe.h"
#include<QtWidgets\QMenuBar>
#include<QtWidgets\QToolBar>
#include<QtWidgets\QStatusBar>
#include<QtWidgets\QFileDialog>
#include<QtWidgets\QMessageBox>
#include<QtWidgets\QInputDialog>
#include<QtWidgets\QPushButton>
#include<QtWidgets\QFormLayout>
#include<QtWidgets\QSpinBox>
#include<QtWidgets\QComboBox>
#include<QtWidgets\QDialogButtonBox>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include"methods.h"

using std::cout;
using std::endl;

AppFrame::AppFrame(QWidget *parent) {
	lb_spec = new QLabel("Choice Id");
	text_spec = new QLineEdit("1");
	bt_reload = new QPushButton("reload");
	bt_sphereMap = new QPushButton("sphereMap");
	cb_mode = new QComboBox();
	text_spec->setMaximumWidth(100);
	cb_mode->addItem("Points", 1);cb_mode->addItem("Wireframe", 2); 
	cb_mode->addItem("Solid Flat", 3);cb_mode->addItem("Solid Smooth", 4);
	cb_mode->setCurrentIndex(2);
	connect(bt_reload, SIGNAL(clicked()), this, SLOT(updateColorMapping()));
	connect(bt_sphereMap, SIGNAL(clicked()), this, SLOT(CalcSphereMapping()));
}

void AppFrame::setControlLayout() {
	Applayout = new QGridLayout();
	Applayout->addWidget(renderColorMap, 0, 0, 1, 3);
	Applayout->addWidget(renderBasis, 0, 3, 1, 2);

	Applayout->addWidget(bt_sphereMap, 1, 0, 1, 1);
	Applayout->addWidget(lb_spec,      1, 1, 1, 1);
	Applayout->addWidget(text_spec,    1, 2, 1, 1);
	Applayout->addWidget(cb_mode,      1, 3, 1, 1);
	Applayout->addWidget(bt_reload,    1, 4, 1, 1);
	setLayout(Applayout);
	setGeometry(300, 300, 600, 600);
}
void AppFrame::setViewer(QString &name, Eigen::MatrixXf &evecs_data) {
	//============================== render SignalColormap ==================
	renderColorMap = new AppMesh();
	renderColorMap->open_mesh_gui(name);
	renderColorMap->add_draw_mode("Colored Vertices");
	renderColorMap->set_draw_mode(5);
	int mapVecId = 1;

	evecs = evecs_data;
	renderColorMap->ColorMapping(evecs.col(mapVecId).data(), evecs.col(mapVecId).minCoeff(), evecs.col(mapVecId).maxCoeff());

	renderBasis = new AppMesh();

	BaseMesh sphereMesh;
	ConstSphere(sphereMesh, 100);
	renderBasis->set_mesh(sphereMesh);
	cout << "vector:" << sphereMesh.n_vertices() << endl;

	renderBasis->add_draw_mode("Colored Vertices");
	renderBasis->set_draw_mode(3);
	
}

void AppFrame::CalcSphereMapping() {
	//==============================sphere mapping=================
	OpenMesh::Utils::Timer t1;
	t1.start();
	VPropHandleT<Vec3f> mapping;
	renderColorMap->sphereConformalMapping(mapping);
	t1.stop();
	cout << "sphere mapping time~" << t1.as_string() << endl;

	//============================== render Basis ==================
	int mapVecId = 1;
	Eigen::VectorXf sphereVal(renderBasis->get_mesh().n_vertices());
	renderColorMap->sphericalPara(mapping, renderBasis->get_mesh(), evecs.col(mapVecId).data(),sphereVal);
	renderBasis->ColorMapping(sphereVal.data(), sphereVal.minCoeff(), sphereVal.maxCoeff());
	bt_sphereMap->setEnabled(false);
	cb_mode->addItem("Solid Colored Vertices", 5);
	renderBasis->set_draw_mode(5);

	setSphereMapping(mapping);
}

void AppFrame::updateColorMapping() {
	int mapVecId=text_spec->text().toInt();
	cout << "current eigenVector: "<<mapVecId << endl;	
	renderColorMap->ColorMapping(evecs.col(mapVecId).data(), evecs.col(mapVecId).minCoeff(), evecs.col(mapVecId).maxCoeff());
	renderColorMap->updateGL();
	
	renderBasis->set_draw_mode(cb_mode->currentIndex()+1);
	if (cb_mode->currentIndex() + 1 == 5) {


		Eigen::VectorXf sphereVal(renderBasis->get_mesh().n_vertices());
		renderColorMap->sphericalPara(getSphereMapping(), renderBasis->get_mesh(), evecs.col(mapVecId).data(), sphereVal);
		//cout << sphereVal.transpose().head(100) << endl;
		cout << sphereVal.minCoeff() <<","<< sphereVal.maxCoeff() << endl;
		renderBasis->ColorMapping(sphereVal.data(), sphereVal.minCoeff(), sphereVal.maxCoeff());
	}
	renderBasis->updateGL();
}

MainFrame::MainFrame(QWidget *parent)
	:QMainWindow(parent)
{
	SetControlsLayout();
	Create_Actions();
	Create_Menus();
	Create_ToolBar();
	Create_SatusBar();
	setWindowTitle("Demo");
	setGeometry(100, 100, 600, 600);
}


void MainFrame::SetControlsLayout()
{
	centerWidget = new  QWidget();
	setCentralWidget(centerWidget);

	OpenMesh::IO::Options opt;
	opt += OpenMesh::IO::Options::VertexColor;
	opt += OpenMesh::IO::Options::VertexNormal;
	opt += OpenMesh::IO::Options::FaceColor;
	opt += OpenMesh::IO::Options::FaceNormal;

	renderWidget = new AppMesh(this);
	renderWidget->setOptions(opt);

	mainLayout = new QGridLayout();
	mainLayout->addWidget(renderWidget, 0, 0);//在布局中添加显示渲染界面
	centerWidget->setLayout(mainLayout);
	
}

void MainFrame::Create_Menus()
{
	fileMenu = menuBar()->addMenu(tr("File"));
	fileMenu->addAction(act_FileOpen);
	fileMenu->addAction(act_FileSave);

	renderMenu = menuBar()->addMenu(tr("Render"));
	renderMenu->addActions(act_RenderType->actions());
	renderMenu->addAction(act_Material);

	application = menuBar()->addMenu(tr("Applications"));
	application->addAction(act_Remove);
	application->addAction(act_LP);

	databaseApp = menuBar()->addMenu(tr("database"));
	databaseApp->addAction(act_calcSphereEigenMapForDB);
}

void MainFrame::Create_ToolBar()
{
	fileToolBar = addToolBar(tr("File"));
	fileToolBar->addAction(act_FileOpen);
	fileToolBar->addSeparator();
	fileToolBar->addAction(act_FileSave);

	renderToolBar = addToolBar(tr("Render model"));
	
	QList<QAction*> Qlist = act_RenderType->actions();
	QList<QAction*>::iterator it;
	for (it = Qlist.begin(); it != Qlist.end(); ++it) {
		renderToolBar->addAction(*it);
		renderToolBar->addSeparator();
	}
	
	appToolBar = addToolBar(tr("Application"));
}

void MainFrame::Create_SatusBar()
{
	statusBar()->showMessage("status: no mesh is read!");
	statusBar()->setStyleSheet("QWidget{ border: 1px solid #d3d3d3; background - color:#FFFFFF }");
}

void MainFrame::Create_Actions()
{
	act_FileOpen = new QAction(QIcon("imgs/fileopen.png"), QString("Open mesh ..."), this);
	act_FileOpen->setShortcut(tr("Ctrl+O"));
	act_FileOpen->setStatusTip(tr("Open a mesh"));	
	connect(act_FileOpen, SIGNAL(triggered()), this, SLOT(s_open_mesh_file()));


	act_FileSave = new QAction(QIcon("imgs/filesave.png"), QString("Save mesh ..."), this);
	act_FileSave->setShortcut(tr("Ctrl+S"));
	act_FileSave->setStatusTip(tr("Save a mesh, up to now only .ply is supported."));
	act_FileSave->setEnabled(false);
/////////////////////////////////////// Render ///////////////////////////////////////////////////////
	QString renderModes[4] = { QString("Points"),QString("Wireframe"),QString("Solid Flat"),QString("Solid Smooth") };
	QIcon renderLabels[4] = { QIcon("imgs/points.png") ,QIcon("imgs/wire.png") ,QIcon("imgs/flat.png"),QIcon("imgs/smooth.png") };

	act_RenderType = new QActionGroup(this);
	for (int i = 0; i < 4; ++i) {
		QAction* act_render = new QAction(renderLabels[i], renderModes[i], this);
		act_render->setData(i + 1);
		act_RenderType->addAction(act_render);
	}

	act_RenderType->setEnabled(false);

	QList<QAction*> Qlist = act_RenderType->actions();
	QList<QAction*>::iterator it;
	for (it = Qlist.begin(); it != Qlist.end(); ++it) {
		(*it)->setCheckable(true);
	}	
	connect(act_RenderType, SIGNAL(triggered(QAction*)), this, SLOT(s_Render(QAction*)));

	act_Material = new QAction(QIcon("imgs/Material.png"),QString("Apply Material"), this);
	act_Material->setEnabled(false);
	connect(act_Material, SIGNAL(triggered()), this, SLOT(s_Material()));
///////////////////////////////Applications/////////////////////////////////////////////
	act_Remove= new QAction(QString("Remove Dumplicated"), this);
	act_Remove->setEnabled(false);
	connect(act_Remove, SIGNAL(triggered()), this, SLOT(s_RemoveDumplicateElements()));

	act_LP= new QAction(QString("Laplace EigenDec"), this);
	act_LP->setEnabled(false);
	connect(act_LP, SIGNAL(triggered()), this, SLOT(s_LPcolormap()));
////////////////////////////////////database/////////////////////////////////////
	act_calcSphereEigenMapForDB = new QAction(QString("CalcSphereMap"), this);
	connect(act_calcSphereEigenMapForDB, SIGNAL(triggered()), this, SLOT(s_CalcSphereEigenMapForDB()));

}

void MainFrame::s_Render(QAction *action) {

	int value = action->data().toInt();
	renderWidget->set_draw_mode(value);
	std::cout << "Render mode: "<< value << std::endl;
	renderWidget->updateGL();
}

void MainFrame::s_open_mesh_file()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open mesh file"),
		tr(""),
		tr("OBJ Files (*.obj);;"
			"OFF Files (*.off);;"
			"STL Files (*.stl);;"
			"All Files (*)"));
	if (!fileName.isEmpty()) {
		renderWidget->open_mesh_gui(fileName);
		act_RenderType->setEnabled(true);
		act_Material->setEnabled(true);
		act_Remove->setEnabled(true);
		act_LP->setEnabled(true);
		renderWidget->setFilename(fileName);
	}
	
}

void MainFrame::s_Material()
{

	//================================= control panel ================================
	GLfloat light[13] = { 0.1f, 0.1f, 0.1f, 1.0f  ,
		0.7f, 0.7f, 0.5f, 1.0f ,
		1.0f, 1.0f, 1.0f, 1.0f ,
		120.0f };

	QDialog dialog(this);
	QList<QDoubleSpinBox*> spinboxlist;
	QGridLayout lay(&dialog);
	for (int i = 0; i < 13; ++i) {
		QDoubleSpinBox* spinbox = new QDoubleSpinBox();
		spinbox->setSingleStep(0.1);
		spinbox->setValue(light[i]);
		spinboxlist.push_back(spinbox);
	}spinboxlist[12]->setRange(0, 125);

	lay.addWidget(new QLabel("ambient: "), 0, 0);
	lay.addWidget(new QLabel("diffuse: "), 1, 0);
	lay.addWidget(new QLabel("specular: "), 2, 0);
	lay.addWidget(new QLabel("shininess: "), 3, 0);

	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 4; ++j)
			lay.addWidget(spinboxlist[i * 4 + j], i, j + 1);
	lay.addWidget(spinboxlist[12], 3, 1);
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
		Qt::Horizontal, &dialog);
	lay.addWidget(&buttonBox, 4, 0, 1, 3);

	QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
	QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

	if (dialog.exec() == QDialog::Accepted) {
		GLfloat mat_a[] = { spinboxlist[0]->value(),spinboxlist[1]->value(),spinboxlist[2]->value(),spinboxlist[3]->value() };
		GLfloat mat_d[] = { spinboxlist[4]->value(),spinboxlist[5]->value(),spinboxlist[6]->value(),spinboxlist[7]->value() };
		GLfloat mat_s[] = { spinboxlist[8]->value(),spinboxlist[9]->value(),spinboxlist[10]->value(),spinboxlist[11]->value() };
		GLfloat shine[] = { spinboxlist[12]->value() };
		cout << "va " << mat_d[0] << endl;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_a);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_d);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_s);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
		renderWidget->updateGL();
	}

}

void MainFrame::s_RemoveDumplicateElements()
{
	renderWidget->RemoveDumplicateItems();
}
void MainFrame::s_LPcolormap()
{
	int n_spectrum = 0;
	bool ok = false;
	QString text=QInputDialog::getText(this, "Input Info", 
		tr("Input the number of spectrum to be computed"), QLineEdit::Normal, QString::null,&ok);
	
	if (ok && !text.isEmpty()) {
		n_spectrum = text.toInt();
		cout << "n: " << n_spectrum << endl;
		if (n_spectrum <= 0) return;
	}
	else {
		return;
	}
//================================================================================
	Eigen::SparseMatrix<float> Laplace;
	OpenMesh::Utils::Timer t1,t2;
	Eigen::VectorXf evalues;
	Eigen::MatrixXf evecs;


	t1.start();
	renderWidget->ComputeWeightedGraphLP(Laplace);
	t1.stop();
	cout << "LP norm: "<<Laplace.norm() << endl;
	cout << "LT-L error:"<<isSymmetry(Laplace) << endl;
	cout << "LP time~" << t1.as_string() << endl;

	t2.start();
	EigenDecomposition(Laplace, n_spectrum, 0, evalues, evecs);
	t2.stop();
	cout << "Eigen time~" << t2.as_string() << endl;
	
	evalues.reverseInPlace();
	evecs.rowwise().reverseInPlace();
	evecs.array().rowwise() /= evalues.transpose().array().sqrt();


	cout << evalues.transpose() << endl; cout << endl;
//===============================================================================	
	AppColorMap = new AppFrame();
	AppColorMap->setViewer(renderWidget->modelName(), evecs);
	AppColorMap->setControlLayout();
	AppColorMap->show();
}


void MainFrame::s_CalcSphereEigenMapForDB()
{
	int n_spectrum = 10;
	int databasesize = 1200;

	int n1, n2, N;
	Eigen::SparseMatrix<float> Laplace;
	Eigen::VectorXf evalues;
	Eigen::MatrixXf evecs;
	QString fname;

	OpenMesh::IO::Options opt;
	opt += OpenMesh::IO::Options::VertexNormal;
	opt += OpenMesh::IO::Options::FaceNormal;

	VPropHandleT<Vec3f> mapping;

	BaseMesh sphereMesh;
	ConstSphere(sphereMesh, 100, false);
	Eigen::VectorXf sphereVal(sphereMesh.n_vertices());


	QString dirname = "F:\database\benchmark\shrec15\NonRigid\\";
	for (int i = 0; i < databasesize; ++i)
	{
		fname = dirname + "T" + QString::number(i) + ".off";
		AppMesh* model = new AppMesh();
		
		model->open_mesh(fname.toLocal8Bit(), opt);


		//Lp eigenMap
		model->ComputeWeightedGraphLP(Laplace);
		EigenDecomposition(Laplace, n_spectrum, 0, evalues, evecs);
		evalues.reverseInPlace();
		evecs.rowwise().reverseInPlace();
		evecs.array().rowwise() /= evalues.transpose().array().sqrt();
		//sphere mapping
		//OpenMesh::VPropHandleT<Vec3f> mapping;
		model->sphereConformalMapping(mapping);
		model->sphericalPara(mapping,sphereMesh, evecs.col(1).data(), sphereVal);
	}

}