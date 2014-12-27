#include "latestlistener.h"

#include <QFileDialog>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QjsonArray>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

using std::vector;
using std::string;

LatestListener::LatestListener(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_player = new QMediaPlayer();
	api_manager = new QNetworkAccessManager(this);
	img_manager = new QNetworkAccessManager(this);
	connect(ui.listen_btn, &QPushButton::clicked, this, &LatestListener::listen_btn_pushed);
	connect(m_player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
	connect(api_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(api_request_finished(QNetworkReply*)));
	connect(img_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(img_request_finished(QNetworkReply*)));
}

LatestListener::~LatestListener()
{
	delete m_player;
	delete api_manager;
	delete img_manager;
}

void LatestListener::listen_btn_pushed(){
	string term = "miwa";
	string str_url = "http://ax.itunes.apple.com/WebObjects/MZStoreServices.woa/wa/wsSearch?term="+url_encode(term, true)+"&country=JP&entity=musicTrack";
	QString url = tr(str_url.c_str());
    api_manager->get(QNetworkRequest(QUrl(url)));
}

void LatestListener::api_request_finished(QNetworkReply *reply){
	//error
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error in" << reply->url() << ":" << reply->errorString();
        return;
    }

	QString rep_txt = (QString)reply->readAll();
	QJsonDocument jsondoc = QJsonDocument::fromJson(rep_txt.toUtf8());
	QJsonArray song_array = jsondoc.object().value("results").toArray();
	vector<QString> song_urls, artworks, titles, artists;

	//extract song info from json
	for(auto song : song_array){
		QJsonObject s_obj = song.toObject();
		song_urls.push_back(s_obj["previewUrl"].toString());
		artworks.push_back(s_obj["artworkUrl100"].toString());
		titles.push_back(s_obj["trackName"].toString());
		artists.push_back(s_obj["artistName"].toString());
	}
	//set title and artist
	ui.title_label->setText(titles[0]);
	ui.artist_label->setText(artists[0]);
	//request artwork image
	img_manager->get(QNetworkRequest(QUrl(artworks[0])));
	//play song
	m_player->setMedia(QUrl(song_urls[0]));
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

string LatestListener::url_encode(const string& str, bool space_to_plus) {
	std::ostringstream buf;
	buf << std::hex << std::setfill('0');
	
	typedef std::string::const_iterator Iterator;
	Iterator end = str.end();
	for(Iterator itr = str.begin(); itr != end; ++itr) {
		if((*itr >= 'a' && *itr <= 'z') || (*itr >= 'A' && *itr <= 'Z') || (*itr >= '0' && *itr <= '9') ||
			*itr == '-' || *itr == '.' || *itr == '_' || *itr == '~') {
			buf << *itr;
		} else if(space_to_plus && *itr == ' ') {
			buf << '+';
		} else {
			buf << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(*itr));
		}
	}
	return buf.str();
}