#include "latestlistener.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	LatestListener w;
	w.setFixedSize(400,200);
	w.show();
	return a.exec();
}
