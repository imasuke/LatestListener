#ifndef LATESTLISTENER_H
#define LATESTLISTENER_H

#include <QtWidgets/QWidget>
#include <QtMultimedia/QMediaPlayer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <vector>
#include <map>
#include "ui_latestlistener.h"

class LatestListener : public QWidget
{
	Q_OBJECT

	enum Page{SETTING, PLAY};

public:
	LatestListener(QWidget *parent = 0);
	~LatestListener();

private:
	void init_member();
	void alloc();
	void change_page(LatestListener::Page p);
	void reset_player();
	QColor get_mean_color(QImage img);
	bool is_contrast_enough(QColor col);
	void change_wiget_color(QWidget *w ,QColor col, QPalette::ColorRole cr);

public slots:
	void listen_btn_clicked();
	void stop_btn_clicked();
	void play_pause_btn_clicked();
	void volume_changed(int volume);
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
	std::map<QString, QString> genre_num; 
	unsigned int play_index;
	unsigned int limit;
	unsigned int volume;
	bool playing;
};

#endif // LATESTLISTENER_H
