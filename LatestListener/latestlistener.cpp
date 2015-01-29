#include "latestlistener.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QjsonArray>
#include <vector>
#include <string>

using std::vector;
using std::string;

//constant
static const unsigned int REQUEST_SONG_LIMIT = 50;
static const unsigned int DEFAULT_VOLUME = 50;
static const unsigned int MAX_ONE_LINE_LENGTH = 15;
static const QString RESOURCE_PATH = "/LatestListener/Resources/";

LatestListener::LatestListener(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	init_member();
	alloc();
	change_page(SETTING);
	//connect
	connect(ui.listen_btn, &QPushButton::clicked, this, &LatestListener::listen_btn_clicked);
	connect(ui.play_btn, &QPushButton::clicked, this, &LatestListener::play_pause_btn_clicked);
	connect(ui.stop_btn, &QPushButton::clicked, this, &LatestListener::stop_btn_clicked);
	connect(ui.volume_slider, SIGNAL(valueChanged(int)), this, SLOT(volume_changed(int)));
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

void LatestListener::init_member(){
	play_index = 0;
	volume = DEFAULT_VOLUME;
	limit = REQUEST_SONG_LIMIT;
	playing = false;
	ui.volume_slider->setSliderPosition(DEFAULT_VOLUME);
	//genre request number
	genre_num.insert(std::make_pair("J-Pop", "27"));
	genre_num.insert(std::make_pair("Anime", "29"));
	genre_num.insert(std::make_pair("Jazz", "11"));
	genre_num.insert(std::make_pair("Rock", "21"));
	genre_num.insert(std::make_pair("Electronic", "7"));
	genre_num.insert(std::make_pair("Opera", "9"));
	genre_num.insert(std::make_pair("Blues", "2"));
	genre_num.insert(std::make_pair("Classical", "5"));
	genre_num.insert(std::make_pair("K-Pop", "51"));
	genre_num.insert(std::make_pair("R&B/Soul", "15"));
	genre_num.insert(std::make_pair("Enka", "28"));
	genre_num.insert(std::make_pair("Country", "6"));
	genre_num.insert(std::make_pair("Children's Music", "4"));
	genre_num.insert(std::make_pair("Hip-Hop/Rap", "18"));
}

void LatestListener::alloc(){
	//allocation
	m_player = new QMediaPlayer();
	api_manager = new QNetworkAccessManager(this);
	img_manager = new QNetworkAccessManager(this);
	song_urls = new vector<QString>();
	artworks = new vector<QString>();
	titles = new vector<QString>();
	artists = new vector<QString>();
}

void LatestListener::change_page(LatestListener::Page p){
	switch (p)
	{
	case LatestListener::SETTING:
		ui.stack->setCurrentIndex(1);
		break;
	case LatestListener::PLAY:
		ui.stack->setCurrentIndex(0);
		break;
	default:
		break;
	}
}

void LatestListener::reset_player(){
	play_index = 0;
	song_urls->clear();
	artworks->clear();
	titles->clear();
	artists->clear();
}

void LatestListener::listen_btn_clicked(){
	QString url, l, g;
	l.setNum(limit);
	g = genre_num[ui.genre_box->currentText()];
	url = "https://itunes.apple.com/jp/rss/topsongs/limit=" + l + "/genre=" + g + "/json";
	api_manager->get(QNetworkRequest(QUrl(url)));
}

void LatestListener::stop_btn_clicked(){
	playing = false;
	m_player->stop();
	reset_player();
	change_wiget_color(this, Qt::white, QPalette::Background);
	change_page(SETTING);
}

void LatestListener::play_pause_btn_clicked(){
	QMediaPlayer::State state = m_player->state();
	if (state == QMediaPlayer::PlayingState){
		m_player->pause();
	}
	else if (state == QMediaPlayer::PausedState){
		m_player->play();
	}
}

void LatestListener::volume_changed(int v){
	volume = v;
	m_player->setVolume(volume);
}

void LatestListener::music_player_changed(QMediaPlayer::State state){
	QString ss;
	switch (state)
	{
	case QMediaPlayer::StoppedState:
		if (playing){
			//update index
			if (play_index < limit-1)
				play_index++;
			else
				play_index = 0;
			//set title and artist
			ui.title_label->setText(titles->at(play_index));
			ui.artist_label->setText(artists->at(play_index));
			//request artwork image
			img_manager->get(QNetworkRequest(QUrl(artworks->at(play_index))));
			//play song
			m_player->setMedia(QUrl(song_urls->at(play_index)));
			m_player->setVolume(volume);
			m_player->play();
		}
		break;
	case QMediaPlayer::PlayingState:
		ss = ui.play_btn->styleSheet();
		ui.play_btn->setStyleSheet(ss+"QPushButton#play_btn {background-image: url(:"+RESOURCE_PATH+"pause_btn.png) }");
		break;
	case QMediaPlayer::PausedState:
		ss = ui.play_btn->styleSheet();
		ui.play_btn->setStyleSheet(ss+"QPushButton#play_btn {background-image: url(:"+RESOURCE_PATH+"play_btn.png) }");
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
	for (auto song : entry_array){
		QJsonObject s_obj = song.toObject();
		song_urls->push_back(s_obj["link"].toArray().at(1).toObject().value("attributes").toObject().value("href").toString());
		artworks->push_back(s_obj["im:image"].toArray().at(2).toObject().value("label").toString());
		titles->push_back(s_obj["im:name"].toObject().value("label").toString());
		artists->push_back(s_obj["im:artist"].toObject().value("label").toString());
	}
	//insert line feed code
	for(unsigned int i=0;i<titles->size();i++){
		if(titles->at(i).size() > MAX_ONE_LINE_LENGTH)
			titles->at(i).insert(MAX_ONE_LINE_LENGTH, '\n');
		if(artists->at(i).size() > 10)
			artists->at(i).insert(MAX_ONE_LINE_LENGTH, '\n');
	}
	//set title and artist
	ui.title_label->setText(titles->at(0));
	ui.artist_label->setText(artists->at(0));
	//request artwork image
	img_manager->get(QNetworkRequest(QUrl(artworks->at(0))));
	//play song
	m_player->setMedia(QUrl(song_urls->at(0)));
	m_player->setVolume(volume);
	m_player->play();
	playing = true;
	//change to play page
	change_page(PLAY);
}

void LatestListener::img_request_finished(QNetworkReply *reply){
	//error
	if (reply->error() != QNetworkReply::NoError) {
		qDebug() << "Error in" << reply->url() << ":" << reply->errorString();
		return;
	}
	QByteArray jpegData = reply->readAll();
	//set image to image label
	QPixmap pixmap;
	pixmap.loadFromData(jpegData);
	ui.image_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	ui.image_label->setScaledContents(true);
	ui.image_label->setPixmap(pixmap);
	//get mean color
	QColor mean = get_mean_color(pixmap.toImage());
	bool use_black = is_contrast_enough(mean);
	//set color to frame and label
	change_wiget_color(this, mean, QPalette::Background);
	change_wiget_color(ui.artist_label, (use_black ? Qt::black : Qt::white), QPalette::Foreground);
	change_wiget_color(ui.title_label, (use_black ? Qt::black : Qt::white), QPalette::Foreground);
}

QColor LatestListener::get_mean_color(QImage img){
	double sr = 0.0, sg = 0.0, sb = 0.0;
	int n_pixel = img.width() * img.height();

	for(int i=0;i<img.height();i++){
		for(int j=0;j<img.width();j++){
			QColor col(img.pixel(j,i));
			sr += (double)col.red() / n_pixel;
			sg += (double)col.green() / n_pixel;
			sb += (double)col.blue() / n_pixel;
		}
	}

	return QColor((int)sr, (int)sg, (int)sb);
}

//whether contrast is enough compare with black
bool LatestListener::is_contrast_enough(QColor col){
	//brightness
	int bright = (col.red()*299 + col.green()*587 + col.blue()*114) / 1000;
	if(bright < 125) return false;
	//color difference
	int col_dif = col.red() + col.green() + col.blue();
	if(col_dif < 500) return false;

	return true;
}

void LatestListener::change_wiget_color(QWidget *w ,QColor col, QPalette::ColorRole cr){
	QPalette f_pal = w->palette();
	f_pal.setColor(cr, col);
	w->setPalette(f_pal);
}
