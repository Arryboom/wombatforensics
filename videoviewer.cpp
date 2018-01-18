#include "videoviewer.h"

// Copyright 2015 Pasquale J. Rinaldi, Jr.
// Distrubted under the terms of the GNU General Public License version 2

VideoViewer::VideoViewer(QWidget* parent) : QDialog(parent), ui(new Ui::VideoViewer)
{
    ui->setupUi(this);
    vplayer = new QMediaPlayer;
    videowidget = new QVideoWidget;
    vplayer->setVideoOutput(videowidget);
    ui->horizontalLayout->addWidget(videowidget);
    ui->pushButton->setText("Pause");
    ui->horizontalSlider->setEnabled(false);
    connect(ui->horizontalSlider, SIGNAL(sliderMoved(int)), this, SLOT(Seek(int)));
    connect(vplayer, SIGNAL(positionChanged(qint64)), this, SLOT(UpdateSlider(qint64)));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(PlayPause()));
    connect(vplayer, SIGNAL(durationChanged(qint64)), this, SLOT(SetDuration(qint64)));
    tskptr = &tskobj;
    tskptr->readimginfo = NULL;
    tskptr->readfsinfo = NULL;
    tskptr->readfileinfo = NULL;
    this->hide();
}

VideoViewer::~VideoViewer()
{
}

void VideoViewer::Seek(int pos)
{
    vplayer->setPosition(pos*1000LL);
}

void VideoViewer::PlayPause()
{
    if(vplayer->state() == QMediaPlayer::PlayingState)
    {
        ui->pushButton->setText("Play");
        vplayer->pause();
    }
    else
    {
        ui->pushButton->setText("Pause");
        vplayer->play();
    }
}

void VideoViewer::UpdateSlider(qint64 pos)
{
    ui->horizontalSlider->setValue(int(pos/1000LL));
    ui->label->setText(QTime(0, 0, 0).addMSecs(pos).toString("HH:mm:ss"));
}

void VideoViewer::SetDuration(qint64 pos)
{
    ui->horizontalSlider->setRange(0, int(pos/1000LL));
    ui->label3->setText(QTime(0, 0, 0).addMSecs(pos).toString("HH:mm:ss"));
}

void VideoViewer::GetVideo(const QModelIndex &index)
{
    QString tmpstr = "";
    QStringList evidlist;
    evidlist.clear();
    std::vector<std::string> pathvector;
    pathvector.clear();
    QDir eviddir = QDir(wombatvariable.tmpmntpath);
    QStringList evidfiles = eviddir.entryList(QStringList("*.evid." + index.sibling(index.row(), 0).data().toString().split("-").at(0).mid(1)), QDir::NoSymLinks | QDir::Files);
    wombatvariable.evidencename = evidfiles.at(0);
    QFile evidfile(wombatvariable.tmpmntpath + wombatvariable.evidencename.split(".evid").at(0) + ".evid." + index.sibling(index.row(), 0).data().toString().split("-").at(0).mid(1));
    evidfile.open(QIODevice::ReadOnly);
    tmpstr = evidfile.readLine();
    evidlist = tmpstr.split(",");
    tskptr->partcount = evidlist.at(3).split("|").size();
    evidfile.close();
    for(int i = 0; i < evidlist.at(3).split("|").size(); i++)
        pathvector.push_back(evidlist.at(3).split("|").at(i).toStdString());
    tskptr->imagepartspath = (const char**)malloc(pathvector.size()*sizeof(char*));
    for(int i = 0; i < pathvector.size(); i++)
        tskptr->imagepartspath[i] = pathvector[i].c_str();
    tskptr->readimginfo = tsk_img_open(tskptr->partcount, tskptr->imagepartspath, TSK_IMG_TYPE_DETECT, 0);
    if(tskptr->readimginfo == NULL)
    {
        qDebug() << tsk_error_get_errstr();
        //LogMessage("Image opening error");
    }
    free(tskptr->imagepartspath);
    tmpstr = "";
    QStringList partlist;
    partlist.clear();
    QStringList partfiles = eviddir.entryList(QStringList(wombatvariable.evidencename.split(".evid").at(0) + ".part." + index.sibling(index.row(), 0).data().toString().split("-").at(2).mid(1)), QDir::NoSymLinks | QDir::Files);
    QFile partfile(wombatvariable.tmpmntpath + partfiles.at(0));
    partfile.open(QIODevice::ReadOnly);
    tmpstr = partfile.readLine();
    partfile.close();
    partlist = tmpstr.split(",");
    tskptr->readfsinfo = tsk_fs_open_img(tskptr->readimginfo, partlist.at(4).toULongLong(), TSK_FS_TYPE_DETECT);
    tskptr->readfileinfo = tsk_fs_file_open_meta(tskptr->readfsinfo, NULL, curobjaddr);
    if(tskptr->readfileinfo->meta != NULL)
    {
        char* ibuffer = new char[tskptr->readfileinfo->meta->size];
        ssize_t imglen = tsk_fs_file_read(tskptr->readfileinfo, 0, ibuffer, tskptr->readfileinfo->meta->size, TSK_FS_FILE_READ_FLAG_NONE);
        QByteArray filedata = QByteArray::fromRawData(ibuffer, imglen);
        QBuffer* filebuf = new QBuffer(vplayer);
        filebuf->setData(filedata);
        filebuf->open(QIODevice::ReadOnly);
        vplayer->setMedia(QMediaContent(), filebuf);
    }
}

void VideoViewer::ShowVideo(const QModelIndex &index)
{
    this->show();
    ui->label_2->setVisible(true);
    curobjaddr = index.sibling(index.row(), 0).data().toString().split("-f").at(1).toULongLong();
    GetVideo(index);
    //QtConcurrent::run(this, &VideoViewer::GetVideo, index);
    vplayer->play();
    ui->label_2->setVisible(false);
}
void VideoViewer::mousePressEvent(QMouseEvent* e)
{
    if(e->type() == QEvent::MouseButtonPress)
    {
        vplayer->stop();
        this->hide();
    }
}
