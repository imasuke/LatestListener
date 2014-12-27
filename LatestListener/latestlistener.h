#ifndef LATESTLISTENER_H
#define LATESTLISTENER_H

#include <QtWidgets/QWidget>
#include <QtMultimedia/QMediaPlayer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <string>
#include "ui_latestlistener.h"

class LatestListener : public QWidget
{
	Q_OBJECT

public:
	LatestListener(QWidget *parent = 0);
	~LatestListener();

private:
	std::string url_encode(const std::string& str, bool space_to_plus = false);

public slots:
	void listen_btn_pushed();
	void api_request_finished(QNetworkReply*);
	void img_request_finished(QNetworkReply*);

private:
	Ui::LatestListenerClass ui;
	QMediaPlayer *m_player;
	QNetworkAccessManager *api_manager;
	QNetworkAccessManager *img_manager;
};

#endif // LATESTLISTENER_H
