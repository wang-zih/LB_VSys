#include <QtWidgets/QApplication>
#include<QtOpenGL\QGLWidget>
#include<QtWidgets\QMenuBar>
#include"mainframe.h"

int main(int argc, char *argv[])
{

	QApplication a(argc, argv);
	/*
	if (!QGLFormat::hasOpenGL()) {
		QString msg = "System has no OpenGL support!";
		QMessageBox::critical(0, QString("OpenGL"), msg + QString(argv[1]));
		return -1;
	}*/
	MainFrame w;
	
	w.show();
	


	return a.exec();
}





