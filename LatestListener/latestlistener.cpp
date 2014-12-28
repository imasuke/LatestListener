#include "latestlistener.h"

#include <QFileDialog>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QjsonArray>
#include <vector>
#include <string>

using std::vector;
using std::string;

LatestListener::LatestListener(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	play_index = 0;
	//allocation
	m_player = new QMediaPlayer();
	api_manager = new QNetworkAccessManager(this);
	img_manager = new QNetworkAccessManager(this);
	song_urls = new vector<QString>();
	artworks = new vector<QString>();
	titles = new vector<QString>();
	artists = new vector<QString>();
	//connect
	connect(ui.listen_btn, &QPushButton::clicked, this, &LatestListener::listen_btn_pushed);
	connect(m_player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
	connect(m_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(music_player_changed(QMediaPlayer::State)));
	connect(api_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(api_request_finished(QNetworkReply*)));
	connect(img_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(img_request_finished(QNetworkReply*)));
}

LatestListener::~LatestListener()
{
	delete m_player;
	delete api_manager;
	delete img_manager;
	delete song_urls;
	delete artworks;
	delete titles;
	delete artists;
}

void LatestListener::listen_btn_pushed(){
	QString url = "https://itunes.apple.com/jp/rss/topsongs/limit=50/genre=27/json";
    api_manager->get(QNetworkRequest(QUrl(url)));
}

void LatestListener::music_player_changed(QMediaPlayer::State state){
	switch (state)
	{
	case QMediaPlayer::StoppedState:
		play_index++;
		//set title and artist
		ui.title_label->setText(titles->at(play_index));
		ui.artist_label->setText(artists->at(play_index));
		//request artwork image
		img_manager->get(QNetworkRequest(QUrl(artworks->at(play_index))));
		//play song
		m_player->setMedia(QUrl(song_urls->at(play_index)));
		m_player->setVolume(50);
		m_player->play();
		break;
	case QMediaPlayer::PlayingState:
		ui.image_label->setText("play");
		break;
	case QMediaPlayer::PausedState:
		ui.image_label->setText("pause");
		break;
	default:
		break;
	}
	
}

void LatestListener::api_request_finished(QNetworkReply *reply){
	//error
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error in" << reply->url() << ":" << reply->errorString();
        return;
    }

	QString rep_txt = (QString)reply->readAll();
	QJsonDocument jsondoc = QJsonDocument::fromJson(rep_txt.toUtf8());
	QJsonObject obj = jsondoc.object();
	QJsonArray entry_array = obj.value("feed").toObject().value("entry").toArray();

	//extract song info from json
	for(auto song : entry_array){
		QJsonObject s_obj = song.toObject();
		song_urls->push_back(s_obj["link"].toArray().at(1).toObject().value("attributes").toObject().value("href").toString());
		artworks->push_back(s_obj["im:image"].toArray().at(2).toObject().value("label").toString());
		titles->push_back(s_obj["im:name"].toObject().value("label").toString());
		artists->push_back(s_obj["im:artist"].toObject().value("label").toString());
	}
	//set title and artist
	ui.title_label->setText(titles->at(0));
	ui.artist_label->setText(artists->at(0));
	//request artwork image
	img_manager->get(QNetworkRequest(QUrl(artworks->at(0))));
	//play song
	m_player->setMedia(QUrl(song_urls->at(0)));
	m_player->setVolume(50);
	m_player->play();
}

void LatestListener::img_request_finished(QNetworkReply *reply){
	//error
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error in" << reply->url() << ":" << reply->errorString();
        return;
    }
    qDebug() << "ContentType:" << reply->header(QNetworkRequest::ContentTypeHeader).toString();
    QByteArray jpegData = reply->readAll();
    QPixmap pixmap;
    pixmap.loadFromData(jpegData);
    ui.image_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui.image_label->setScaledContents(true);
    ui.image_label->setPixmap(pixmap);
}
