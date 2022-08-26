#include "transcriptgenerator.h"
#include <QFileDialog>
#include<QStandardPaths>
#include<QDir>
#include<QDebug>
#include <QProgressBar>
#include<QApplication>
#include<QWidget>
#include<QMessageBox>

TranscriptGenerator::TranscriptGenerator(QObject *parent,    QUrl *fileUr)
    : QObject{parent}
{

    fileUrl=fileUr;
    qInfo()<<*fileUrl;
}

TranscriptGenerator::TranscriptGenerator(QUrl *fileUr)
{

    fileUrl=fileUr;
    qInfo()<<*fileUrl;
}

void TranscriptGenerator::Upload_and_generate_Transcript()
{
    QFile myfile(fileUrl->toLocalFile());
    QFileInfo fileInfo(myfile);
    QString filename(fileInfo.fileName());

    if(filename.isEmpty()) return;
    qInfo()<<"hi";
    QString str = myfile.fileName();
    QDir audioFolder(".Audio");
    audioFolder.removeRecursively();
    if(!audioFolder.exists()){
        qInfo()<<"not here";
        qInfo()<<audioFolder.path();
        qInfo()<<audioFolder.mkpath(".");
    }

//    QString pier=fileInfo.dir().path()+"/Audio";
//    QDir dir(pier);
//    if(!dir.exists()){
//        qInfo()<<dir.exists();
//        qInfo()<<dir.mkpath(".");
//    }
    //qInfo()<<fileInfo.completeBaseName()<<".mp3";
//    QString audioFile=fileInfo.dir().path()+"/Audio/"+fileInfo.completeBaseName()+".mp3";
    QString audioFile=".Audio/"+fileInfo.completeBaseName()+".mp3";
    QFile d2(audioFile);

    if(!d2.exists()){
        QProgressBar progressBar;
        progressBar.setMinimum(0);
        progressBar.setMaximum(100);
        progressBar.setValue(10);
        progressBar.show();
        progressBar.raise();
        progressBar.activateWindow();
//        QString audiofilelocation=fileInfo.dir().path().replace(" ", "\\ ")+"/Audio/"
//            +fileInfo.completeBaseName().replace(" ", "\\ ")+".mp3";
        QString audiofilelocation=audioFolder.path().replace(" ", "\\ ")
                +"/"
                +fileInfo.completeBaseName().replace(" ", "\\ ")+".mp3";

        std::string pi=" ffmpeg -i " +str.replace(" ", "\\ ").toStdString()
            +" -vn -acodec libmp3lame -ac 1 -ab 160k -ar 16000 "+audiofilelocation.toStdString();

        int result = system(pi.c_str());
        qInfo()<<result;
//        QString pier2=fileInfo.dir().path()+"/Audio/mp3";
        QString pier2=".Audio/mp3";
        QString pier2x=pier2;
        QString pier3=pier2+"/waves";
        QDir d3(pier2);
        if(!d3.exists()){
            qInfo()<<d3.mkpath(".");
        }
        pier2=d3.path();

         progressBar.setValue(50);

//        std::string pi2="mp3splt -s -p th=-40,min=0.4,rm=50_50,trackjoin=2.5 "
//            +audiofilelocation.toStdString()
//            +" -o @f-@n -d "
//            +pier2.replace(" ", "\\ ").toStdString();
         std::string pi2="mp3splt -s -p th=-40,min=0.4,rm=50_50,trackjoin=2.5 "
             +audiofilelocation.toStdString()
             +" -o @f-@n -d "
             +pier2.replace(" ", "\\ ").toStdString();
        result = system(pi2.c_str());
        qInfo()<<result;

        progressBar.setValue(80);

//        std::string pi3="mp3splt -s -P -p th=-40,min=0.4,rm=50_50,trackjoin=2.5 -o @f_@m:@s.@h_@M:@S.@H "
//            +audiofilelocation.toStdString()
//            + " > "
//            +fileInfo.dir().path().replace(" ", "\\ ").toStdString()+"/Audio/text";
        std::string pi3="mp3splt -s -P -p th=-40,min=0.4,rm=50_50,trackjoin=2.5 -o @f_@m:@s.@h_@M:@S.@H "
            +audiofilelocation.toStdString()
            + " > "
            +d3.path().replace(" ", "\\ ").toStdString()+"/text";
        result = system(pi3.c_str());
        qInfo()<<result;


        qInfo()<<pier3;
        QDir d4(pier3);
        if(!d4.exists()){
            qInfo()<<d4.mkpath(".");
        }
        pier3=d4.path();
        pier3.replace(" ", "\\ ");


        QDir mp3folder(pier2x);
        QStringList mp3s = mp3folder.entryList(QStringList() <<  "*.mp3",QDir::Files);
        foreach(QString filename, mp3s) {

            int lastPoint = filename.lastIndexOf(".");
            QString fileNameNoExt = filename.left(lastPoint);



            filename.replace(" ", "\\ ");
            fileNameNoExt.replace(" ", "\\ ");


                    std::string pi5="sox "
                                    +pier2.toStdString()
                                    +"/"
                                    +filename.toStdString()
                                    +" "
                                    +pier3.toStdString()
                                    +"/"
                                    +fileNameNoExt.toStdString()
                                    +".wav";
                    result = system(pi5.c_str());
                    qInfo()<<result;
        }

        progressBar.setValue(100);
        progressBar.hide();
    }





}
