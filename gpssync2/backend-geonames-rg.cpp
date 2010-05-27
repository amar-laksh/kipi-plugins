
#include "backend-geonames-rg.moc"


//KDE includes
#include <kio/scheduler.h>
#include <kurl.h>
#include <kio/jobclasses.h>
#include <kdebug.h>
#include <QMessageBox>
#include <kio/job.h>
#include <klocale.h>

//local includes
#include "backend-geonames-rg.h"
#include "gpsreversegeocodingwidget.h"
#include "backend-rg.h"

//Qt includes
#include <QDomDocument>


namespace KIPIGPSSyncPlugin
{


class GeonamesInternalJobs {

public:

    GeonamesInternalJobs()
    : request(),
      data(),
      kioJob(0)
    {
    }

    RGInfo request;
    QByteArray data;
    KIO::Job* kioJob;
};


class BackendGeonamesRGPrivate
{

public:
    
    BackendGeonamesRGPrivate()
    :jobs()
    {
    }
    

    QList<GeonamesInternalJobs> jobs;

};

BackendGeonamesRG::BackendGeonamesRG(QObject* const parent)
: RGBackend(parent), d(new BackendGeonamesRGPrivate())
{
    
    
}

BackendGeonamesRG::~BackendGeonamesRG()
{
    delete d;
}

void BackendGeonamesRG::nextPhoto()
{

}

void BackendGeonamesRG::callRGBackend(QList<RGInfo> rgList, QString language)
{

    //TODO: Remove dublicates from rgList to mergedQuery
    QList<RGInfo> mergedQuery = rgList;
    d->jobs.clear();


    for( int i = 0; i < mergedQuery.count(); i++){

        GeonamesInternalJobs newJob;
        newJob.request = mergedQuery.at(i);
//        newJob.data.append("");    
  
        KUrl jobUrl("http://ws.geonames.org/findNearbyPlaceName");
        jobUrl.addQueryItem("lat", mergedQuery.at(i).coordinates.latString());
        jobUrl.addQueryItem("lng", mergedQuery.at(i).coordinates.lonString());

        newJob.kioJob = KIO::get(jobUrl, KIO::NoReload, KIO::HideProgressInfo);
        d->jobs<<newJob;
        d->jobs[i].kioJob = newJob.kioJob;

        kDebug()<<d->jobs[0].kioJob;

        connect(newJob.kioJob, SIGNAL(data(KIO::Job*, const QByteArray&)), 
                this, SLOT(dataIsHere(KIO::Job*,const QByteArray &)));
        connect(newJob.kioJob, SIGNAL(result(KJob*)),
                this, SLOT(slotResult(KJob*)));    
    

    }

}

void BackendGeonamesRG::dataIsHere(KIO::Job* job, const QByteArray & data)
{

    
    for(int i = 0; i < d->jobs.count(); ++i){

        if(d->jobs.at(i).kioJob == job){

            
            d->jobs[i].data.append(data);
            break;

        }
        

    }

}


void BackendGeonamesRG::makeDOMFromXML(QString xmlData)
{

    QString resultString;

    QDomDocument doc;
    doc.setContent(xmlData);

    QDomElement docElem =  doc.documentElement();

    QDomNode n = docElem.firstChild().firstChild();

   while(!n.isNull()){

        QDomElement e = n.toElement();
        if(!e.isNull()){

            QDomText t = n.toText();

            resultString.append(e.tagName() + ":" + e.text() + "\n");

        }

        n = n.nextSibling();

    }

    kDebug()<<resultString;

}


void BackendGeonamesRG::slotResult(KJob* kJob)
{



    KIO::Job* kioJob = qobject_cast<KIO::Job*>(kJob);


    for(int i = 0;i < d->jobs.count(); ++i){

        if(d->jobs.at(i).kioJob == kioJob){

            //kDebug()<<d->jobs.at(i).data;
           
            QString dataString(d->jobs.at(i).data);
            dataString.remove(0,55);
            dataString.chop(1);

            makeDOMFromXML(dataString);

 
            break;
        }

    }
    

}



} //KIPIGPSSyncPlugin
