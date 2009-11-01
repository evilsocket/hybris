#include <stdio.h>
#include <QtGui/qmessagebox.h>

extern "C" void MessageBox( char *title, char *text ){
	QMessageBox::information ( 0, title, title, QMessageBox::Ok );
}


