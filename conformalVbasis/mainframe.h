#ifndef MAINFRAME_H
#define MAINFRAME_H
#include<string>
#include<QtWidgets\QMainWindow>
#include<QtWidgets\QMenu>
#include <QtWidgets\QWidget>
#include<QtWidgets\QLabel>
#include<QtWidgets\QLineEdit>

#include<QtWidgets\QGridLayout>
#include<QtWidgets\QVboxLayout>
#include"AppMesh.h"

class AppFrame:public QWidget
{
	Q_OBJECT

public:
	AppFrame(QWidget *parent = 0);
	~AppFrame() {};
	void setViewer(AppMesh *outer) { renderColorMap = outer; };
	void setControlLayout();
	void setViewer(QString &name, Eigen::MatrixXf &evecs);
	void setSphereMapping(VPropHandleT<Vec3f>& mapping) { spheremapping=mapping; };
	VPropHandleT<Vec3f>& getSphereMapping() { return spheremapping; };

	public slots:
	void updateColorMapping();
	void CalcSphereMapping();

private:
	AppMesh *renderColorMap;
	AppMesh *renderBasis;

	QGridLayout* Applayout;
	QLabel* lb_spec;
	QLineEdit* text_spec;
	QComboBox* cb_mode;
	QPushButton* bt_sphereMap;
	QPushButton* bt_reload;
	Eigen::MatrixXf evecs;
	VPropHandleT<Vec3f> spheremapping;
};

class MainFrame : public QMainWindow
{
	Q_OBJECT

public:
	MainFrame(QWidget *parent = 0);
	~MainFrame() {};
	void SetControlsLayout();
	void Create_Actions();
	void Create_Menus();
	void Create_ToolBar();
	void Create_SatusBar();

	
	public slots:
	void s_open_mesh_file();
	void s_Render(QAction *action);
	void s_Material();
	void s_LPcolormap();
	void s_RemoveDumplicateElements();
	void s_CalcSphereEigenMapForDB();
	

private:

	AppMesh *renderWidget;
	QWidget *centerWidget;
	AppFrame *AppColorMap;

	QMenu *fileMenu;
	QMenu *renderMenu;
	QMenu *application;
	QMenu *databaseApp;
	QToolBar *fileToolBar;
	QToolBar *renderToolBar;
	QToolBar *appToolBar;


	QAction* act_FileOpen;
	QAction* act_FileSave;
	QActionGroup* act_RenderType; //use QActionGroup to manage them
	QAction* act_Material;
	QAction* act_LP;
	QAction* act_Remove;
	QAction* act_calcSphereEigenMapForDB;
	///////////////////////////////////////////////

	QLabel *sliderLabel_specx;
	QLabel *sliderLabel_specy;
	QLabel *originImg;
	QLabel *transformedImg;

	QGridLayout *mainLayout;

private:

};


#endif