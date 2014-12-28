#ifndef LATESTLISTENER_H
#define LATESTLISTENER_H

#include <QtWidgets/QWidget>
#include <QtMultimedia/QMediaPlayer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <vector>
#include "ui_latestlistener.h"

class LatestListener : public QWidget
{
	Q_OBJECT

public:
	LatestListener(QWidget *parent = 0);
	~LatestListener();

private:

public slots:
	void listen_btn_pushed();
	void music_player_changed(QMediaPlayer::State);
	void api_request_finished(QNetworkReply*);
	void img_request_finished(QNetworkReply*);

private:
	Ui::LatestListenerClass ui;
	QMediaPlayer *m_player;
	QNetworkAccessManager *api_manager;
	QNetworkAccessManager *img_manager;
	std::vector<QString> *song_urls;
	std::vector<QString> *artworks;
	std::vector<QString> *titles;
	std::vector<QString> *artists;
	unsigned int play_index;
	
};

#endif // LATESTLISTENER_H
