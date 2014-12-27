#ifndef LATESTLISTENER_H
#define LATESTLISTENER_H

#include <QtWidgets/QWidget>
#include "ui_latestlistener.h"

class LatestListener : public QWidget
{
	Q_OBJECT

public:
	LatestListener(QWidget *parent = 0);
	~LatestListener();

private:
	Ui::LatestListenerClass ui;
};

#endif // LATESTLISTENER_H
