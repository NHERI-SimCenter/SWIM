/* *****************************************************************************
Copyright (c) 2018-2019, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS 
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, 
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */


#include "mainwindow.h"
 //#include "ui_mainwindow.h"

#include <iostream>

// layouts
#include <HeaderWidget.h>
#include <FooterWidget.h>
//#include "sectiontitle.h"

// custom
//#include <SimpleSpreadsheetWidget.h>
#include <qcustomplot/qcustomplot.h>
//#include "experiment.h"
#include "resp.h"
#include "historywidget.h"
#include "deformwidget.h"
#include "responsewidget.h"
#include "hysteresiswidget.h"

// widget libraries
#include <QtGui>
#include <QtWidgets>
#include <QtCore>
#include <QDebug>
#include <QGuiApplication>

//#include <Response.h>
//#include <Information.h>
// other
#include <math.h>

// files
#include <QFile>
//#include <QtSql>
//#include <QMap>

//styles
//#include <QGroupBox>
//#include <QFrame>


// fortran libraries
#ifdef _FORTRAN_LIBS
#include <BandGenLinSOE.h>
#include <BandGenLinLapackSolver.h>
#endif


#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QHostInfo>


//---------------------------------------------------------------
// Misc. functions
QCheckBox *addCheck(QString text, QString unitText = QObject::tr(""),
           QGridLayout *gridLay =0, int row =-1, int col =-1, int nrow =1, int ncol =1);
QComboBox *addCombo(QString text, QStringList items, QString *unitText =0,
           QGridLayout *gridLay =0, int row =-1, int col =-1, int nrow =1, int ncol =1);
QDoubleSpinBox *addDoubleSpin(QString text,QString *unitText =0,
           QGridLayout *gridLay =0, int row =-1, int col =-1, int nrow =1, int ncol =1);
QSpinBox *addSpin(QString text, QString *unitText =0,
           QGridLayout *gridLay =0, int row =-1, int col =-1, int nrow =1, int ncol =1);
QLabel *addLabel(QString text,
           QGridLayout *gridLay =0, int row =-1, int col =-1, int nrow =1, int ncol =1);
QLineEdit *addLineEdit(QString text, QString labelText,
           QGridLayout *gridLay =0, int row =-1, int col =-1, int nrow =1, int ncol =1);


//---------------------------------------------------------------
// new window
QWidget *createNewWindow(QString title)
{
    QWidget *window = new QWidget;
    window->show();
    window->setWindowTitle(title);

    return window;
}

// constructor
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{

    //
    // user settings
    //

    /***************************************
     removing so user remains anonymous
    QSettings settings("SimCenter", "uqFEM");
    QVariant savedValue = settings.value("uuid");

    QUuid uuid;
    if (savedValue.isNull()) {
        uuid = QUuid::createUuid();
        settings.setValue("uuid",uuid);
    } else
        uuid =savedValue.toUuid();
    ******************************************/

    theSteel.a1 = 0.0;
    theSteel.a3 = 0.0;

    //ui->setupUi(this);
    pause = false;
    // constants
    pi = 4*atan(1);

    // create layout and actions
    mainLayout = new QHBoxLayout();
    largeLayout = new QVBoxLayout();
    createActions();

    // Experiment image
    experimentImage = new QLabel();
   // experimentImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    experimentImage->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    
    // create header
    createHeaderBox();

    // load
    // loadAISC();

    copyDir(QDir(qApp->applicationDirPath()).filePath("ExampleFiles/wallDemo1"),QDir(rootDir).filePath("wallDemo1"),true);
    copyDir(QDir(qApp->applicationDirPath()).filePath("ExampleFiles/wallDemo2"),QDir(rootDir).filePath("wallDemo2"),true);
    expDirName = QDir(rootDir).filePath("wallDemo1");

    // create input / output panels
    createInputPanel();
    createOutputPanel();

    largeLayout->addLayout(mainLayout);

    // main widget set to screen size
    QWidget *widget = new QWidget();
    widget->setLayout(largeLayout);
    this->setCentralWidget(widget);

    // create footer
    createFooterBox();

    //
    // adjust size of application window to the available display
    //
    QRect rec = QGuiApplication::primaryScreen()->geometry();
    int height = this->height()<int(0.75*rec.height())?int(0.75*rec.height()):this->height();
    int width  = this->width()<int(0.85*rec.width())?int(0.85*rec.width()):this->width();
    this->resize(width, height);
    wwidth = int(round(width*0.5));
    wheight= int(round(height*0.9));
    //this->setMaximumHeight(100);


    // initialize data
    initialize();
    //reset();

    inExp->clear();
    for (int i=0; i<expNamesList.size(); i++)
    {
        inExp->addItem(expNamesList[i], expDirList[i]);
    }
    inExp->setCurrentIndex(currentExpInd);
    connect(inExp,SIGNAL(currentIndexChanged(int)), this, SLOT(inExp_currentIndexChanged(int)));


/*
    // access a web page which will increment the usage count for this tool
    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    manager->get(QNetworkRequest(QUrl("http://opensees.berkeley.edu/OpenSees/developer/swim/use.php")));
    //  manager->get(QNetworkRequest(QUrl("https://simcenter.designsafe-ci.org/multiple-degrees-freedom-analytics/")));


    QNetworkRequest request;
    QUrl host("http://www.google-analytics.com/collect");
    request.setUrl(host);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");

    // setup parameters of request
    QString requestParams;
    QUuid uuid = QUuid::createUuid();
    QString hostname = QHostInfo::localHostName() + "." + QHostInfo::localDomainName();

    requestParams += "v=1"; // version of protocol
    requestParams += "&tid=UA-126287558-1"; // Google Analytics account
    requestParams += "&cid=" + uuid.toString(); // unique user identifier
    requestParams += "&t=event";  // hit type = event others pageview, exception
    requestParams += "&an=SWIM";   // app name
    requestParams += "&av=1.0.0"; // app version
    requestParams += "&ec=SWIM";   // event category
    requestParams += "&ea=start"; // event action

    // send request via post method
    manager->post(request, requestParams.toStdString().c_str());
*/
}

//---------------------------------------------------------------
MainWindow::~MainWindow()
{
  //    delete ui;
    delete AISCshapes;

    // experiment
    delete time;
    delete expD;
    delete expP;

    // response
    delete Ux;
    delete Uy;
    delete q1;
    delete q2;
    delete q3;
}
//---------------------------------------------------------------
void MainWindow::initialize()
{
    // initialize loading
    numSteps = 2;
    pause = true;

    // experimental data
    expD = new QVector<double>(numSteps,0.);
    expP = new QVector<double>(numSteps,0.);
    time = new QVector<double>(numSteps,0.);
    dt = 0.1;

    // response
    Ux = new Resp();
    Uy = new Resp();
    q1 = new Resp();
    q2 = new Resp();
    q3 = new Resp();

    // slider
    slider->setValue(1);
/*
    // spinBox
    inNe->setValue(10);
    inNIP->setValue(10);
    inNbf->setValue(10);
    inNd->setValue(10);
    inNtw->setValue(10);
    inNtf->setValue(10);

    // initialize QComboBoxes
    inOrient->setCurrentIndex(1);
    inSxn->setCurrentIndex(1);
    inElType->setCurrentIndex(1);
    inElDist->setCurrentIndex(1);
    inIM->setCurrentIndex(1);
    inShape->setCurrentIndex(1);
    inMat->setCurrentIndex(1);
    in_conn1->setCurrentIndex(1);
    in_conn2->setCurrentIndex(1);

    // bools
    matDefault->setChecked(false);
    matAsymm->setChecked(true);
    matFat->setChecked(true);
    connSymm->setChecked(false);
    */

    eleSizeBEEdt->setValue(beLength);
    eleSizeWebEdt->setValue(webLength);
}

// reset
void MainWindow::reset()
{
    stop = false;
    pause = true;

    // remove experiment name
    //inExp->clear();


    // Load default experiments
    // inExp->setCurrentText("TCBF3_W8X28.json");
    if (inExp->count() != 0) {
        inExp->setCurrentIndex(0);
        this->loadExperimentalFile(inExp->itemData(0).toString());
        hPlot->plotModel();
    }
}

bool MainWindow::saveFile(const QString &fileName)
{
    //
    // open file
    //

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return false;
    }
    currentFile = fileName;


    //
    // create a json object, fill it in & then use a QJsonDocument
    // to write the contents of the object to the file in JSON format
    //

    QJsonObject json;
    QJsonObject element;
    QJsonObject section;
    QJsonObject material;
    QJsonObject connection;

    // Add element data
    element.insert(QStringLiteral("elementModel"), inElType->currentText());
    element.insert(QStringLiteral("workPointLength"), inLwp->value());
    element.insert(QStringLiteral("braceLength"), inL->value());
    element.insert(QStringLiteral("numSubElements"), inNe->value());
    element.insert(QStringLiteral("numIntegrationPoints"), inNIP->value());
    element.insert(QStringLiteral("camber"), inDelta->value());
    element.insert(QStringLiteral("subElDistribution"), inElDist->currentText());
    element.insert(QStringLiteral("integrationMethod"), inIM->currentText());
    element.insert(QStringLiteral("camberShape"), inShape->currentText());
    json.insert(QStringLiteral("element"), element);

    // Add section data
    section.insert(QStringLiteral("sectionType"), inSxn->currentText());
    section.insert(QStringLiteral("orientation"), inOrient->currentText());
    section.insert(QStringLiteral("nbf"), inNbf->value());
    section.insert(QStringLiteral("ntf"), inNtf->value());
    section.insert(QStringLiteral("nd"), inNd->value());
    section.insert(QStringLiteral("ntw"), inNtw->value());
    json.insert(QStringLiteral("section"), section);

    // Add material data
    material.insert(QStringLiteral("includeFatigue"), matFat->isChecked());
    material.insert(QStringLiteral("useDefaults"), matDefault->isChecked());
    material.insert(QStringLiteral("E"), inEs->value());
    material.insert(QStringLiteral("fy"), infy->value());
    // Add fatigue settings
    QJsonObject fatigue;
    fatigue.insert(QStringLiteral("m"), inm->value());
    fatigue.insert(QStringLiteral("e0"), ine0->value());
    fatigue.insert(QStringLiteral("emin"), inemin->value());
    fatigue.insert(QStringLiteral("emax"), inemax->value());
    material.insert("fatigue", fatigue);

    // Add material model settings
    QJsonObject materialModel;
    materialModel.insert("model", inMat->currentText());
    
    switch (inMat->currentIndex()) {
       // Uniaxial bi-linear material model
       case 0: {
	 QJsonObject kinematicHardening;
	 QJsonObject isotropicHardening;
	 // Kinematic hardening settings
	 kinematicHardening.insert(QStringLiteral("b"), inb->value());
	 // Isotropic hardening settings
	 isotropicHardening.insert(QStringLiteral("a1"), ina1->value());
	 isotropicHardening.insert(QStringLiteral("a2"), ina2->value());
	 isotropicHardening.insert(QStringLiteral("a3"), ina3->value());
	 isotropicHardening.insert(QStringLiteral("a4"), ina4->value());
	 // Add hardening settings to material model
	 materialModel.insert(QStringLiteral("kinematicHardening"), kinematicHardening);
	 materialModel.insert(QStringLiteral("isotropicHardening"), isotropicHardening); 
	 break;
       }

       // Uniaxial Giuffre-Menegotto-Pinto model
       case 1: {
	 QJsonObject kinematicHardening;
	 QJsonObject isotropicHardening;
	 QJsonObject hardeningTrans;
	 // Kinematic hardening settings
	 kinematicHardening.insert(QStringLiteral("b"), inb->value());
	 // Isotropic hardening settings
	 isotropicHardening.insert(QStringLiteral("a1"), ina1->value());
	 isotropicHardening.insert(QStringLiteral("a2"), ina2->value());
	 isotropicHardening.insert(QStringLiteral("a3"), ina3->value());
	 isotropicHardening.insert(QStringLiteral("a4"), ina4->value());
	 // Add elast to hardening transitions
	 hardeningTrans.insert(QStringLiteral("R0"), inR0->value());
	 hardeningTrans.insert(QStringLiteral("r1"), inR1->value());
	 hardeningTrans.insert(QStringLiteral("r2"), inR2->value());	 
	 // Add hardening settings to material model
	 materialModel.insert(QStringLiteral("kinematicHardening"), kinematicHardening);
	 materialModel.insert(QStringLiteral("isotropicHardening"), isotropicHardening);
	 materialModel.insert(QStringLiteral("hardeningTransitions"), hardeningTrans);
	 break;
       }
	 
       // Uniaxial asymmetric Giuffre-Menegotto-Pinto model
       case 2: {
	 QJsonObject kinematicHardening;
	 QJsonObject kinematicHardeningTension;
	 QJsonObject kinematicHardeningComp;
	 QJsonObject isotropicHardening;
	 QJsonObject isotropicHardeningTension;
	 QJsonObject isotropicHardeningComp;	 
	 // Kinematic hardening settings
	 kinematicHardeningTension.insert(QStringLiteral("b"), inbk->value());
	 kinematicHardeningTension.insert(QStringLiteral("R0"), inR0k->value());
	 kinematicHardeningTension.insert(QStringLiteral("r1"), inr1->value());
	 kinematicHardeningTension.insert(QStringLiteral("r2"), inr2->value());
	 kinematicHardeningComp.insert(QStringLiteral("b"), inbkc->value());
	 kinematicHardeningComp.insert(QStringLiteral("R0"), inR0kc->value());
	 kinematicHardeningComp.insert(QStringLiteral("r1"), inr1c->value());
	 kinematicHardeningComp.insert(QStringLiteral("r2"), inr2c->value());
	 kinematicHardening.insert(QStringLiteral("tension"), kinematicHardeningTension);
	 kinematicHardening.insert(QStringLiteral("compression"), kinematicHardeningComp);
	 // Isotropic hardening settings
	 isotropicHardeningTension.insert(QStringLiteral("b"), inbi->value());
	 isotropicHardeningTension.insert(QStringLiteral("rho"), inrhoi->value());
	 isotropicHardeningTension.insert(QStringLiteral("bl"), inbl->value());
	 isotropicHardeningTension.insert(QStringLiteral("Ri"), inRi->value());
	 isotropicHardeningTension.insert(QStringLiteral("lyp"), inlyp->value());
	 isotropicHardeningComp.insert(QStringLiteral("b"), inbic->value());
	 isotropicHardeningComp.insert(QStringLiteral("rho"), inrhoic->value());
	 isotropicHardeningComp.insert(QStringLiteral("bl"), inblc->value());
	 isotropicHardeningComp.insert(QStringLiteral("Ri"), inRic->value());
	 isotropicHardening.insert(QStringLiteral("tension"), isotropicHardeningTension);
	 isotropicHardening.insert(QStringLiteral("compression"), isotropicHardeningComp);
	 // Add hardering settings to material model
	 materialModel.insert(QStringLiteral("asymmetric"), matAsymm->isChecked());
	 materialModel.insert(QStringLiteral("kinematicHardening"), kinematicHardening);
	 materialModel.insert(QStringLiteral("isotropicHardening"), isotropicHardening);
	 break;
       }
    }

    // Add material model to material
    material.insert(QStringLiteral("materialModel"), materialModel);
    json.insert(QStringLiteral("material"), material);

    // Add connection data
    QJsonObject connection_1;
    QJsonObject connection_2;
    connection_1.insert(QStringLiteral("model"), in_conn1->currentText());
    connection_1.insert(QStringLiteral("gussetLength"), inl_conn1->value());
    connection_1.insert(QStringLiteral("A"), inRigA_conn1->value());
    connection_1.insert(QStringLiteral("I"), inRigI_conn1->value());
    connection_2.insert(QStringLiteral("model"), in_conn2->currentText());
    connection_2.insert(QStringLiteral("gussetLength"), inl_conn2->value());
    connection_2.insert(QStringLiteral("A"), inRigA_conn2->value());
    connection_2.insert(QStringLiteral("I"), inRigI_conn2->value());

    connection.insert(QStringLiteral("connection1"), connection_1);
    connection.insert(QStringLiteral("connection2"), connection_2);
    connection.insert(QStringLiteral("symmetricConnections"), connSymm->isChecked());
    
    json.insert(QStringLiteral("connections"), connection);
      
    // Add test information
    QJsonObject test;
    QJsonArray axialDeformation;
    QJsonArray axialForce;
    QJsonArray timeSteps;
    
    for (int i = 0; i < time->size(); ++i) {
      axialDeformation.push_back((*expD)[i]);
      axialForce.push_back((*expP)[i]);
      timeSteps.push_back((*time)[i]);
    }
    
    test.insert(QStringLiteral("type"), experimentType);
    test.insert(QStringLiteral("axialDef"), axialDeformation);
    test.insert(QStringLiteral("axialForce"), axialForce);
    test.insert(QStringLiteral("timeSteps"), timeSteps);
    json.insert(QStringLiteral("test"), test);   

    QJsonDocument doc(json);
    file.write(doc.toJson());

    // close file
    file.close();

    return true;
}

// read file
void MainWindow::loadFile(const QString &fileName)
{
    // open files
    QFile mFile(fileName);

    // open warning
    if (!mFile.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), mFile.errorString()));
        return;
    }

    // place file contents into json object
    QString mText = mFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(mText.toUtf8());
    if (doc.isNull() || doc.isEmpty()) {
        QMessageBox::warning(this, tr("Application"),
                tr("Error loading file: not a JSON file or is empty."));
        return;
    }
    QJsonObject jsonObject = doc.object();
    QJsonValue json;    

    // Read input JSON for saved analysis
    if (jsonObject["brace"].isNull() || jsonObject["brace"].isUndefined()) {
      // Load element data
      json = jsonObject["element"];
      if (json.isNull() || json.isUndefined())
        QMessageBox::warning(this, "Warning","Element data not specified.");
      else {
	QJsonObject theData = json.toObject();
	inElType->setCurrentText(theData["elementModel"].toString());
	inLwp->setValue(theData["workPointLength"].toDouble());
	inL->setValue(theData["braceLength"].toDouble());
	inNe->setValue(theData["numSubElements"].toInt());
	inNIP->setValue(theData["numIntegrationPoints"].toInt());
	inDelta->setValue(theData["camber"].toDouble());
	inElDist->setCurrentText(theData["subElDistribution"].toString());
	inIM->setCurrentText(theData["integrationMethod"].toString());
	inShape->setCurrentText(theData["camberShape"].toString());	
      }

      // Load section data
      json = jsonObject["section"];
      if (json.isNull() || json.isUndefined())
        QMessageBox::warning(this, "Warning","Section data not specified.");
      else {
	QJsonObject theData = json.toObject();
	inSxn->setCurrentText(theData["sectionType"].toString());
	inOrient->setCurrentText(theData["orientation"].toString());
	inNbf->setValue(theData["nbf"].toInt());
	inNtf->setValue(theData["ntf"].toInt());
	inNd->setValue(theData["nd"].toInt());
	inNtw->setValue(theData["ntw"].toInt());
      }

      // Load material data
      json = jsonObject["material"];
      if (json.isNull() || json.isUndefined())
        QMessageBox::warning(this, "Warning","Material data not specified.");
      else {
	QJsonObject theData = json.toObject();
	QJsonObject theOtherData;
	QJsonObject theOtherOtherData;
	matFat->setChecked(theData["includeFatigue"].toBool());
	matDefault->setChecked(theData["useDefaults"].toBool());
	inEs->setValue(theData["E"].toDouble());
	infy->setValue(theData["fy"].toDouble());
	// Set fatigue data
	if (theData["fatigue"].isNull() || theData["fatigue"].isUndefined()) {
	  QMessageBox::warning(this, "Warning","Fatigue data not specified.");	  
	} else {
	  theOtherData = theData["fatigue"].toObject();
	  inm->setValue(theOtherData["m"].toDouble());
	  ine0->setValue(theOtherData["e0"].toDouble());
	  inemin->setValue(theOtherData["emin"].toDouble());
	  inemax->setValue(theOtherData["emax"].toDouble());	  
	}
	// Set material model
	if (theData["materialModel"].isNull() || theData["materialModel"].isUndefined()) {
	  QMessageBox::warning(this, "Warning","Material model data not specified.");	  	  
	} else {
	  theOtherData = theData["materialModel"].toObject();
	  inMat->setCurrentText(theOtherData["model"].toString());	  
	}

	switch (inMat->currentIndex()) {
	  // Uniaxial bi-linear material model	  
	  case 0: {
	    // Kinematic hardening
	    if (theOtherData["kinematicHardening"].isNull() || theOtherData["kinematicHardering"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Kinematic hardening data not specified.");	  	  	      
	    } else {
	      theOtherOtherData = theOtherData["kinematicHardening"].toObject();
	      inb->setValue(theOtherOtherData["b"].toDouble());	      
	    }
	    // Isotropic hardening
	    if (theOtherData["isotropicHardening"].isNull() || theOtherData["isotropicHardering"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Isotropic hardening data not specified.");	      
	    } else {
	      theOtherOtherData = theOtherData["isotropicHardening"].toObject();
	      ina1->setValue(theOtherOtherData["a1"].toDouble());
	      ina2->setValue(theOtherOtherData["a2"].toDouble());
	      ina3->setValue(theOtherOtherData["a3"].toDouble());
	      ina4->setValue(theOtherOtherData["a4"].toDouble());	      
	    }
	    break;
	  }

	  // Uniaxial Giuffre-Menegotto-Pinto model	    
	  case 1: {
	    // Kinematic hardening
	    if (theOtherData["kinematicHardening"].isNull() || theOtherData["kinematicHardering"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Kinematic hardening data not specified.");	  	  	      
	    } else {
	      theOtherOtherData = theOtherData["kinematicHardening"].toObject();
	      inb->setValue(theOtherOtherData["b"].toDouble());	      
	    }
	    // Isotropic hardening
	    if (theOtherData["isotropicHardening"].isNull() || theOtherData["isotropicHardering"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Isotropic hardening data not specified.");	      
	    } else {
	      theOtherOtherData = theOtherData["isotropicHardening"].toObject();
	      ina1->setValue(theOtherOtherData["a1"].toDouble());
	      ina2->setValue(theOtherOtherData["a2"].toDouble());
	      ina3->setValue(theOtherOtherData["a3"].toDouble());
	      ina4->setValue(theOtherOtherData["a4"].toDouble());	      
	    }
	    // Hardening transitions
	    if (theOtherData["hardeningTransitions"].isNull() || theOtherData["hardeningTransistions"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Hardening transitions data not specified.");	      	      
	    } else {
	      theOtherOtherData = theOtherData["hardeningTransitions"].toObject();
	      inR0->setValue(theOtherOtherData["R0"].toDouble());
	      inR1->setValue(theOtherOtherData["r1"].toDouble());
	      inR2->setValue(theOtherOtherData["r2"].toDouble());	      
	    }
	    break;
	  }

          // Uniaxial asymmetric Giuffre-Menegotto-Pinto model	    
	  case 2: {
	    // Kinematic hardening
	    if (theOtherData["kinematicHardening"].isNull() || theOtherData["kinematicHardering"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Kinematic hardening data not specified.");	  	  	      
	    } else {
	      // Tension
	      theOtherOtherData = theOtherData["kinematicHardening"].toObject();
	      if (theOtherOtherData["tension"].isNull() || theOtherOtherData["tension"].isUndefined()) {
		QMessageBox::warning(this, "Warning","Kinematic hardening tension data not specified.");
	      } else {
		QJsonObject lottaData = theOtherOtherData["tension"].toObject();
		inbk->setValue(lottaData["b"].toDouble());
		inR0k->setValue(lottaData["R0"].toDouble());
		inr1->setValue(lottaData["r1"].toDouble());
		inr2->setValue(lottaData["r2"].toDouble());
	      }
	      // Compression
	      if (theOtherOtherData["compression"].isNull() || theOtherOtherData["compression"].isUndefined()) {
		QMessageBox::warning(this, "Warning","Kinematic hardening compression data not specified.");
	      } else {
		QJsonObject lottaData = theOtherOtherData["compression"].toObject();
		inbkc->setValue(lottaData["b"].toDouble());
		inR0kc->setValue(lottaData["R0"].toDouble());
		inr1c->setValue(lottaData["r1"].toDouble());
		inr2c->setValue(lottaData["r2"].toDouble());		
	      } 
	    }
	    // Isotropic hardening
	    if (theOtherData["isotropicHardening"].isNull() || theOtherData["isotropicHardering"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Isotropic hardening data not specified.");	      
	    } else {
	      // Tension
	      theOtherOtherData = theOtherData["isotropicHardening"].toObject();
	      if (theOtherOtherData["tension"].isNull() || theOtherOtherData["tension"].isUndefined()) {
		QMessageBox::warning(this, "Warning","Isotropic hardening tension data not specified.");
	      } else {
		QJsonObject lottaData = theOtherOtherData["tension"].toObject();
		inbi->setValue(lottaData["b"].toDouble());
		inrhoi->setValue(lottaData["rho"].toDouble());
		inbl->setValue(lottaData["bl"].toDouble());
		inRi->setValue(lottaData["Ri"].toDouble());
		inlyp->setValue(lottaData["lyp"].toDouble());
	      }
	      // Compression
	      if (theOtherOtherData["compression"].isNull() || theOtherOtherData["compression"].isUndefined()) {
		QMessageBox::warning(this, "Warning","Isotropic hardening compression data not specified.");
	      } else {
		QJsonObject lottaData = theOtherOtherData["compression"].toObject();		
		inbic->setValue(lottaData["b"].toDouble());
		inrhoic->setValue(lottaData["rho"].toDouble());
		inblc->setValue(lottaData["bl"].toDouble());
		inRic->setValue(lottaData["Ri"].toDouble());
	      }
	    }
	    matAsymm->setChecked(theOtherData["asymmetric"].toBool());
	    break;
	  }

	  default: {
	    QMessageBox::warning(this, "Warning","Material model specified does not exist or specified incorrectly.");
	    break;
	  }
	}
      }

      // Load connection data
      json = jsonObject["connections"];
      if (json.isNull() || json.isUndefined())
        QMessageBox::warning(this, "Warning","Connection data not specified.");
      else {
	// Connection 1
	QJsonObject theData = json.toObject();
	QJsonObject connection;
	if (theData["connection1"].isNull() || theData["connection1"].isUndefined()) {
	  QMessageBox::warning(this, "Warning","Connection 1 data not specified.");	  
	} else {
	  connection = theData["connection1"].toObject();
	  in_conn1->setCurrentText(connection["model"].toString());
	  inl_conn1->setValue(connection["gussetLength"].toDouble());
	  inRigA_conn1->setValue(connection["A"].toDouble());
	  inRigI_conn1->setValue(connection["I"].toDouble());
	}
	// Connection 2
	if (theData["connection2"].isNull() || theData["connection2"].isUndefined()) {
	  QMessageBox::warning(this, "Warning","Connection 2 data not specified.");	  
	} else {
	  connection = theData["connection2"].toObject();
	  in_conn2->setCurrentText(connection["model"].toString());
	  inl_conn2->setValue(connection["gussetLength"].toDouble());
	  inRigA_conn2->setValue(connection["A"].toDouble());
	  inRigI_conn2->setValue(connection["I"].toDouble());	  
	}

	connSymm->setChecked(theData["symmetricConnections"].toBool());
      }

    } else {
        qDebug() << "HERE IN LOAD";

      // read brace
      json = jsonObject["brace"];

      // load brace data
      if (json.isNull() || json.isUndefined())
        QMessageBox::warning(this, "Warning","Brace data not specified.");

      else {
        QJsonObject theData = json.toObject();

        // brace section
        if (theData["sxn"].isNull() || theData["sxn"].isUndefined())
	  QMessageBox::warning(this, "Warning","Section not specified.");

        else {
	  QString text = theData["sxn"].toString();
	  int index = inSxn->findText(text);

	  if (index != -1) {
	    inSxn->setCurrentIndex(index);

	  } else
	    QMessageBox::warning(this, "Warning","Loaded section not in current AISC Shape Database.");
        }

        // orient
        if (theData["orient"].isNull() || theData["orient"].isUndefined())
	  QMessageBox::warning(this, "Warning","Brace: Orientation not specified.");

        else {
	  QString text = theData["orient"].toString();
	  int index = inOrient->findText(text);

	  if (index != -1) {
	    inOrient->setCurrentIndex(index);

	  } else
	    QMessageBox::warning(this, "Warning","Orientation not defined.");
        }

        // brace length
        if ((theData["width"].isNull())     || (theData["height"].isNull())
	    || theData["width"].isUndefined() || theData["height"].isUndefined())
	  QMessageBox::warning(this, "Warning","Brace length not specified.");

        else {
	  braceWidth = theData["width"].toDouble();
	  braceHeight = theData["height"].toDouble();

	  Lwp = sqrt(pow(braceWidth,2)+pow(braceHeight,2));
	  inLwp->setValue(Lwp);
	  angle = atan(braceHeight/braceWidth);
        }

        // fy
        if (theData["fy"].isNull() || theData["fy"].isUndefined())
	  QMessageBox::warning(this, "Warning","Brace: fy not specified.");

        else {
	  theSteel.fy=theData["fy"].toDouble();
	  infy->setValue(theSteel.fy);
        }

        // Es
        if (theData["E"].isNull() || theData["E"].isUndefined())
	  QMessageBox::warning(this, "Warning","Brace: Es not specified.");

        else {
	  theSteel.Es=theData["E"].toDouble();
	  inEs->setValue(theSteel.Es);
        }
      }

      // read connection-1
      json = jsonObject["connection-1"];

      // load brace data
      if (json.isNull() || json.isUndefined()) {
        QMessageBox::warning(this, "Warning","Connection-1 data not specified. \nConnection set to 5% workpoint length.");
        inl_conn1->setValue(0.05*Lwp);

      } else {
        QJsonObject theData = json.toObject();

	conn1.fy = theData["fy"].toDouble();
	conn1.Es = theData["E"].toDouble();
	conn1.tg = theData["tg"].toDouble();
	conn1.H = theData["H"].toDouble();
	conn1.W = theData["W"].toDouble();
	conn1.lb = theData["lb"].toDouble();
	conn1.lc = theData["lc"].toDouble();
	conn1.lbr = theData["lbr"].toDouble();
	conn1.eb = theData["eb"].toDouble();
	conn1.ec = theData["ec"].toDouble();
	
        // geometry
        if (theData["H"].isNull() || theData["H"].isUndefined()
	    || theData["W"].isNull() || theData["W"].isUndefined()
	    || theData["lb"].isNull() || theData["lb"].isUndefined()
	    || theData["lc"].isNull() || theData["lc"].isUndefined()
	    || theData["lbr"].isNull() || theData["lbr"].isUndefined()
	    || theData["eb"].isNull() || theData["eb"].isUndefined()
	    || theData["ec"].isNull() || theData["ec"].isUndefined())
	  {
            if (theData["L"].isNull() || theData["L"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Connection-1: not enough geometric information. \nConnection set to 5% workpoint length.");
	      inl_conn1->setValue(0.05*Lwp);

            } else {
	      double L2=theData["L"].toDouble();
	      inl_conn1->setValue(L2);
            }
	  }

        else {
	  double H=theData["H"].toDouble();
	  double W=theData["W"].toDouble();
	  double lb=theData["lb"].toDouble();
	  double lc=theData["lc"].toDouble();
	  double lbr=theData["lbr"].toDouble();
	  double eb=theData["eb"].toDouble();
	  double ec=theData["ec"].toDouble();

	  // estimate the whitmore width
	  double c = 0.5*sqrt(pow(W - lb,2)+pow(H - lc,2));
	  double lw = 2*lbr*tan(30*pi/180) + 2*c;

	  // calculate connection length
	  double w = lc*tan(angle)+c/sin(angle);
	  double L2;
	  if (W <= w)
	    L2 = W/cos(angle) - c*tan(angle) - lbr + ec/(2*cos(angle));
	  else
	    L2 = w/cos(angle) - c*tan(angle) - lbr + eb/(2*sin(angle));

	  inl_conn1->setValue(L2);
        }
      }

      // read connection-2
      json = jsonObject["connection-2"];

      // load brace data
      if (json.isNull() || json.isUndefined()) {
        QMessageBox::warning(this, "Warning","Connection-1 data not specified. \nConnection set to 5% workpoint length.");
        inl_conn1->setValue(0.05*Lwp);

      } else {
        QJsonObject theData = json.toObject();

	conn2.fy = theData["fy"].toDouble();
	conn2.Es = theData["E"].toDouble();
	conn2.tg = theData["tg"].toDouble();
	conn2.H = theData["H"].toDouble();
	conn2.W = theData["W"].toDouble();
	conn2.lb = theData["lb"].toDouble();
	conn2.lc = theData["lc"].toDouble();
	conn2.lbr = theData["lbr"].toDouble();
	conn2.eb = theData["eb"].toDouble();
	conn2.ec = theData["ec"].toDouble();

	// geometry
        if (theData["H"].isNull() || theData["H"].isUndefined()
	    || theData["W"].isNull() || theData["W"].isUndefined()
	    || theData["lb"].isNull() || theData["lb"].isUndefined()
	    || theData["lc"].isNull() || theData["lc"].isUndefined()
	    || theData["lbr"].isNull() || theData["lbr"].isUndefined()
	    || theData["eb"].isNull() || theData["eb"].isUndefined()
	    || theData["ec"].isNull() || theData["ec"].isUndefined())
	  {
            if (theData["L"].isNull() || theData["L"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Connection-1: not enough geometric information. \nConnection set to 5% workpoint length.");
	      inl_conn2->setValue(0.05*Lwp);

            } else {
	      double L2=theData["L"].toDouble();
	      inl_conn2->setValue(L2);
            }
	  }

        else {
	  double H=theData["H"].toDouble();
	  double W=theData["W"].toDouble();
	  double lb=theData["lb"].toDouble();
	  double lc=theData["lc"].toDouble();
	  double lbr=theData["lbr"].toDouble();
	  double eb=theData["eb"].toDouble();
	  double ec=theData["ec"].toDouble();

	  // estimate the whitmore width
	  double c = 0.5*sqrt(pow(W - lb,2)+pow(H - lc,2));
	  double lw = 2*lbr*tan(30*pi/180) + 2*c;

	  // calculate connection length
	  double w = lc*tan(angle)+c/sin(angle);
	  double L2;
	  if (W <= w)
	    L2 = W/cos(angle) - c*tan(angle) - lbr + ec/(2*cos(angle));
	  else
	    L2 = w/cos(angle) - c*tan(angle) - lbr + eb/(2*sin(angle));

	  inl_conn2->setValue(L2);
        }
      }

      // re-set symm connections
      connSymm->setCheckState(Qt::Unchecked);      
    }

    // read experiment loading
    json = jsonObject["test"];

    /*
    Experiment *exp = new Experiment();
    int ok = exp->inputFromJSON(json);

    if (ok == -1)
        QMessageBox::warning(this, "Warning","Experiment loading not specified.");
    else if (ok == -2)
        QMessageBox::warning(this, "Warning","Experiment history: axial deformation not specified.");
    else if (ok == -3)
        QMessageBox::warning(this, "Warning","Experiment history: axial force not specified.");
    else if (ok == -4)
        QMessageBox::warning(this, "Warning","Loaded axial force and deformation history are not the same length. Histories are truncated accordingly.");
    else if (ok == -5) {
      QMessageBox::warning(this, "Warning", "Experiment test type not specified."); 
    }
    */

    inExp->setCurrentIndex(0);

    /*
    setExp(exp);

    // Set test type
    experimentType = exp->getTestType();
    
    // name experiment
    QString name = fileName.section("/", -1, -1);

    // set as current    
    if (inExp->findText(name) == -1) {
      inExp->addItem(name, fileName);
    }
    inExp->setCurrentIndex(inExp->findText(name));
*/
    // close file
    mFile.close();
}

void MainWindow::loadWallExperimentalFile(const QString &fileName)
{
    // open files
    QFile mFile(fileName);

    // open warning
    if (!mFile.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), mFile.errorString()));
        return;
    }

    // close files
    mFile.close();
}

void MainWindow::deletePanels()
{
    hasResult = false;

    qDeleteAll(mainLayout->children());
    qDeleteAll(largeLayout->children());

    // create layout and actions
    mainLayout = new QHBoxLayout();
    largeLayout = new QVBoxLayout();




    inBox = nullptr;
    inLay = nullptr;




    // deleting wall wall
    theWall = nullptr;
    wallConfigLay = nullptr;
    wallSAMLay = nullptr;
    wallBIMLay = nullptr;

    floorSAMs.clear();
    floorBIMs.clear();
    steelBoxSAMs.clear();
    rebarBoxSAMs.clear();
    concreteBoxSAMs.clear();
    rcBoxSAMs.clear();
    matBIMs.clear();

    //QString expDirName = "/Users/simcenter/Codes/SimCenter/SWIM/data/wallDemo";
    //QString openseespath = "/Users/simcenter/Codes/OpenSees-March2019/bin/opensees";
    //QDir expDir = QDir(expDirName);

    matSelectorBIM.clear();

    numMatsBIM=0;
    numFloors=0;
    currentFloorIDbim = 0;
    wallLength=0. ;
    beLength=0. ;
    webLength=0. ;

    thePreprocessor = nullptr;




    rebarMaterials.clear();
    concreteMaterials.clear();
    rcMaterials.clear();

    rct1Edt.clear();
    rct2Edt.clear();
    rct3Edt.clear();
    rcMat1Edt.clear();
    rcMat2Edt.clear();
    rcMat3Edt.clear();

    concreteEEdt.clear();
    concretefpcEdt.clear();
    concretenuEdt.clear();
    concretebetaEdt.clear();
    concreteApEdt.clear();
    concreteAnEdt.clear();
    concreteBnEdt.clear();

    rebarSteelMatEdt.clear();
    rebarAngEdt.clear();


    matIDselector_rc = nullptr ;
    matIDselector_concrete = nullptr ;
    matIDselector_rebar = nullptr ;// index is rebarID
    matIDselector_steel = nullptr ;// index is steelID

    matIDList_rc.clear();
    matIDList_concrete.clear();
    matIDList_rebar.clear();
    matIDList_steel.clear();

    rcMatStrList.clear();

    //QStringList steelModelTypes = {"Steel01"};

    matIDList_rc_mat1.clear();
    matIDList_rc_mat2.clear();
    matIDList_rc_mat3.clear();


    steelTyeEdt.clear();
    steelEEdt.clear();
    steelfyEdt.clear();
    steelbEdt.clear();

    webRCselector.clear();
    beRCselector.clear();


    idFloorEdt_BIM = nullptr ;
    idFloorEdt_SAM = nullptr ;

    webLay = nullptr;

    eleSizeWebEdt = nullptr;
    eleSizeBEEdt = nullptr;

    progressbar = nullptr;

    openseesProcess = nullptr;

    openseesErrCount = 0;

    expWall = new ExperimentWall();

    openseespathEdt = nullptr;


    dispx.clear();
    dispy.clear() ;

    inBox = nullptr;

    //original
    addExp = nullptr;
    playButton = nullptr;

    beta = 0.5;
    An = 0.5;
    Ap = 0.5;
    Bn = 0.5;

    betaAI = 0.5;
    AnAI = 0.5;
    ApAI = 0.5;
    BnAI = 0.5;
    nLAI = 2;

    stepOpenSees = 0;





}


// read experimental file
void MainWindow::loadNew()
{
    /*
    deletePanels();
    createInputPanel();
    createOutputPanel();
    */
}

// read experimental file
void MainWindow::loadExperimentalFile(const QString &expDirNametmp)
{
    expDirName = expDirNametmp;
    loadNewBtn_clicked();

    /*
    // open files
    QFile mFile(expDirName);

    // open warning
    if (!mFile.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(expDirName), mFile.errorString()));
        return;
    }

    // place file contents into json object
    QString mText = mFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(mText.toUtf8());
    if (doc.isNull() || doc.isEmpty()) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Error loading file: not a JSON file or is empty."));
        return;
    }
    QJsonObject jsonObject = doc.object();
    QJsonValue json;

    // read brace
    json = jsonObject["brace"];

    // name experiment
    QString name = expDirName.section("/", -1, -1);

    // set as current
    if (inExp->findText(name) == -1) {
        inExp->addItem(name, expDirName);
    }
    inExp->setCurrentIndex(inExp->findText(name));


    // close file
    mFile.close();
    */
}

void MainWindow::loadNewBtn_clicked()
{
    deletePanels();
    createInputPanel();
    createOutputPanel();


    largeLayout->addLayout(mainLayout);

    // main widget set to screen size
    QWidget *widget = new QWidget();
    widget->setLayout(largeLayout);
    this->setCentralWidget(widget);

    // create footer
    createFooterBox();

    /*
    //
    // adjust size of application window to the available display
    //
    QRect rec = QGuiApplication::primaryScreen()->geometry();
    int height = this->height()<int(0.75*rec.height())?int(0.75*rec.height()):this->height();
    int width  = this->width()<int(0.85*rec.width())?int(0.85*rec.width()):this->width();
    this->resize(width, height);
    //this->setMaximumHeight(100);
    */


    // initialize data
    initialize();
    //reset();

    if(openseespath != "" && openseespathEdt->text()=="")
        openseespathEdt->setText(openseespath);

    inExp->clear();
    for (int i=0; i<expNamesList.size(); i++)
    {
        inExp->addItem(expNamesList[i], expDirList[i]);
    }
    inExp->setCurrentIndex(currentExpInd);
    connect(inExp,SIGNAL(currentIndexChanged(int)), this, SLOT(inExp_currentIndexChanged(int)));


    //inExp->setCurrentIndex(1);
    dPlot->setExpDir(inExp->itemData(inExp->currentIndex()).toString());
    updateSAMFile();
    dPlot->plotModel();




}



// load experiment
void MainWindow::addExp_clicked()
{
    // call load file function
    QString dirname = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                rootDir,
                                                QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);
    QDir directory(dirname);
    QString bimPath = directory.absoluteFilePath("BIM.json");
    QString evtPath = directory.absoluteFilePath("EVT.json");
    QString edpPath = directory.absoluteFilePath("EDP.json");
    QFile bimFile(bimPath);
    QFile evtFile(evtPath);
    QFile edpFile(edpPath);

    if(bimFile.exists() && evtFile.exists() && edpFile.exists())
    {
        QFileInfo info1(dirname);
        QString newExpName = info1.baseName();

        expNamesList.append(newExpName);
        expDirList.append(dirname);
        currentExpInd = expNamesList.size()-1;
        loadExperimentalFile(dirname);
    }else{
        QMessageBox::information(this,tr("SWIM Information"), "Select a directory containing BIM.json EVT.json and EDP.json", tr("I know."));
    }


}
//---------------------------------------------------------------
// load AISC Shape Database
void MainWindow::loadAISC()
{
    // resource name
    QString Filename = ":/MyResources/aisc-shapes-database-v15.0.csv";

    // open file
    QFile mFile(Filename);
    if (!mFile.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(Filename), mFile.errorString()));
        return;
    }

    // creat csv model
    //AISCshapes = new QTableWidget(this);
    AISCshapes = new QStandardItemModel(this);

    // read file
    QTextStream mText(&mFile);
    while (!mText.atEnd())
    {
        // read line-by-line
        QString line = mText.readLine();

        // add to list
        QList<QStandardItem *> newItem;
        for (QString item : line.split(","))
        {
            newItem.append(new QStandardItem(item));
        }

        // add to model
        AISCshapes->insertRow(AISCshapes->rowCount(),newItem);
    }

    // properties
    QList<QStandardItem *> AISCprop = AISCshapes->takeRow(0);
    for (int i = 1; i < AISCprop.size(); i++)
    {
        propList.append(AISCprop.at(i)->text());
    }

    // sections
    QList<QStandardItem *> AISCsxn = AISCshapes->takeColumn(0);
    for (int i = 0; i < AISCsxn.size(); i++)
    {
        sxnList.append(AISCsxn.at(i)->text());
    }

    // headers
    AISCshapes->setHorizontalHeaderLabels(propList);
    AISCshapes->setVerticalHeaderLabels(sxnList);

    // close file
    mFile.close();
}

// AISC button
void MainWindow::addAISC_clicked()
{
    QTableView *table = new QTableView;
    table->setModel(AISCshapes);
    table->setWindowTitle("AISC Shape Database");
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->selectionModel()->selectedRows();
    table->show();
    int index = inSxn->findText(sxn);
    if (index != -1) {
        table->selectRow(index);
    }

    //
    // adjust size to the available display
    //
    QRect rec = QGuiApplication::primaryScreen()->geometry();
    table->resize(int(0.65*rec.width()), int(0.65*rec.height()));
    qDebug() << .65*rec.width();

    // connect signals / slots
    connect(table->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(theAISC_sectionClicked(int)));
}

// select section in AISC table
void MainWindow::theAISC_sectionClicked(int row)
{
    inSxn->setCurrentIndex(row);
}
//---------------------------------------------------------------
// sxn Combo-Box
void MainWindow::inSxn_currentIndexChanged(int row)
{
    sxn = sxnList.at(row);

    // read sxn shape
    QString sxnString = AISCshapes->item(row,propList.indexOf("Type"))->text();

    // parse by enum class
    if (sxnString == "W" || sxnString == "M" || sxnString == "S" || sxnString == "HP")
        sxnType = sxnShape::W;
    else if (sxnString == "C" || sxnString == "MC")
        sxnType = sxnShape::C;
    else if (sxnString == "L" || sxnString == "2L")
        sxnType = sxnShape::L;
    else if (sxnString == "WT" || sxnString == "MT" || sxnString == "ST")
        sxnType = sxnShape::WT;
    else if (sxnString == "HSS")
        sxnType = sxnShape::HSS;
    else if (sxnString == "RND" || sxnString == "PIPE")
        sxnType = sxnShape::RND;

    // properties
    theSxn.A = AISCshapes->item(row,propList.indexOf("A"))->text().toDouble();
    theSxn.Ix = AISCshapes->item(row,propList.indexOf("Ix"))->text().toDouble();
    theSxn.Zx = AISCshapes->item(row,propList.indexOf("Zx"))->text().toDouble();
    theSxn.Sx = AISCshapes->item(row,propList.indexOf("Sx"))->text().toDouble();
    theSxn.rx = AISCshapes->item(row,propList.indexOf("rx"))->text().toDouble();
    theSxn.Iy = AISCshapes->item(row,propList.indexOf("Iy"))->text().toDouble();
    theSxn.Zy = AISCshapes->item(row,propList.indexOf("Zy"))->text().toDouble();
    theSxn.Sy = AISCshapes->item(row,propList.indexOf("Sy"))->text().toDouble();
    theSxn.ry = AISCshapes->item(row,propList.indexOf("ry"))->text().toDouble();

    // parse by sxn type
    switch (sxnType)
    {
    case sxnShape::W:
    case sxnShape::M:
    case sxnShape::S:
    case sxnShape::HP:
        theSxn.d = AISCshapes->item(row,propList.indexOf("d"))->text().toDouble();
        theSxn.bf = AISCshapes->item(row,propList.indexOf("bf"))->text().toDouble();
        theSxn.tw = AISCshapes->item(row,propList.indexOf("tw"))->text().toDouble();
        theSxn.tf = AISCshapes->item(row,propList.indexOf("tf"))->text().toDouble();
        theSxn.bftf = AISCshapes->item(row,propList.indexOf("bf/2tf"))->text().toDouble();
        theSxn.htw = AISCshapes->item(row,propList.indexOf("h/tw"))->text().toDouble();
        break;

    case sxnShape::C:
    case sxnShape::MC:
        QMessageBox::warning(this, "Warning","Section not yet implemented.");
        theSxn.d = AISCshapes->item(row,propList.indexOf("d"))->text().toDouble();
        theSxn.bf = AISCshapes->item(row,propList.indexOf("bf"))->text().toDouble();
        theSxn.tw = AISCshapes->item(row,propList.indexOf("tw"))->text().toDouble();
        theSxn.tf = AISCshapes->item(row,propList.indexOf("tf"))->text().toDouble();
        theSxn.bftf = AISCshapes->item(row,propList.indexOf("b/t"))->text().toDouble();
        theSxn.htw = AISCshapes->item(row,propList.indexOf("h/tw"))->text().toDouble();
        break;

    case sxnShape::L:
    case sxnShape::dL:
        QMessageBox::warning(this, "Warning","Section not yet implemented.");
        theSxn.d = AISCshapes->item(row,propList.indexOf("d"))->text().toDouble();
        theSxn.bf = AISCshapes->item(row,propList.indexOf("b"))->text().toDouble();
        theSxn.tw = AISCshapes->item(row,propList.indexOf("t"))->text().toDouble();
        theSxn.tf = AISCshapes->item(row,propList.indexOf("t"))->text().toDouble();
        theSxn.bftf = AISCshapes->item(row,propList.indexOf("b/t"))->text().toDouble();
        theSxn.htw = AISCshapes->item(row,propList.indexOf("b/t"))->text().toDouble();
        break;

    case sxnShape::WT:
    case sxnShape::MT:
    case sxnShape::ST:
        QMessageBox::warning(this, "Warning","Section not yet implemented.");
        theSxn.d = AISCshapes->item(row,propList.indexOf("d"))->text().toDouble();
        theSxn.bf = AISCshapes->item(row,propList.indexOf("bf"))->text().toDouble();
        theSxn.tw = AISCshapes->item(row,propList.indexOf("tw"))->text().toDouble();
        theSxn.tf = AISCshapes->item(row,propList.indexOf("tf"))->text().toDouble();
        theSxn.bftf = AISCshapes->item(row,propList.indexOf("bf/2tf"))->text().toDouble();
        theSxn.htw = AISCshapes->item(row,propList.indexOf("D/t"))->text().toDouble();
        break;

    case sxnShape::HSS:
        theSxn.d = AISCshapes->item(row,propList.indexOf("Ht"))->text().toDouble();
        theSxn.bf = AISCshapes->item(row,propList.indexOf("B"))->text().toDouble();
        theSxn.tw = AISCshapes->item(row,propList.indexOf("tdes"))->text().toDouble();
        theSxn.tf = AISCshapes->item(row,propList.indexOf("tdes"))->text().toDouble();
        theSxn.bftf = AISCshapes->item(row,propList.indexOf("b/tdes"))->text().toDouble();
        theSxn.htw = AISCshapes->item(row,propList.indexOf("h/tdes"))->text().toDouble();
        break;

    case sxnShape::RND:
    case sxnShape::PIPE:
        theSxn.d = AISCshapes->item(row,propList.indexOf("OD"))->text().toDouble();
        theSxn.bf = AISCshapes->item(row,propList.indexOf("OD"))->text().toDouble();
        theSxn.tw = AISCshapes->item(row,propList.indexOf("tdes"))->text().toDouble();
        theSxn.tf = AISCshapes->item(row,propList.indexOf("tdes"))->text().toDouble();
        theSxn.bftf = AISCshapes->item(row,propList.indexOf("D/t"))->text().toDouble();
        theSxn.htw = AISCshapes->item(row,propList.indexOf("D/t"))->text().toDouble();
        break;

    default:
        theSxn.d = AISCshapes->item(0,propList.indexOf("d"))->text().toDouble();
        theSxn.bf = AISCshapes->item(0,propList.indexOf("bf"))->text().toDouble();
        theSxn.tw = AISCshapes->item(0,propList.indexOf("tw"))->text().toDouble();
        theSxn.tf = AISCshapes->item(0,propList.indexOf("tf"))->text().toDouble();
        theSxn.bftf = AISCshapes->item(0,propList.indexOf("bf/2tf"))->text().toDouble();
        theSxn.htw = AISCshapes->item(0,propList.indexOf("h/tw"))->text().toDouble();
    };

    // call orientation
    inOrient_currentIndexChanged(inOrient->currentIndex());
}
// orientation Combo-Box
void MainWindow::inOrient_currentIndexChanged(int row)
{
    // if round section
    if (sxnType == sxnShape::RND) {
        inNbf->setEnabled(false);
        inNtf->setEnabled(false);
        inNd->setEnabled(true);
        inNtw->setEnabled(true);
        inNbf->setMinimum(0);
        inNtf->setMinimum(0);
        inNbf->setValue(0);
        inNtf->setValue(0);
    } else if (orient != inOrient->itemText(row)) {
        orient = inOrient->itemText(row);
        inNbf->setMinimum(1);
        inNtf->setMinimum(1);

        // switch fibers
        if (orient == "x-x") {
            inNd->setEnabled(true);
            inNtf->setEnabled(true);
            inNd->setValue(nbf);
            inNtf->setValue(ntw);
            inNbf->setValue(1);
            inNtw->setValue(1);
            inNbf->setEnabled(false);
            inNtw->setEnabled(false);

        } else if (orient == "y-y") {
            inNbf->setEnabled(true);
            inNtw->setEnabled(true);
            inNbf->setValue(nd);
            inNtw->setValue(ntf);
            inNtf->setValue(1);
            inNd->setValue(1);
            inNtf->setEnabled(false);
            inNd->setEnabled(false);
        }
    }

    // define section
    if (orient == "x-x")
    {
        theSxn.I = theSxn.Ix;
        theSxn.Z = theSxn.Zx;
        theSxn.S = theSxn.Sx;
        theSxn.r = theSxn.rx;
    }
    else if (orient == "y-y")
    {
        theSxn.I = theSxn.Iy;
        theSxn.Z = theSxn.Zy;
        theSxn.S = theSxn.Sy;
        theSxn.r = theSxn.ry;
    }

    // label
    dlabel->setText(QString("d = %1 in.").arg(theSxn.d));
    dlabel->setToolTip(tr("Depth"));
    bflabel->setText(QString("bf = %1 in.").arg(theSxn.bf));
    bflabel->setToolTip(tr("Flange width"));
    twlabel->setText(QString("tw = %1 in.").arg(theSxn.tw));
    twlabel->setToolTip(tr("Web thickness"));
    tflabel->setText(QString("tf = %1 in.").arg(theSxn.tf));
    tflabel->setToolTip(tr("Flange thickness"));
    Alabel->setText(QString("A = %1 in<sup>2</sup>").arg(theSxn.A));
    Alabel->setToolTip(tr("Cross-sectional area"));
    Ilabel->setText(QString("I = %1 in<sup>4</sup>").arg(theSxn.I));
    Ilabel->setToolTip(tr("Moment of inertia"));
    Zlabel->setText(QString("Z = %1 in<sup>4</sup>").arg(theSxn.Z));
    Zlabel->setToolTip(tr("Plastic section modulus"));
    Slabel->setText(QString("S = %1 in<sup>4</sup>").arg(theSxn.S));
    Slabel->setToolTip(tr("Elastic section modulus"));
    rlabel->setText(QString("r = %1 in<sup>3</sup>").arg(theSxn.r));
    rlabel->setToolTip(tr("Radius of gyration"));

    // to do: user-defined section

    zeroResponse();
}
// element model type
void MainWindow::inElType_currentIndexChanged(int row)
{
    elType = inElType->itemText(row);
    if (elType == "truss")
    {
        inNe->setEnabled(false);
        inNe->setValue(1);
        inNIP->setEnabled(false);
        inNIP->setValue(2);
    } else {
        inNe->setEnabled(true);
        inNIP->setEnabled(true);
    }

    zeroResponse();
}

// element distribution
void MainWindow::inElDist_currentIndexChanged(int row)
{
    elDist = inElDist->itemText(row);
    buildModel();
    if (elDist == "user-defined") {
        QMessageBox::warning(this, "Warning","User-defined nodes not yet implemented.");
        // to do
    }
}

// integration method
void MainWindow::inIM_currentIndexChanged(int row)
{
    IM = inIM->itemText(row);
    zeroResponse();
}

// integration method
void MainWindow::inShape_currentIndexChanged(int row)
{
    shape = inShape->itemText(row);
    //buildModel();
}

// material model
void MainWindow::inMat_currentIndexChanged(int row)
{
    mat = inMat->itemText(row);

    switch (inMat->currentIndex()) {
    case 0:
        bBox->setVisible(true);
        steel01Box->setVisible(true);
        steel02Box->setVisible(false);
        steel4Frame->setVisible(false);
        break;

    case 1:
        bBox->setVisible(true);
        steel01Box->setVisible(true);
        steel02Box->setVisible(true);
        steel4Frame->setVisible(false);
        break;

    case 2:
        bBox->setVisible(false);
        steel01Box->setVisible(false);
        steel02Box->setVisible(false);
        steel4Frame->setVisible(true);
        break;
    }

    zeroResponse();
}

// connection-1 model
void MainWindow::in_conn1_currentIndexChanged(int row)
{
    type_conn1 = row;

    if (inclConnSymm == true)
        in_conn2->setCurrentIndex(in_conn1->currentIndex());

    zeroResponse();
}

// connection-2 model
void MainWindow::in_conn2_currentIndexChanged(int row)
{
    type_conn2 = row;
    zeroResponse();
}

void MainWindow::inExp_currentIndexChanged(int row) {
  if (row != -1) {
    currentExpInd = row;
    loadExperimentalFile(inExp->itemData(row).toString());
    //loadWallExperimentalFile(inExp->itemData(row).toString()); // adding wall
  }
}

//---------------------------------------------------------------
// spin-box
// number of elements
void MainWindow::inNe_valueChanged(int var)
{
    // define new
    ne = var;
    //buildModel();
}
// number of IPs
void MainWindow::inNIP_valueChanged(int var)
{
    NIP = var;
    //buildModel();
}
// fibers across bf
void MainWindow::inNbf_valueChanged(int var)
{
    if (nbf != var){
        nbf = var;
        //zeroResponse();
    }
}
// fibers across tf
void MainWindow::inNtf_valueChanged(int var)
{
    if (ntf != var){
        ntf = var;
        //zeroResponse();
    }
}
// fibers across d
void MainWindow::inNd_valueChanged(int var)
{
    if (nd != var){
        nd = var;
        //zeroResponse();
    }
}
// fibers across tw
void MainWindow::inNtw_valueChanged(int var)
{
    if (ntw != var){
        ntw = var;
        //zeroResponse();
    }
}
//---------------------------------------------------------------
// double spin box
// wp length
void MainWindow::inLwp_valueChanged(double var)
{
    if (Lwp != var) {
        Lwp = var;

        //buildModel();
    }
}

// brace length
void MainWindow::inL_valueChanged(double var)
{
    if (L != var) {
        L = var;

        //buildModel();
    }
}

// camber
void MainWindow::inDelta_valueChanged(double var)
{
    if (delta != var) {
        delta = var/100.0;
        //deltaL->setText(QString("                                                        = L/%1").arg(1/(delta)));
        //buildModel();
    }
}
// Youngs mod
void MainWindow::inEs_valueChanged(double var)
{
    if (theSteel.Es != var) {
        theSteel.Es = var;
        //zeroResponse();
    }
}
// yield strength
void MainWindow::infy_valueChanged(double var)
{
    if (theSteel.fy != var) {
        theSteel.fy = var;
        //zeroResponse();
    }
}
// strain hardening
void MainWindow::inb_valueChanged(double var)
{/*
    if (theSteel.bk != var) {
        theSteel.bk = var;
        inbk->setValue(var);

        // asymm
        if (inclAsymm == false)
            inbkc->setValue(var);

        zeroResponse();
    }
    */
}
// material props
void MainWindow::ina1_valueChanged(double var)
{
    if (theSteel.a1 != var) {
        theSteel.a1 = var;
        //zeroResponse();
    }
}
void MainWindow::ina2_valueChanged(double var)
{
    if (theSteel.a2 != var) {
        theSteel.a2 = var;
        //zeroResponse();
    }
}
void MainWindow::ina3_valueChanged(double var)
{
    if (theSteel.a3 != var) {
        theSteel.a3 = var;
        //zeroResponse();
    }
}
void MainWindow::ina4_valueChanged(double var)
{
    if (theSteel.a4 != var) {
        theSteel.a4 = var;
        //zeroResponse();
    }
}
//
void MainWindow::inR0_valueChanged(double var)
{
    if (theSteel.R0k != var) {
        theSteel.R0k = var;
        inR0k->setValue(var);

        // asymm
        if (inclAsymm == false)
            inR0kc->setValue(var);

        //zeroResponse();
    }
}
void MainWindow::inR1_valueChanged(double var)
{
    if (theSteel.r1 != var) {
        theSteel.r1 = var;
        inr1->setValue(var);

        // asymm
        if (inclAsymm == false)
            inr1c->setValue(var);

        //zeroResponse();
    }
}
void MainWindow::inR2_valueChanged(double var)
{
    if (theSteel.r2 != var) {
        theSteel.r2 = var;
        inr2->setValue(var);

        // asymm
        if (inclAsymm == false)
            inr2c->setValue(var);

        //zeroResponse();
    }
}
//
void MainWindow::inbk_valueChanged(double var)
{
    if (theSteel.bk != var) {
        theSteel.bk = var;
        inb->setValue(var);

        // asymm
        if (inclAsymm == false)
            inbkc->setValue(var);

        //zeroResponse();
    }
}
void MainWindow::inR0k_valueChanged(double var)
{
    if (theSteel.R0k != var) {
        theSteel.R0k = var;
        inR0->setValue(var);

        // asymm
        if (inclAsymm == false)
            inR0kc->setValue(var);

        //zeroResponse();
    }
}
void MainWindow::inr1_valueChanged(double var)
{
    if (theSteel.r1 != var) {
        theSteel.r1 = var;
        inR1->setValue(var);

        // asymm
        if (inclAsymm == false)
            inr1c->setValue(var);

        //zeroResponse();
    }
}
void MainWindow::inr2_valueChanged(double var)
{
    if (theSteel.r2 != var) {
        theSteel.r2 = var;
        inR2->setValue(var);

        // asymm
        if (inclAsymm == false)
            inr2c->setValue(var);

        //zeroResponse();
    }
}
//
void MainWindow::inbkc_valueChanged(double var)
{
    if (theSteel.bkc != var) {
        theSteel.bkc = var;
        //zeroResponse();
    }
}
void MainWindow::inR0kc_valueChanged(double var)
{
    if (theSteel.R0kc != var) {
        theSteel.R0kc = var;
        //zeroResponse();
    }
}
void MainWindow::inr1c_valueChanged(double var)
{
    if (theSteel.r1c != var) {
        theSteel.r1c = var;
        //zeroResponse();
    }
}
void MainWindow::inr2c_valueChanged(double var)
{
    if (theSteel.r2c != var) {
        theSteel.r2c = var;
        //zeroResponse();
    }
}
//
void MainWindow::inbi_valueChanged(double var)
{
    if (theSteel.bi != var) {
        theSteel.bi = var;

        // asymm
        if (inclAsymm == false)
            inbic->setValue(var);

        //zeroResponse();
    }
}
void MainWindow::inrhoi_valueChanged(double var)
{
    if (theSteel.rhoi != var) {
        theSteel.rhoi = var;

        // asymm
        if (inclAsymm == false)
            inrhoic->setValue(var);

        //zeroResponse();
    }
}
void MainWindow::inbl_valueChanged(double var)
{
    if (theSteel.bl != var) {
        theSteel.bl = var;

        // asymm
        if (inclAsymm == false)
            inblc->setValue(var);

        //zeroResponse();
    }
}
void MainWindow::inRi_valueChanged(double var)
{
    if (theSteel.Ri != var) {
        theSteel.Ri = var;

        // asymm
        if (inclAsymm == false)
            inRic->setValue(var);

        //zeroResponse();
    }
}
void MainWindow::inlyp_valueChanged(double var)
{
    if (theSteel.lyp != var) {
        theSteel.lyp = var;
        //zeroResponse();
    }
}
void MainWindow::inbic_valueChanged(double var)
{
    if (theSteel.bic != var) {
        theSteel.bic = var;
        //zeroResponse();
    }
}
void MainWindow::inrhoic_valueChanged(double var)
{
    if (theSteel.rhoic != var) {
        theSteel.rhoic = var;
        //zeroResponse();
    }
}
void MainWindow::inblc_valueChanged(double var)
{
    if (theSteel.blc != var) {
        theSteel.blc = var;
        //zeroResponse();
    }
}
void MainWindow::inRic_valueChanged(double var)
{
    if (theSteel.Ric != var) {
        theSteel.Ric = var;
        //zeroResponse();
    }
}
//
void MainWindow::inm_valueChanged(double var)
{
    if (theFat.m != var) {
        theFat.m = -var;

        //zeroResponse();
    }
}
void MainWindow::ine0_valueChanged(double var)
{
    if (theFat.e0 != var) {
        theFat.e0 = var;

        //zeroResponse();
    }
}
void MainWindow::inemax_valueChanged(double var)
{
    if (theFat.emax != var) {
        theFat.emax = var;

        //zeroResponse();
    }
}
void MainWindow::inemin_valueChanged(double var)
{
    if (theFat.emin != var) {
        theFat.emin = -var;

        //zeroResponse();
    }
}

// connection-1
void MainWindow::inl_conn1_valueChanged(double var)
{
    if (conn1.L != var) {
        conn1.L = var;
        inL->setValue(Lwp-conn1.L-conn2.L);

        if (inclConnSymm == true)
            inl_conn2->setValue(var);

        //buildModel();
    }
}

void MainWindow::inRigA_conn1_valueChanged(double var)
{
    if (conn1.rigA != var) {
        conn1.rigA = var;

        if (inclConnSymm == true)
            inRigA_conn2->setValue(var);

        //zeroResponse();
    }
}

void MainWindow::inRigI_conn1_valueChanged(double var)
{
    if (conn1.rigI != var) {
        conn1.rigI = var;

        if (inclConnSymm == true)
            inRigI_conn2->setValue(var);

        //zeroResponse();
    }
}

// connection-2
void MainWindow::inl_conn2_valueChanged(double var)
{
    if (conn2.L != var) {
        conn2.L = var;
        inL->setValue(Lwp-conn1.L-conn2.L);

        //buildModel();
    }
}

void MainWindow::inRigA_conn2_valueChanged(double var)
{
    if (conn2.rigA != var) {
        conn2.rigA = var;

        //zeroResponse();
    }
}

void MainWindow::inRigI_conn2_valueChanged(double var)
{
    if (conn2.rigI != var) {
        conn2.rigI = var;

        //zeroResponse();
    }
}

//---------------------------------------------------------------
// check boxes
void MainWindow::matDefault_checked(int state)
{/*
    if (state == Qt::Checked) {
        zeroResponse();
        inclDefault = true;

        // set values
        inb->setValue(0.003);
        inbk->setValue(0.003);
        inbi->setValue(0.0025);
        inbl->setValue(0.004);
        //
        ina1->setValue(0);
        ina2->setValue(1.);
        ina3->setValue(0);
        ina4->setValue(1.);
        //
        inR0->setValue(20.);
        inR1->setValue(0.925);
        inR2->setValue(0.15);
        //
        inR0k->setValue(20.);
        inr1->setValue(0.925);
        inr2->setValue(0.15);
        inrhoi->setValue(1.34);
        inRi->setValue(1.0);
        inlyp->setValue(1.0);

        // compression
        if (inclAsymm == true) {

            inbkc->setValue(0.023);
            inbic->setValue(0.0045);
            inblc->setValue(0.004);
            //
            inR0kc->setValue(25.);
            inr1c->setValue(0.9);
            inr2c->setValue(0.15);
            inrhoic->setValue(0.77);
            inRic->setValue(1.0);
        }

        // set enabled
        ina1->setEnabled(false);
        ina2->setEnabled(false);
        ina3->setEnabled(false);
        ina4->setEnabled(false);
        //
        inR0->setEnabled(false);
        inR1->setEnabled(false);
        inR2->setEnabled(false);
        //
        inR0k->setEnabled(false);
        inr1->setEnabled(false);
        inr2->setEnabled(false);
        inrhoi->setEnabled(false);
        inbl->setEnabled(false);
        inRi->setEnabled(false);
        inlyp->setEnabled(false);
        //
        inR0kc->setEnabled(false);
        inr1c->setEnabled(false);
        inr2c->setEnabled(false);
        inrhoic->setEnabled(false);
        inblc->setEnabled(false);
        inRic->setEnabled(false);

    } else {
        inclDefault = false;
        zeroResponse();
        //
        ina1->setEnabled(true);
        ina2->setEnabled(true);
        ina3->setEnabled(true);
        ina4->setEnabled(true);
        //
        inR0->setEnabled(true);
        inR1->setEnabled(true);
        inR2->setEnabled(true);
        //
        inR0k->setEnabled(true);
        inr1->setEnabled(true);
        inr2->setEnabled(true);
        inrhoi->setEnabled(true);
        inbl->setEnabled(true);
        inRi->setEnabled(true);
        inlyp->setEnabled(true);
        //
        if (inclAsymm == true) {
            inR0kc->setEnabled(true);
            inr1c->setEnabled(true);
            inr2c->setEnabled(true);
            inrhoic->setEnabled(true);
            inblc->setEnabled(true);
            inRic->setEnabled(true);
        }
    }
    */
}
void MainWindow::matFat_checked(int state)
{/*
    if (state == Qt::Checked) {
        inclFat = true;
        fatBox->setVisible(true);

        zeroResponse();

    } else {
        inclFat = false;
        fatBox->setVisible(false);

        zeroResponse();
    }
    */
}
void MainWindow::matAsymm_checked(int state)
{/*
    if (state == Qt::Checked) {
        inclAsymm = true;
        zeroResponse();

        // set values
        // compression - set values
        inbkc->setValue(0.023);
        inbic->setValue(0.0045);
        inblc->setValue(0.004);
        //
        inR0kc->setValue(25.);
        inr1c->setValue(0.9);
        inr2c->setValue(0.15);
        inrhoic->setValue(0.77);
        inRic->setValue(1.0);

        // set enabled
        inbkc->setEnabled(true);
        inbic->setEnabled(true);

        if (inclDefault == false) {
            inR0kc->setEnabled(true);
            inr1c->setEnabled(true);
            inr2c->setEnabled(true);
            inrhoic->setEnabled(true);
            inblc->setEnabled(true);
            inRic->setEnabled(true);
        }

    } else {
        inclAsymm = false;
        zeroResponse();

        // compression - set values
        inbkc->setValue(inbk->value());
        inbic->setValue(inbi->value());
        //
        inR0kc->setValue(inR0k->value());
        inr1c->setValue(inr1->value());
        inr2c->setValue(inr2->value());
        inrhoic->setValue(inrhoi->value());
        inblc->setValue(inbl->value());
        inRic->setValue(inRi->value());

        // set enabled
        inbkc->setEnabled(false);
        inR0kc->setEnabled(false);
        inr1c->setEnabled(false);
        inr2c->setEnabled(false);
        inbic->setEnabled(false);
        inrhoic->setEnabled(false);
        inblc->setEnabled(false);
        inRic->setEnabled(false);
    }
    */
}

void MainWindow::connSymm_checked(int state)
{/*
    if (state == Qt::Checked) {
        inclConnSymm = true;
        inl_conn2->setEnabled(false);
        inRigA_conn2->setEnabled(false);
        inRigI_conn2->setEnabled(false);
        in_conn2->setEnabled(false);

        inl_conn2->setValue(inl_conn1->value());
        inRigA_conn2->setValue(inRigA_conn1->value());
        inRigI_conn2->setValue(inRigI_conn1->value());
        in_conn2->setCurrentIndex(in_conn1->currentIndex());

    } else {
        inclConnSymm = false;

        inl_conn2->setEnabled(true);
        inRigA_conn2->setEnabled(true);
        inRigI_conn2->setEnabled(true);
        in_conn2->setEnabled(true);
    }
    */
}
//---------------------------------------------------------------
// slider
void MainWindow::slider_valueChanged(int value)
{
    //pause = true;
    stepCurr = slider->value();

    double Dcurr = (*expD)[value];
    double tcurr = (*time)[value];
    //tlabel->setText(QString("deformation = %1 in.").arg(Dcurr,0,'f',2));

    // update plots
    tPlot->moveDot(tcurr,Dcurr);

    // update deform
   dPlot->plotResponse(value);
   //mPlot->plotResponse(value);
   //pPlot->plotResponse(value);
   hPlot->plotResponse(value);
}

/*
// press slider
void MainWindow::slider_sliderPressed()
{
    movSlider = true;
}

// released slider
void MainWindow::slider_sliderReleased()
{
    movSlider = false;
}
*/

// stop
/*
void MainWindow::stop_clicked()
{
    stop = true;
}
*/

// play
void MainWindow::play_clicked() {
    if(hasResult)
    {
        if (pause == false) {
            playButton->setText("Play");
            playButton->setToolTip(tr("Play simulation and experimental results"));
            pause = true;
        } else {
            if (playButton->text() == QString("Rewind")) {
                playButton->setText("Play");
                playButton->setToolTip(tr("Play simulation and experimental results"));
                slider->setValue(0);
            } else {
                pause = false;
                playButton->setText("Pause");
                playButton->setToolTip(tr("Pause Results"));
            }
        }
        stepCurr = stepCurr >= numSteps ? 0 : stepCurr;

        // play loop
        while (pause == false) {

            slider->setValue(stepCurr);
            QCoreApplication::processEvents();
            stepCurr++;

            if (stepCurr++ == numSteps) {
                pause = true;
                playButton->setText("Rewind");
            }
        };
    }else{
        QMessageBox::information(this,tr("SWIM Information"), "No results. Click Analyze to run.", tr("I know."));
    }

}

// pause
void MainWindow::pause_clicked()
{
    pause = true;
}

// zero
void MainWindow::restart_clicked()
{
    pause = true;
    playButton->setText("Play");
    playButton->setToolTip(tr("Play simulation and experimental results"));
    slider->setValue(0);
}


void MainWindow::exit_clicked()
{
  QApplication::quit();
}

//---------------------------------------------------------------
void MainWindow::zeroResponse()
{
    // plot original coordinates
    dPlot->setModel(&xc,&yc);
    dPlot->plotModel();
    mPlot->setModel(&xc);
    mPlot->plotModel();
    pPlot->setModel(&xc);
    pPlot->plotModel();

    // re-size response and initialize to zero.
    Ux->reSize(nn,numSteps);
    Uy->reSize(nn,numSteps);

    // force quantities
    q1->reSize(ne,numSteps);
    q2->reSize(ne,numSteps);
    q3->reSize(ne,numSteps);

    //dPlot->setResp(Ux,Uy);
    mPlot->setResp(q2,q3);
    pPlot->setResp(q1,q1);

    hPlot->setResp(&(*Ux->data[nn-1]),&(*q1->data[0]));
    //for (int j=0; j<numSteps; j++)
    //    qDebug() << j<< (*Ux->data[nn-1])[j];

    slider->setValue(0);
}

// x coordinates
QVector<double> xCoord(const double L, const int ne, const QString elDist)
{
    // sub-element length
    double l = L/ne;

    // initialize
    QVector<double> x(ne+1);

    // evenly distribute nodes
    for (int j=1; j<=ne; j++) {
        x[j] = x[j-1] + l;
    }

    // take middle 2-3 elements concentrated around mid-length
    if (elDist == "concentrated" && ne > 5) {
        int a = (ne+1)/2-1-0.5*(ne % 2);
        int b = 3 + (ne % 2);
        x = x.mid(a,b);
        x.prepend(0);
        x.append(L);
    }

    // user-defined
    if (elDist == "user-defined") {

    }

    return x;
}

// x coordinates
QVector<double> yCoord(const QVector<double> x, const double p, const QString shape)
{
    int nn = x.size();
    double l = x[nn-1] - x[0];
    QVector<double> y(nn);

    // node closest to midpoint (in case ne = odd)
    double xo = x[nn/2-(nn-1)%2];

    // perturbation shape
    // perturb mid-node
    if (shape == "midpoint perturbation") {
        if (nn != 2) {
            if (nn % 2 == 1) {
                y[nn/2] = p;
            } else {
                y[nn/2-0.5] = p;
                y[nn/2+0.5] = p;
            }
        }

    // arrange in linear shape
    } else if (shape == "linear") {
        // y = p/(L/2)*x if x<L/2
        for (int j=1; j<=nn/2; j++) {
            y[j] = p/xo*x[j];
        }

        // y = 2*p - p/(L/2)*x if x>L/2
        for (int j=ceil(nn/2); j<nn-1; j++) {
            y[j] = l*p/xo-p/xo*x[j];
        }

    // arange nodes in parabolic shape
    } else if (shape == "parabolic") {
        // adjust perturbation to account for ne = odd
        double po = p*l/(4*xo*(1-xo/l));

        // y = 4*p/L*x*(1-x/L)
        for (int j=1; j<nn-1; j++) {
            y[j] = 4*po/l*x[j]*(1-x[j]/l);
        }

    // arrange nodes in sinusoidal shape
    } else if (shape == "sinusoidal") {
        // adjust perturbation to account for ne = odd
        double po = p/(sin(4*atan(1)/l*xo));

        // y = p*sin(pi/L*x);
        for (int j=1; j<nn-1; j++) {
            y[j] = po*sin(4*atan(1)/l*x[j]);
        }
    }

    return y;
}


double interpolate(QVector<double> &xData, QVector<double> &yData, double x, bool extrapolate)
{
    int size = xData.size();

    // left end of interval
    int j=0;
    if (x >= xData[size-2]) // special case - beyond right end
        j = size-2;
    else
        while (x>xData[j+1])
            j++;

    // points on either side of x
    double xL = xData[j], yL = yData[j], xR = xData[j+1], yR = yData[j+1];

    // if not extrapolating
    if (!extrapolate)
    {
        if (x<xL)
            yR = yL;
        if (x>xR)
            yL = yR;
    }

    // gradient
    double dydx;
    if (xR==xL)
        dydx = 0;
    else
        dydx = (yR-yL)/(xR-xL);

    // linear interpolation
    return yL + dydx*(x - xL);
}

// build model
void MainWindow::buildModel()
{
    pause = true;
    playButton->setText("Play");
    playButton->setToolTip(tr("Play simulation and experimental results"));
    QCoreApplication::processEvents();

    // element lengths
    if (conn1.L < 0.01*Lwp) {
        inl_conn1->setValue(0.01*Lwp);
    }
    if (conn2.L < 0.01*Lwp) {
        inl_conn2->setValue(0.01*Lwp);
    }

    //L = 0.9*Lwp;
    //double Lc1 = 0.05*Lwp;
    //double Lc2 = 0.05*Lwp;

    // camber
    double p = delta*L;


    // node coordinates - should these be pointers?
    // x-coordinates
    xc = xCoord(L, ne, elDist);

    // y-coordinates
    yc = yCoord(xc, p, shape);

    // add nodes for spring elements
    xc.prepend(0); xc.append(L);
    yc.prepend(0); yc.append(0);

    // add nodes for boundary condition elements
    xc.prepend(-conn1.L); xc.append(L+conn2.L);
    yc.prepend(0); yc.append(0);


    // nn
    nn = xc.size();

    // initialize response
    zeroResponse();
}

// Open file
void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
      reset();
      loadFile(fileName);      
    }
    currentFile = fileName;    
}

bool MainWindow::save()
{
    if (currentFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(currentFile);
    }
}

bool MainWindow::saveAs()
{
    //
    // get filename
    //

    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;

    // and save the file
    return saveFile(dialog.selectedFiles().first());
}

// Description of this software package that shows in Help menu
void MainWindow::about()
{
    QString textAbout = "\
            <p> This NHERI SimCenter educational application will allow the user to explore how modeling assumptions effect the response of a concrete shear wall element. \
             The application will allow the user to explore effects such as: <ul>\
            <li>number of elements, </li> \
            <li> material type and properties,</li> \
            on the response.\
            <p>\
            To allow the user to test validity of their modelling assumptions, the results are compared to data obtained from a number of experimental tests.\
            <p>\
            Developers <ul><li> Main Developer: Charles Wang, c_w@berkeley.edu.</li>\
            <li> Others who have contributed to Coding, Debugging, Testing and Documentation: Frank McKenna.</li>\
            </ul><p>\
           \
            \
            ";

    QMessageBox msgBox;
    QSpacerItem *theSpacer = new QSpacerItem(500, 0, QSizePolicy::Maximum, QSizePolicy::Expanding);
    msgBox.setText(textAbout);
    QGridLayout *layout = (QGridLayout*)msgBox.layout();
    layout->addItem(theSpacer, layout->rowCount(),0,1,layout->columnCount());
    msgBox.exec();
}

// Link to submit feedback through issue on GitHub
void MainWindow::submitFeedback()
{
  QDesktopServices::openUrl(QUrl("https://github.com/NHERI-SimCenter/SWIM/issues", QUrl::TolerantMode));
}

// Version this release
void MainWindow::version()
{
    QMessageBox::about(this, tr("Version"),
                       tr("Version 1.0.1"));
}

// Copyright specification to include in Help menu
void MainWindow::cite()
{
    QString textCite = "\
        <p>\
Charles Wang. (2019, October 1). \
SimCenter/SWIM: v1.0.0 (Version v1.0.0). Zenodo. http://doi.org/10.5281/zenodo.3466376 \
      <p>\
      ";


    QMessageBox msgBox;
    QSpacerItem *theSpacer = new QSpacerItem(700, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    msgBox.setText(textCite);
    QGridLayout *layout = (QGridLayout*)msgBox.layout();
    layout->addItem(theSpacer, layout->rowCount(),0,1,layout->columnCount());
    msgBox.exec();
}



// Copyright specification to include in Help menu
void MainWindow::copyright()
{
    QString textCopyright = "\
        <p>\
        The source code is licensed under a BSD 2-Clause License:<p>\
        \"Copyright (c) 2017-2019, The Regents of the University of California (Regents).\"\
        All rights reserved.<p>\
        <p>\
        Redistribution and use in source and binary forms, with or without \
        modification, are permitted provided that the following conditions are met:\
        <p>\
         1. Redistributions of source code must retain the above copyright notice, this\
         list of conditions and the following disclaimer.\
         \
         \
         2. Redistributions in binary form must reproduce the above copyright notice,\
         this list of conditions and the following disclaimer in the documentation\
         and/or other materials provided with the distribution.\
         <p>\
         THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND\
         ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\
         WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\
         DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR\
         ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\
         (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\
         LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND\
            ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\
            (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\
            SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\
            <p>\
            The views and conclusions contained in the software and documentation are those\
            of the authors and should not be interpreted as representing official policies,\
            either expressed or implied, of the FreeBSD Project.\
            <p>\
            REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, \
            THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.\
            THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS \
            PROVIDED \"AS IS\". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,\
            UPDATES, ENHANCEMENTS, OR MODIFICATIONS.\
            <p>\
            ------------------------------------------------------------------------------------\
            <p>\
            The compiled binary form of this application is licensed under a GPL Version 3 license.\
            The licenses are as published by the Free Software Foundation and appearing in the LICENSE file\
            included in the packaging of this application. \
            <p>\
            ------------------------------------------------------------------------------------\
            <p>\
            This software makes use of the QT packages (unmodified): core, gui, widgets and network\
                                                                     <p>\
                                                                     QT is copyright \"The Qt Company Ltd&quot; and licensed under the GNU Lesser General \
                                                                     Public License (version 3) which references the GNU General Public License (version 3)\
      <p>\
      The licenses are as published by the Free Software Foundation and appearing in the LICENSE file\
      included in the packaging of this application. \
      <p>\
      ------------------------------------------------------------------------------------\
      <p>\
      This software makes use of the OpenSees Software Framework. OpenSees is copyright \"The Regents of the University of \
      California\". OpenSees is open-source software whose license can be\
      found at http://opensees.berkeley.edu.\
      <p>\
      ";


    QMessageBox msgBox;
    QSpacerItem *theSpacer = new QSpacerItem(700, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    msgBox.setText(textCopyright);
    QGridLayout *layout = (QGridLayout*)msgBox.layout();
    layout->addItem(theSpacer, layout->rowCount(),0,1,layout->columnCount());
    msgBox.exec();
}

// build model
void MainWindow::doAnalysis()
{
}

// build model
void MainWindow::doWallAnalysis()
{
    // running dialog
/*
    QProgressDialog progressDialog("Running Analysis...", "Cancel", 0, INT_MAX, this);
    QProgressBar* bar = new QProgressBar(&progressDialog);
    bar->setRange(0,numSteps);
    bar->setValue(0);
    progressDialog.setBar(bar);
    progressDialog.setMinimumWidth(500);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setValue(0);

    stop = false;

    // clear existing model
    theDomain.clearAll();
    OPS_clearAllUniaxialMaterial();
    ops_Dt = 0.0;

    // orientations
    //Vector x(3); x(0) = 1.0; x(1) = 0.0; x(2) = 0.0;
    //Vector y(3); y(0) = 0.0; y(1) = 1.0; y(2) = 0.0;

    // direction for spring elements
    //ID dir(2); dir[0] = 0; dir[1] = 2;

    // number of nodes
    //int nn = xc.size(); // + 2 for connections

    // pinned constraints
    static Matrix eqPIN(2,2);
    eqPIN.Zero(); eqPIN(0,0)=1.0; eqPIN(1,1)=1.0;
    static ID dofPIN(2);
    dofPIN(0) = 0; dofPIN(1) = 1;

    // fixed constraints
    static Matrix eqFIX(3,3);
    eqFIX.Zero(); eqFIX(0,0)=1.0; eqFIX(1,1)=1.0; eqFIX(2,2)=1.0;
    static ID dofFIX(3);
    dofFIX(0) = 0; dofFIX(1) = 1; dofFIX(2) = 2;

    // nodes
    Node **theNodes = new Node *[nn];


    Node *theNode = new Node(1, 2, 0.,0.);
    theNodes[1] = theNode;
    theDomain.addNode(theNode);
    theNode = new Node(2, 2, 7.5785,0.);
    theNodes[2] = theNode;
    theDomain.addNode(theNode);
    theNode = new Node(3, 2, 7.5785,11.3329);
    theNodes[3] = theNode;
    theDomain.addNode(theNode);
    theNode = new Node(4, 2, 0.,11.3329);
    theNodes[4] = theNode;
    theDomain.addNode(theNode);
    theNode = new Node(5, 2, 7.5785,22.6657);
    theNodes[5] = theNode;
    theDomain.addNode(theNode);
    theNode = new Node(6, 2, 0.,22.6657);
    theNodes[6] = theNode;
    theDomain.addNode(theNode);
    theNode = new Node(7, 2, 7.5785,33.9986 );
    theNodes[7] = theNode;
    theDomain.addNode(theNode);
    theNode = new Node(8, 2, 0.,33.9986 );
    theNodes[8] = theNode;
    theDomain.addNode(theNode);


    // uniaxial materials
    UniaxialMaterial *theMat1 = 0;
    theMat1 = new Steel01(1, 84.6555, 29000, 0.01 );
    UniaxialMaterial *theMat2 = 0;
    theMat2 = new Steel01(2, 83.5388, 29000, 0.01 );
    UniaxialMaterial *theMat3 = 0;
    theMat3 = new Steel01(3, 84.6555, 29000, 0.01 );
*/
    // nDmats
    int matTag = 0;

    /*
        # nDMaterial PlaneStressRebarMaterial matTag matTag_for_uniaxialMaterial angle_of_rebar
        nDMaterial PlaneStressRebarMaterial 1 1 90
        nDMaterial PlaneStressRebarMaterial 2 2 90
        nDMaterial PlaneStressRebarMaterial 3 3 0
    */
/*
    NDMaterial *theMat;
    matTag += 1;
    theMat = new PlaneStressRebarMaterial(matTag, *theMat1, 90.);
    OPS_addNDMaterial(theMat);
    matTag += 1;
    theMat = new PlaneStressRebarMaterial(matTag, *theMat2, 90.);
    OPS_addNDMaterial(theMat);
    matTag += 1;
    theMat = new PlaneStressRebarMaterial(matTag, *theMat3, 0.);
    OPS_addNDMaterial(theMat);

    //define pars for PlasticDamageConcretePlaneStress
    double E = 4635.43;
    double nu = 0.2;
    double fc = 6.61349*0.7;
    double ft = fc*0.25;
    double beta = 5.462261077231849e-01 ;
    double Ap = 9.665784174420770e-02 ;
    double An = 3.528346194898871e+00 ;
    double Bn = 4.914497665322617e-01;
    //nDMaterial PlasticDamageConcretePlaneStress 4 $E $nu $ft $fc $beta $Ap $An $Bn ;# concrete
    matTag += 1;
    theMat = new PlasticDamageConcretePlaneStress(matTag, E, nu, ft, fc, beta, Ap, An, Bn);
    OPS_addNDMaterial(theMat);
*/

    /*
    matTag += 1;
    theMat = new PlaneStressLayeredMaterial(matTag, 3,theMat, 5.85852,theMat, 0.0316793,theMat,  0.0148032);
    OPS_addNDMaterial(theMat);

    nDMaterial PlaneStressLayeredMaterial 5 3 4 5.85852 1 0.0316793 3 0.0148032
    nDMaterial PlaneStressLayeredMaterial 6 3 4 5.81253 2 0.0693821 3 0.0230882
    nDMaterial PlaneStressLayeredMaterial 7 3 4 5.85852 1 0.0316793 3 0.0148032
    nDMaterial PlaneStressLayeredMaterial 8 3 4 5.81253 2 0.0693821 3 0.0230882
    */

    /*
    FourNodeQuad *theEle;
    theEle = new FourNodeQuad(1, 1, 2,3,4,
                           *theMat, 1.0, uBulk, 1.0, 1.0, 1.0, evoid, alpha, 0.0, g * 1.0); // -9.81 * theMat->getRho() TODO: theMat->getRho()

    theDomain->addElement(theEle);
*/



}

void MainWindow::doWallAnalysisOpenSees()
{
    QString tclFileName = expDirName + "/wall.tcl";
    stepOpenSees = 0;



    QFile openseesExefile(openseespath);

    if(openseesExefile.exists())
    {
        if(!QDir(openseespath).exists()){
            // do FEA in opensees
            progressbar->show();
            updateSAMFile();// update sam file
            preprocess();//create tcl

            // init the opensess process
            //delete openseesProcess;
            openseesProcess = new QProcess(this);
            openseesProcess->setWorkingDirectory(expDirName);
            //connect(openseesProcess, SIGNAL(readyReadStandardOutput()),this,SLOT(onOpenSeesFinished()));
            connect(openseesProcess, SIGNAL(readyReadStandardError()),this,SLOT(onOpenSeesFinished()));

            openseesProcess->start(openseespath,QStringList()<<tclFileName);
            openseesErrCount = 1;
        }else{
             QMessageBox::information(this,tr("SWIM Information"), "The path you gave is not ane executable", tr("I know."));
        }

    } else {
        QMessageBox::information(this,tr("SWIM Information"), "Please specify FEM engine (in SAM tab) with OpenSees path.", tr("I know."));
    }

}

int MainWindow::getstepOpenSees()
{
    return stepOpenSees;
}
void MainWindow::onOpenSeesFinished()
{

    //writeSurfaceMotion();
    QString str_err = openseesProcess->readAllStandardError();

    int stepOpenSeestmp = 0;

    if(openseesErrCount==1)
    {
        if(str_err.contains("cyclic is done."))
        {
            expWall->setD(&thePreprocessor->dispVec);
            expWall->setP(&thePreprocessor->forceVec);
            expWall->setTime();
            setExp(expWall);

            stepOpenSeestmp = getstepOpenSees();


            PostProcessor *postprocessor = new PostProcessor(expDirName,numSteps);
            if(postprocessor->getLineCount()!=numSteps)
            {
            //if(stepOpenSeestmp<int(ceil((numSteps-1.0)/numSteps*100.0))){//
            //if(false){
                openseesErrCount = 2;
                QMessageBox::information(this,tr("OpenSees Information"), "OpenSees failed to complete. Try using lesser elements. ", tr("I know."));
                hasResult = false;
                stepOpenSees = 0;
                emit signalProgress(1);
                progressbar->hide();
            }
            else
            {
                //qDebug() << "opensees says:" << str_err;
                openseesErrCount = 2;

                // do some postprocessing here

                QMessageBox::information(this,tr("OpenSees Information"), "Analysis is done.", tr("I know."));


                emit signalProgress(100);
                progressbar->hide();

                //std::vector<double> *a = &thePreprocessor->dispVec;
                //std::vector<double> b = postprocessor->force[0];


                hPlot->setModel(expD,expP);
                hPlot->plotModel();


                //hPlot->setResp(&(*Ux->data[nn-1]),&(*q1->data[0]));
                QVector<double> responseForceWall;
                std::vector<double> responseForceWall_std = postprocessor->getForce();
                dispx = postprocessor->getDispx();
                dispy = postprocessor->getDispy();

                for(auto f : responseForceWall_std)
                {
                    responseForceWall.append(f);
                }

                hPlot->setResp(expD,&responseForceWall);
                hPlot->plotResponse(0);

                dPlot->setResp(&dispx, &dispy);
                dPlot->plotResponse(0);

                hasResult = true;
            }



        }else{
            //qDebug() << str_err;
            QRegExp rxlen("(.+)(%)");
            int pos = rxlen.indexIn(str_err);
            if (pos > -1) {
                QString value = rxlen.cap(1);
                stepOpenSees = int(ceil(value.toDouble()));
                if(stepOpenSees>0)
                    emit signalProgress(stepOpenSees);
            }


        }
    }

}


void MainWindow::setExp(ExperimentWall *exp)
{
    numSteps = exp->getNumSteps();

    /*
    // experimental data
    expD = new QVector<double>(numSteps,0.);
    expP = new QVector<double>(numSteps,0.);
    time = new QVector<double>(numSteps,0.);
    */

    // re-size
    time->resize(numSteps);
    expP->resize(numSteps);
    expD->resize(numSteps);

    // extract from experiment
    time = exp->getTime();
    dt = exp->getdt();
    expP = exp->getDataP();
    expD = exp->getDataD();

    // update experiment plot
    tPlot->setData(expD,time);
    hPlot->setModel(expD,expP);
    hPlot->plotModel();


    // zero response
    //zeroResponse();

    // set slider
    slider->setRange(0,numSteps-1);
    //slider->setValue(0);

}

// layout functions
// header

void MainWindow::createHeaderBox()
{
    HeaderWidget *header = new HeaderWidget();
    header->setHeadingText(tr("Shear Wall Intelligent Modeling"));
    largeLayout->addWidget(header);
}

// footer
void MainWindow::createFooterBox()
{
    FooterWidget *footer = new FooterWidget();
    largeLayout->addWidget(footer);
}

void setLimits(QDoubleSpinBox *widget, int min, int max, int decimal = 0, double step = 1)
{
    widget->setMinimum(min);
    widget->setMaximum(max);
    widget->setDecimals(decimal);
    widget->setSingleStep(step);
}

void setLimits(QSpinBox *widget, int min, int max, double step = 1)
{
    widget->setMinimum(min);
    widget->setMaximum(max);
    widget->setSingleStep(step);
}

// input panel
void MainWindow::createInputPanel()
{
    // units
    QString blank(tr(" "));
    QString kips(tr("k"  ));
    QString g(tr("g"  ));
    QString kipsInch(tr("k/in "));
    QString inch(tr("in. "));
    QString sec(tr("sec "));
    QString percent(tr("\% "));
    QString ksi(tr("ksi "));

    // Lists
    QStringList expList = { "" };
    QStringList elTypeList = { "force-based fiber", "displacement-based fiber", "truss" };
    QStringList distList = { "distributed", "concentrated", "user-defined" };
    QStringList IMList = { "Gauss-Lobatto" };
    QStringList shapeList = { "midpoint perturbation", "linear", "parabolic", "sinusoidal" };
    QStringList orientList = { "y-y", "x-x" };
    QStringList matList = { "uniaxial bilinear (steel01)",
                            "uniaxial Giuffre-Menegotto-Pinto (steel02)",
                            "uniaxial asymmetric Giuffre-Menegotto-Pinto (steel4)" };
    QStringList connList = { "pinned",
                             "fixed" };
                           //"spring with bilinear material" };

    // boxes
    inBox = new QGroupBox("Input");

    // tabs
    QTabWidget *tabWidget = new QTabWidget(this);



    // layouts
    inLay = new QVBoxLayout;

    QGridLayout *expLay = new QGridLayout();

    /*
    QPushButton *loadNewBtn = new QPushButton("Load new");
    expLay->addWidget(loadNewBtn,0,2);
    connect(loadNewBtn,SIGNAL(clicked()), this, SLOT(loadNewBtn_clicked()));
    */

    QPushButton *addExp = new QPushButton("Add Experiment");
    addExp->setToolTip(tr("Load different experiment"));

    inExp = addCombo(tr("Experiment: "),expList,&blank,expLay,0,0);
    inExp->clear();
    inExp->setToolTip(tr("Experiment name"));
    expLay->addWidget(addExp,0,1);
    QRect rec = QApplication::desktop()->screenGeometry();
    int height = 0.7*rec.height();
    //FMK    int width = 0.7*rec.width();
    //FMK inExp->setMinimumWidth(0.6*width/2);
    expLay->setColumnStretch(2,1);














    // buttons
    // buttons
    QHBoxLayout *buttonLay = new QHBoxLayout();

    QPushButton *run = new QPushButton("Analyze");
    run->setToolTip(tr("Run simulation with current properities"));
    //QPushButton *stop = new QPushButton("stop");
    QPushButton *reset = new QPushButton("Reset");
    reset->setToolTip(tr("Clear simulation results and reload default experiment"));
    AIbtn = new QPushButton("AI");
    AIbtn->setToolTip(tr("Set parameters predicted by AI"));
    connect(AIbtn,SIGNAL(clicked()), this, SLOT(AIbtn_clicked()));
    playButton = new QPushButton("Play");
    playButton->setToolTip(tr("Play simulation and experimental results"));
   // QPushButton *pause = new QPushButton("Pause");
   // pause->setToolTip(tr("Pause current results playback"));
   //QPushButton *restart = new QPushButton("Restart");
   // restart->setToolTip(tr("Restart results playback"));
    QPushButton *exitApp = new QPushButton("Exit");
    exitApp->setToolTip(tr("Exit Application"));


    buttonLay->addWidget(AIbtn);
    buttonLay->addWidget(run);
    buttonLay->addWidget(playButton);
    //buttonLay->addWidget(reset);

   // buttonLay->addWidget(pause);
   // buttonLay->addWidget(restart);
    buttonLay->addWidget(exitApp);
   // buttonLay->setColumnStretch(3,1);

    // set tab layouts
    inLay->addLayout(expLay);

    /*
     * ---------------------------------------------------
     *adding wall - begin
     * ---------------------------------------------------
     */
    createSAM();
    numFloors = theWall->numFloors;

    // BIM tab
    QWidget *wallConfigTab = new QWidget;
    wallBIMLay = new QGridLayout();


    theExpNameEdt = addLineEdit(tr("Experiment name: "), thisexpname,wallBIMLay,0,0);
    theExpNameEdt->setDisabled(true);



    QSpinBox *nofEdt = addSpin(tr("Number of floors: "),&blank,wallBIMLay,1,0);
    nofEdt->setValue(theWall->numFloors);
    nofEdt->setDisabled(true);

    idFloorEdt_BIM = addSpin(tr("Select floor: "),&blank,wallBIMLay,2,0);
    idFloorEdt_BIM->setMinimum(1);
    idFloorEdt_BIM->setMaximum(theWall->numFloors);
    idFloorEdt_BIM->setValue(1);

    createBIMui();

    wallBIMLay->setRowStretch(wallBIMLay->rowCount(),1);

    connect(nofEdt,SIGNAL(valueChanged(int)), this, SLOT(nofEdt_valueChanged(int)));
    connect(idFloorEdt_BIM, SIGNAL(valueChanged(int)), this, SLOT(idFloorEdt_valueChanged_BIM(int)));

    wallConfigTab->setLayout(wallBIMLay);
    tabWidget->addTab(wallConfigTab, "Wall info");


    // SAM tab
    QWidget *wallSAMTab = new QWidget;
    wallSAMLay = new QGridLayout();

    QGroupBox *meshBox = new QGroupBox("Global Mesh Control");
    QGridLayout *meshBoxLay = new QGridLayout();
    eleSizeWebEdt = addDoubleSpin(tr("Web Element Size"),&inch,meshBoxLay,0,0);
    eleSizeWebEdt->setToolTip(tr(""));
    eleSizeWebEdt->setRange(2.0, webLength);
    eleSizeBEEdt = addDoubleSpin(tr("Boundary Element Size"),&inch,meshBoxLay,1,0);
    eleSizeBEEdt->setToolTip(tr(""));
    eleSizeBEEdt->setRange(2.0, beLength);

    connect(eleSizeWebEdt, SIGNAL(valueChanged(double)), this, SLOT(ESize_valueChanged_SAM(double)));
    connect(eleSizeBEEdt, SIGNAL(valueChanged(double)), this, SLOT(ESize_valueChanged_SAM(double)));
    connect(eleSizeWebEdt, SIGNAL(valueChanged(double)), this, SLOT(webESize_valueChanged_SAM(double)));
    connect(eleSizeBEEdt, SIGNAL(valueChanged(double)), this, SLOT(beESize_valueChanged_SAM(double)));

    meshBoxLay->setColumnStretch(1,1);
    meshBoxLay->setRowStretch(1,1);
    meshBox->setLayout(meshBoxLay);

    wallSAMLay->addWidget(meshBox,0,0);


    idFloorEdt_SAM = addSpin(tr("Select floor: "),&blank,wallSAMLay,2,0);
    idFloorEdt_SAM->setMaximum(theWall->numFloors);
    idFloorEdt_SAM->setMinimum(1);
    idFloorEdt_SAM->setValue(1); // default


    connect(idFloorEdt_SAM, SIGNAL(valueChanged(int)), this, SLOT(idFloorEdt_valueChanged_SAM(int)));

    wallSAMTab->setLayout(wallSAMLay);
    tabWidget->addTab(wallSAMTab, "SAM");

    getSAM();

    createSAMui();
    wallSAMLay->setRowStretch(wallSAMLay->rowCount(),1);


    preprocess(); // SAM->tcl

/*
    // AI predicts parameters and update the UI
    std::vector<float> AIinputsVec = getAIinputs();
    std::vector<float> predValues = ai.predict(AIinputsVec);// ['Ap','An', 'Bn', 'beta','N']
    for(auto concreteIDtmp : matIDList_concrete)
    {
        matIDselector_concrete->setCurrentText(concreteIDtmp);
        int cIDtmp = concreteIDtmp.toInt();
        Ap = double(std::max(predValues[0],float(0.)));
        An = double(std::max(predValues[1],float(0.)));
        Bn = double(std::max(predValues[2],float(0.)));
        beta = double(std::max(predValues[3],float(0.)));
        concreteApEdt[cIDtmp]->setValue(Ap);
        concreteAnEdt[cIDtmp]->setValue(An);
        concreteBnEdt[cIDtmp]->setValue(Bn);
        concretebetaEdt[cIDtmp]->setValue(beta);
    }
    int nL = std::max(int(predValues[4]),1);// number of elements along web length
    eleSizeWebEdt->setValue(webLength/double(nL));
    eleSizeBEEdt->setValue(beLength/2.0);// default mesh of boundary: 2 elements
    */
    updateSAMFile();






    /*
     * ---------------------------------------------------
     *adding wall - end
     * ---------------------------------------------------
     */


/*
    // add layout to tab
    tabWidget->addTab(elTab, "Element");
    tabWidget->addTab(sxnTab, "Section");
    tabWidget->addTab(matTab, "Material");
    tabWidget->addTab(connTab, "Connection");
    //tabWidget->addTab(analyTab, "Analysis");
    //FMK    tabWidget->setMinimumWidth(0.5*width);
*/
    // add tab
    inLay->addWidget(tabWidget);
    //inLay->addStretch();

    // add buttons
    inLay->addLayout(buttonLay);



    // add to main layout
    inBox->setLayout(inLay);


    QScrollArea *inscrollArea = new QScrollArea;
    //inscrollArea->setBackgroundRole(QPalette::Dark);
    inscrollArea->setWidget(inBox);
    rec = QGuiApplication::primaryScreen()->geometry();
    if(wheight<0.1)
    {
        wheight = this->height()<int(0.55*rec.height())?int(0.55*rec.height()):this->height();
        wwidth  = this->width()<int(0.55*rec.width())?int(0.55*rec.width()):this->width();
        //wwidth = wwidth;
    }
    //inscrollArea->setMinimumSize(wwidth*0.5, wheight);
    //inscrollArea->setMaximumSize(wwidth*2, wheight);
    //inscrollArea->resize(wwidth, wheight);
    inscrollArea->setMinimumWidth(tabWidget->width()+50);
    //inscrollArea->setMaximumWidth(wwidth);
    mainLayout->addWidget(inscrollArea, 0);

    this->setMaximumWidth(tabWidget->width()*1.7);
    this->setMaximumHeight(tabWidget->height()*2);



    //mainLayout->addWidget(inBox, 0);



    //largeLayout->addLayout(mainLayout);

    // connect signals / slots
    // buttons
    connect(addExp,SIGNAL(clicked()), this, SLOT(addExp_clicked()));

    connect(reset,SIGNAL(clicked()), this, SLOT(reset()));
    //connect(run,SIGNAL(clicked()), this, SLOT(doAnalysis()));
    //connect(run,SIGNAL(clicked()), this, SLOT(doWallAnalysis()));
    connect(run,SIGNAL(clicked()), this, SLOT(doWallAnalysisOpenSees()));
    //connect(stop,SIGNAL(clicked()), this, SLOT(stop_clicked()));
    connect(playButton,SIGNAL(clicked()), this, SLOT(play_clicked()));
    //connect(pause,SIGNAL(clicked()), this, SLOT(pause_clicked()));
    //connect(restart,SIGNAL(clicked()), this, SLOT(restart_clicked()));
    connect(exitApp,SIGNAL(clicked()), this, SLOT(exit_clicked()));


}

void MainWindow::AIbtn_clicked()
{
    // AI predicts parameters and update the UI
    std::vector<float> AIinputsVec = getAIinputs();
    std::vector<float> predValues = ai.predict(AIinputsVec);// ['Ap','An', 'Bn', 'beta','N']
    for(auto concreteIDtmp : matIDList_concrete)
    {
        matIDselector_concrete->setCurrentText(concreteIDtmp);
        int cIDtmp = concreteIDtmp.toInt();
        Ap = double(std::max(predValues[0],float(0.01)));
        An = double(std::max(predValues[1],float(0.01)));
        Bn = double(std::max(predValues[2],float(0.01)));
        beta = double(std::max(predValues[3],float(0.01)));

        ApAI = Ap;
        AnAI = An;
        BnAI = Bn;
        betaAI = beta;

        concreteApEdt[cIDtmp]->setValue(Ap);
        concreteAnEdt[cIDtmp]->setValue(An);
        concreteBnEdt[cIDtmp]->setValue(Bn);
        concretebetaEdt[cIDtmp]->setValue(beta);

        concreteApEdt[cIDtmp]->setStyleSheet("border-color: green;");
        concreteAnEdt[cIDtmp]->setStyleSheet("background-color: yellow");
        concreteBnEdt[cIDtmp]->setStyleSheet("background-color: yellow");
        concretebetaEdt[cIDtmp]->setStyleSheet("background-color: yellow");

    }
    nLAI = std::max(int(predValues[4]),1);// number of elements along web length
    eleSizeWebEdt->setValue(webLength/double(nLAI));
    eleSizeBEEdt->setValue(beLength/2.0);// default mesh of boundary: 2 elements
    eleSizeWebEdt->setStyleSheet("background-color: yellow");
    eleSizeBEEdt->setStyleSheet("background-color: yellow");
    updateSAMFile();

}

void MainWindow::ESize_valueChanged_SAM(double esize)
{
    hasResult = false;
     if (dPlot != nullptr)
     {
         updateSAMFile();
         dPlot->plotModel();
     }
}
void MainWindow::webESize_valueChanged_SAM(double esize)
{
    eleSizeWebEdt->setStyleSheet("");
}
void MainWindow::beESize_valueChanged_SAM(double esize)
{
    eleSizeBEEdt->setStyleSheet("");
}

// handle when user change the number of floors // adding wall
void MainWindow::nofEdt_valueChanged(int nof)
{
    QString blank(tr(" "));
    QString inch(tr("inch "));


    //wallConfigLay->setRowStretch(wallConfigLay->rowCount(),2);

}

// handle when user change the current id of floor in BIM tab// adding wall
void MainWindow::idFloorEdt_valueChanged_BIM(int inFloor)
{
    QString blank(tr(" "));
    QString inch(tr("inch "));

    //wallSAMLay->removeWidget(floorSAMs[inFloor]);
    //delete floorSAMs[inFloor];
    if (inFloor>0)
    {
        for (int i=0; i<numFloors; i++){
            floorBIMs[i]->hide();
        }
        floorBIMs[inFloor-1]->show();

        currentFloorIDbim = inFloor-1;
    }
}

// handle when user change the current id of floor in BIM tab// adding wall
void MainWindow::matSelectorBIM_valueChanged_BIM(int matID)
{

        for (int i=0; i<numMatsBIM; i++){
            matBIMs[currentFloorIDbim][i]->hide();
        }
        matBIMs[currentFloorIDbim][matID]->show();
}

// handle when user change the current id of floor in SAM tab// adding wall
void MainWindow::idFloorEdt_valueChanged_SAM(int inFloor)
{

    /*
    if (inFloor>0)
    {
        for (int i=0; i<numFloors; i++){
            floorSAMs[i]->hide();
        }
        floorSAMs[inFloor-1]->show();
    }
    */
    for(int i=0; i<numFloors; i++)
    {
        webRCselector[i]->hide();
        beRCselector[i]->hide();
    }
    webRCselector[inFloor-1]->show();
    beRCselector[inFloor-1]->show();

}

// create SAM   // adding wall
void MainWindow::createSAM()
{

    QString bimFileName = expDirName + "/BIM.json";
    QString samFileName = expDirName + "/SAM.json";

    int nL = 5;
    int nH = nL;
    int nW = 5;


    char *filenameEVENT = 0;

    theWall = new ConcreteShearWall();
    theWall->initConcrete(beta, An, Ap, Bn);
    theWall->readBIM(filenameEVENT, bimFileName.toStdString().c_str());
    theWall->writeSAM(samFileName.toStdString().c_str(), nL, nH, nW);
    thisexpname = QString::fromStdString(theWall->expName);

    printf("SAM file created successfully. \n");

}

void MainWindow::updateSAMFile()
{

    QString bimFileName = expDirName + "/BIM.json";
    QString samFileName = expDirName + "/SAM.json";

    double esizeWeb = eleSizeWebEdt->value();
    double esizeBE = eleSizeBEEdt->value();

    int nL = int(round(webLength/esizeWeb));
    int nH = nL; // this number doesn't matter
    int nW = int(round(beLength/esizeBE));

    // update parameters
    //QMap<int, QDoubleSpinBox*> concreteApEdt;
    /*
    QMapIterator<int, QDoubleSpinBox*> i(concreteApEdt);
    while (i.hasNext()) {
        i.next();
        QDoubleSpinBox *spinObj = i.value();
        spinObj->value()
        rcMatStrList.append(QString::number(rebarObj["name"].toInt()));
    }
    */
    /*
    Ap = concreteApEdt.first()->value();
    An = concreteAnEdt.first()->value();
    Bn = concreteBnEdt.first()->value();
    beta = concretebetaEdt.first()->value();
    */

    /*
    double beta = 0.5;
    double An = 0.5;
    double Ap = 0.5;
    double Bn = 0.5;
    */

    char *filenameEVENT = 0;

    theWall = new ConcreteShearWall();
    theWall->initConcrete(beta, An, Ap, Bn);
    theWall->readBIM(filenameEVENT, bimFileName.toStdString().c_str());
    theWall->writeSAM(samFileName.toStdString().c_str(), nL, nH, nW);

    printf("SAM file created successfully. \n");

}

void MainWindow::preprocess()
{
    QString bimFileName = expDirName + "/BIM.json";
    QString samFileName = expDirName + "/SAM.json";
    QString evtFileName = expDirName + "/EVT.json";
    QString edpFileName = expDirName + "/EDP.json";
    QString tclFileName = expDirName + "/wall.tcl";

    QByteArray bimFileName_ba = bimFileName.toLatin1();
    QByteArray samFileName_ba = samFileName.toLatin1();
    QByteArray evtFileName_ba = evtFileName.toLatin1();
    QByteArray edpFileName_ba = edpFileName.toLatin1();
    QByteArray tclFileName_ba = tclFileName.toLatin1();

    char *filenameBIM = bimFileName_ba.data();
    char *filenameSAM = samFileName_ba.data();
    char *filenameEVENT = evtFileName_ba.data();
    char *filenameEDP = edpFileName_ba.data();
    char *filenameTCL = tclFileName_ba.data();


    //delete thePreprocessor;
    thePreprocessor = new OpenSeesTclBuilder();
    string ModelType = thePreprocessor->getModelType(filenameSAM);// continuum or beamcolumn?


    if (!ModelType.compare("beamcolumn"))
    {
        std::cout << "Model type: " << ModelType << endl;
        thePreprocessor->createInputFileBeamColumn(filenameBIM,
                                                   filenameSAM,
                                                   filenameEVENT,
                                                   filenameEDP,
                                                   filenameTCL);

    }else if(!ModelType.compare("continuum")){
        std::cout << "Model type: " << ModelType << endl;

        thePreprocessor->createInputFile(filenameBIM,
                                         filenameSAM,
                                         filenameEVENT,
                                         filenameEDP,
                                         filenameTCL);

    }else{
        std::cout << "No matching model type." << endl;
    }





}

void MainWindow::getSAM()
{
    QString samFileName = expDirName + "/SAM.json";
    QString in;
    QFile inputFile(samFileName);
    if(inputFile.open(QFile::ReadOnly)) {
    //inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    in = inputFile.readAll();
    inputFile.close();
    }else{
        QMessageBox::information(this,tr("SWIM Information"), "No SAM.json found.", tr("I know."));
    }

    QJsonDocument indoc = QJsonDocument::fromJson(in.toUtf8());
    //qWarning() << indoc.isNull();
    if (indoc.isNull())
    {
        qWarning() << "SAM.json is missing.";
    }
    else{
        samRoot = indoc.object();

        SAM = samRoot["Structural Analysis Model"].toObject();
        geometry = SAM["geometry"].toObject();
        properties = SAM["properties"].toObject();
        nodeMapping = SAM["nodeMapping"].toObject();
        uniaxialMaterials = properties["uniaxialMaterials"].toArray();


        ndMaterials = properties["ndMaterials"].toArray();
        for(auto ndmat : ndMaterials)
        {
            int matID = ndmat.toObject()["name"].toInt();
            if(ndmat.toObject()["type"].toString()=="PlaneStressRebar")
                rebarMaterials.insert(matID, ndmat.toObject());
            if(ndmat.toObject()["type"].toString()=="Concrete")
                concreteMaterials.insert(matID, ndmat.toObject());
            if(ndmat.toObject()["type"].toString()=="LayeredConcrete")
                rcMaterials.insert(matID, ndmat.toObject());
        }
    }

}

void MainWindow::createSAMui()
{
    // create SAM ui
    QString blank(tr(" "));
    QString inch(tr("inch "));


    // steel01
    // Steel01 (int tag, double fy, double E0, double b, double a1, double a2, double a3, double a4)
    QGroupBox *floorBox = new QGroupBox("Assign materials models to wall components");
    QGridLayout *floorLay = new QGridLayout();

    // web
    QGroupBox *webBox = new QGroupBox("Material models");
    webLay = new QGridLayout();

    // get rcMatStrList
    QMapIterator<int, QJsonObject> i(rebarMaterials);
    while (i.hasNext()) {
        i.next();
        QJsonObject rebarObj = i.value();
        rcMatStrList.append(QString::number(rebarObj["name"].toInt()));
    }
    i = concreteMaterials;
    while (i.hasNext()) {
        i.next();
        QJsonObject rebarObj = i.value();
        rcMatStrList.append(QString::number(rebarObj["name"].toInt()));
    }


    // rc
    //QMapIterator<int, QJsonObject> i(rcMaterials);
    i = rcMaterials;
    int rcJsonInd = 0;
    while (i.hasNext()) {
        i.next();
        int rcID = i.key();


        QGroupBox *RCWebBox = new QGroupBox("");
        QGridLayout *RCWebLay = new QGridLayout();
        QJsonObject theObj = i.value();

        QStringList theList;
        QJsonArray rclayersJson = theObj["layers"].toArray();
        QVector<double> theTickness;
        for (auto l : rclayersJson)
        {
            theList.append(QString::number(l.toObject()["material"].toInt()));
            theTickness.append(l.toObject()["thickness"].toDouble());
        }
        matIDList_rc_mat1.insert(rcID, theList);


        rct1Edt.insert(rcID, addDoubleSpin(tr("Layer 1 thickness"),&inch,RCWebLay,0,0));
        rcMat1Edt.insert(rcID, addCombo(tr("Layer 1 mat id"),rcMatStrList,&blank,RCWebLay,1,0));
        rct2Edt.insert(rcID, addDoubleSpin(tr("Layer 2 thickness"),&inch,RCWebLay,2,0));
        rcMat2Edt.insert(rcID, addCombo(tr("Layer 2 mat id"),rcMatStrList,&blank,RCWebLay,3,0));
        rct3Edt.insert(rcID, addDoubleSpin(tr("Layer 3 thickness"),&inch,RCWebLay,4,0));
        rcMat3Edt.insert(rcID, addCombo(tr("Layer 3 mat id"),rcMatStrList,&blank,RCWebLay,5,0));

        rct1Edt[rcID]->setRange(0.,1e15); rct1Edt[rcID]->setDecimals(6);
        rct2Edt[rcID]->setRange(0.,1e15); rct2Edt[rcID]->setDecimals(6);
        rct3Edt[rcID]->setRange(0.,1e15); rct3Edt[rcID]->setDecimals(6);

        rct1Edt[rcID]->setValue(theTickness[0]);
        rct2Edt[rcID]->setValue(theTickness[1]);
        rct3Edt[rcID]->setValue(theTickness[2]);
        rcMat1Edt[rcID]->setCurrentText(theList[0]);
        rcMat2Edt[rcID]->setCurrentText(theList[1]);
        rcMat3Edt[rcID]->setCurrentText(theList[2]);


        RCWebLay->setColumnStretch(1,1);
        RCWebBox->setLayout(RCWebLay);

        connect(rct1Edt[rcID], SIGNAL(valueChanged(double)), this, SLOT(rc_valueChanged_SAM(double)));
        connect(rct2Edt[rcID], SIGNAL(valueChanged(double)), this, SLOT(rc_valueChanged_SAM(double)));
        connect(rct3Edt[rcID], SIGNAL(valueChanged(double)), this, SLOT(rc_valueChanged_SAM(double)));
        connect(rcMat1Edt[rcID], SIGNAL(currentTextChanged(QString)), this, SLOT(rc_valueChanged_SAM(QString)));
        connect(rcMat2Edt[rcID], SIGNAL(currentTextChanged(QString)), this, SLOT(rc_valueChanged_SAM(QString)));
        connect(rcMat3Edt[rcID], SIGNAL(currentTextChanged(QString)), this, SLOT(rc_valueChanged_SAM(QString)));

        rcBoxSAMs.insert(rcID, RCWebBox);

        webLay->addWidget(RCWebBox,1,0, Qt::AlignTop);
        webLay->setRowStretch(1,1);

        RCWebBox->hide();
        matIDList_rc.append(QString::number(rcID));

        rcJsonInd++;

    }
    rcBoxSAMs.first()->show();




    // concrete
    //QMapIterator<int, QJsonObject> i(concreteMaterials);
    i = concreteMaterials;
    while (i.hasNext()) {
        i.next();
        int concreteID = i.key();


        QGroupBox *concreteWebBox = new QGroupBox("");
        QGridLayout *concreteWebLay = new QGridLayout();
        QJsonObject rebarObj = i.value();

        concreteEEdt.insert(concreteID, addDoubleSpin(tr("E"),&blank,concreteWebLay,0,0));
        concretefpcEdt.insert(concreteID, addDoubleSpin(tr("fpc"),&blank,concreteWebLay,1,0));
        concretenuEdt.insert(concreteID, addDoubleSpin(tr("nu"),&blank,concreteWebLay,2,0));
        concretebetaEdt.insert(concreteID, addDoubleSpin(tr("beta"),&blank,concreteWebLay,3,0));
        concreteApEdt.insert(concreteID, addDoubleSpin(tr("Ap"),&blank,concreteWebLay,4,0));
        concreteAnEdt.insert(concreteID, addDoubleSpin(tr("An"),&blank,concreteWebLay,5,0));
        concreteBnEdt.insert(concreteID, addDoubleSpin(tr("Bn"),&blank,concreteWebLay,6,0));

        concreteEEdt[concreteID]->setRange(0.,1e15); concreteEEdt[concreteID]->setDecimals(6);
        concretefpcEdt[concreteID]->setRange(0.,1e15); concretefpcEdt[concreteID]->setDecimals(6);
        concretenuEdt[concreteID]->setRange(0.,1e15); concretenuEdt[concreteID]->setDecimals(6);
        concretebetaEdt[concreteID]->setRange(0.,1e15); concretebetaEdt[concreteID]->setDecimals(6);
        concreteApEdt[concreteID]->setRange(0.,1e15); concreteApEdt[concreteID]->setDecimals(6);
        concreteAnEdt[concreteID]->setRange(0.,1e15); concreteAnEdt[concreteID]->setDecimals(6);
        concreteBnEdt[concreteID]->setRange(0.,1e15); concreteBnEdt[concreteID]->setDecimals(6);

        concreteEEdt[concreteID]->setValue(rebarObj["E"].toDouble());
        concretefpcEdt[concreteID]->setValue(rebarObj["fpc"].toDouble());
        concretenuEdt[concreteID]->setValue(rebarObj["nu"].toDouble());
        concretebetaEdt[concreteID]->setValue(rebarObj["beta"].toDouble());
        concreteApEdt[concreteID]->setValue(rebarObj["Ap"].toDouble());
        concreteAnEdt[concreteID]->setValue(rebarObj["An"].toDouble());
        concreteBnEdt[concreteID]->setValue(rebarObj["Bn"].toDouble());

        concreteWebLay->setColumnStretch(1,1);
        concreteWebBox->setLayout(concreteWebLay);




        connect(concreteEEdt[concreteID], SIGNAL(valueChanged(double)), this, SLOT(concrete_valueChanged_SAM(double)));
        connect(concretefpcEdt[concreteID], SIGNAL(valueChanged(double)), this, SLOT(concrete_valueChanged_SAM(double)));
        connect(concretenuEdt[concreteID], SIGNAL(valueChanged(double)), this, SLOT(concrete_valueChanged_SAM(double)));

        connect(concretebetaEdt[concreteID], SIGNAL(valueChanged(double)), this, SLOT(concrete_valueChanged_SAM(double)));
        connect(concreteApEdt[concreteID], SIGNAL(valueChanged(double)), this, SLOT(concrete_valueChanged_SAM(double)));
        connect(concreteAnEdt[concreteID], SIGNAL(valueChanged(double)), this, SLOT(concrete_valueChanged_SAM(double)));
        connect(concreteBnEdt[concreteID], SIGNAL(valueChanged(double)), this, SLOT(concrete_valueChanged_SAM(double)));

        connect(concretebetaEdt[concreteID], SIGNAL(valueChanged(double)), this, SLOT(beta_valueChanged_SAM(double)));
        connect(concreteApEdt[concreteID], SIGNAL(valueChanged(double)), this, SLOT(Ap_valueChanged_SAM(double)));
        connect(concreteAnEdt[concreteID], SIGNAL(valueChanged(double)), this, SLOT(An_valueChanged_SAM(double)));
        connect(concreteBnEdt[concreteID], SIGNAL(valueChanged(double)), this, SLOT(Bn_valueChanged_SAM(double)));

        concreteBoxSAMs.insert(concreteID, concreteWebBox);

        webLay->addWidget(concreteWebBox,1,1, Qt::AlignTop);
        webLay->setRowStretch(1,1);


        concreteWebBox->hide();
        matIDList_concrete.append(QString::number(concreteID));

    }
    concreteBoxSAMs.first()->show();

    // rebar layout
    //QMapIterator<int, QJsonObject> i(rebarMaterials);
    // steel
    i = rebarMaterials;
    QStringList matIDList_rebar_steel;
    while (i.hasNext()) {
        i.next();
        int rebarID = i.key();
        matIDList_rebar_steel.append(QString::number(rebarMaterials[rebarID]["material"].toInt()));
    }
    i = rebarMaterials;
    while (i.hasNext()) {
        i.next();
        int rebarID = i.key();
        QGroupBox *rebarWebBox = new QGroupBox("");
        QGridLayout *rebarWebLay = new QGridLayout();
        QJsonObject rebarObj = i.value();
        rebarSteelMatEdt.insert(rebarID, addCombo(tr("Steel model"),matIDList_rebar_steel,&blank,rebarWebLay,0,0));
        rebarAngEdt.insert(rebarID, addDoubleSpin(tr("Rebar angle"),&blank,rebarWebLay,1,0));
        rebarAngEdt[rebarID]->setRange(0.0, 1e15);
        rebarAngEdt[rebarID]->setDecimals(2);

        rebarSteelMatEdt[rebarID]->setCurrentText(QString::number(rebarMaterials[rebarID]["material"].toInt()));
        rebarAngEdt[rebarID]->setValue(rebarMaterials[rebarID]["angle"].toDouble());

        rebarWebLay->setColumnStretch(1,1);
        rebarWebBox->setLayout(rebarWebLay);

        connect(rebarSteelMatEdt[rebarID], SIGNAL(currentTextChanged(QString)), this, SLOT(rebar_valueChanged_SAM(QString)));
        connect(rebarAngEdt[rebarID], SIGNAL(valueChanged(double)), this, SLOT(rebar_valueChanged_SAM(double)));

        rebarBoxSAMs.insert(rebarID, rebarWebBox);

        //webLay->addWidget(rebarWebBox,1,2, Qt::AlignTop);
        webLay->addWidget(rebarWebBox,3,0, Qt::AlignTop);
        webLay->setRowStretch(1,1);

        rebarWebBox->hide();
        matIDList_rebar.append(QString::number(rebarID));

    }
    rebarBoxSAMs.first()->show();



    // steel
    for (int i=0; i < uniaxialMaterials.size(); i++)
    {
        /*
        int steelName = uniaxialMaterials[i].toObject()["name"].toInt();
        QString steeltype = uniaxialMaterials[i].toObject()["type"].toString();
        double E = uniaxialMaterials[i].toObject()["name"].toDouble();
        double fy = uniaxialMaterials[i].toObject()["fy"].toDouble();
        double b = uniaxialMaterials[i].toObject()["b"].toDouble();
        */

        QGroupBox *steelWebBox = new QGroupBox("");
        QGridLayout *steelWebLay = new QGridLayout();
        steelTyeEdt.append(addCombo("Material model", steelModelTypes,&blank, steelWebLay,0,0));
        steelTyeEdt[i]->setToolTip(tr("Material model"));
        steelEEdt.append(addDoubleSpin(tr("E"),&blank,steelWebLay,1,0));
        steelEEdt[i]->setMaximum(1e20);steelEEdt[i]->setDecimals(6);
        steelEEdt[i]->setToolTip(tr("Young's modulus"));
        steelEEdt[i]->setValue(uniaxialMaterials[i].toObject()["E"].toDouble());
        steelfyEdt.append(addDoubleSpin(tr("fy"),&blank,steelWebLay,2,0));
        steelfyEdt[i]->setMaximum(1e20);steelfyEdt[i]->setDecimals(6);
        steelfyEdt[i]->setToolTip(tr("Yield strength"));
        double aa= uniaxialMaterials[i].toObject()["fy"].toDouble();
        steelfyEdt[i]->setValue(uniaxialMaterials[i].toObject()["fy"].toDouble());
        steelbEdt.append(addDoubleSpin(tr("b"),&blank,steelWebLay,3,0));
        steelbEdt[i]->setMaximum(1e20); steelbEdt[i]->setDecimals(6);
        steelbEdt[i]->setToolTip(tr("hardening"));
        steelbEdt[i]->setValue(uniaxialMaterials[i].toObject()["b"].toDouble());

        connect(steelEEdt[i], SIGNAL(valueChanged(double)), this, SLOT(steel_valueChanged_SAM(double)));
        connect(steelfyEdt[i], SIGNAL(valueChanged(double)), this, SLOT(steel_valueChanged_SAM(double)));
        connect(steelbEdt[i], SIGNAL(valueChanged(double)), this, SLOT(steel_valueChanged_SAM(double)));

        steelWebLay->setRowStretch(i,1);
        steelWebLay->setColumnStretch(0,1);
        steelWebBox->setLayout(steelWebLay);

        steelBoxSAMs.append(steelWebBox);

        //webLay->addWidget(steelWebBox,1,3, Qt::AlignTop);
        webLay->addWidget(steelWebBox,3,1, Qt::AlignTop);
        webLay->setRowStretch(1+i,1);

        steelWebBox->hide();
        matIDList_steel.append(QString::number(i+1));
    }
    steelBoxSAMs[0]->show();





    // adding mat id selectors
    matIDselector_rc = addCombo(tr("Reinforced concrete layout "),matIDList_rc,&blank,webLay,0,0);
    matIDselector_concrete = addCombo(tr("Concrete model "),matIDList_concrete,&blank,webLay,0,1);
    //matIDselector_rebar = addCombo(tr("Rebar layout "),matIDList_rebar,&blank,webLay,0,2);
    //matIDselector_steel = addCombo(tr("Steel model "),matIDList_steel,&blank,webLay,0,3);
    matIDselector_rebar = addCombo(tr("Rebar layout "),matIDList_rebar,&blank,webLay,2,0);
    matIDselector_steel = addCombo(tr("Steel model "),matIDList_steel,&blank,webLay,2,1);







    webBox->setLayout(webLay);

    webLay->setColumnStretch(0,1);
    webLay->setColumnStretch(1,1);
    webLay->setColumnStretch(2,1);
    webLay->setColumnStretch(3,1);

    connect(matIDselector_rc, SIGNAL(currentTextChanged(QString)), this, SLOT(rcSelector_valueChanged_SAM(QString)));
    connect(matIDselector_concrete, SIGNAL(currentTextChanged(QString)), this, SLOT(concreteSelector_valueChanged_SAM(QString)));
    connect(matIDselector_rebar, SIGNAL(currentTextChanged(QString)), this, SLOT(rebarSelector_valueChanged_SAM(QString)));
    connect(matIDselector_steel, SIGNAL(currentTextChanged(QString)), this, SLOT(steelSelector_valueChanged_SAM(QString)));



    // Web add material
    for (int j=0; j< numFloors; j++)
    {
        webRCselector.append(addCombo(tr("Reinforced concrete layout for web"),matIDList_rc,&blank,floorLay,0,0));
        beRCselector.append(addCombo(tr("Reinforced concrete layout for boundary"),matIDList_rc,&blank,floorLay,1,0));
        webRCselector[j]->setCurrentText(matIDList_rc[0+2*j]);
        beRCselector[j]->setCurrentText(matIDList_rc[1+2*j]);
        webRCselector[j]->setEditable(true);
        beRCselector[j]->setEditable(true);
        webRCselector[j]->hide();
        beRCselector[j]->hide();
        connect(webRCselector[j], SIGNAL(currentTextChanged(QString)), this, SLOT(webRCselector_valueChanged_SAM(QString)));
        connect(beRCselector[j], SIGNAL(currentTextChanged(QString)), this, SLOT(beRCselector_valueChanged_SAM(QString)));
    }
    webRCselector[0]->show();
    beRCselector[0]->show();






    //floorLay->addWidget(webBox,1,1, Qt::AlignTop);
    wallSAMLay->addWidget(webBox,1,0, Qt::AlignTop);



    floorLay->setRowStretch(0,1);
    floorLay->setRowStretch(1,1);
    floorLay->setRowStretch(2,1);
    floorLay->setRowStretch(3,1);
    floorBox->setLayout(floorLay);

    wallSAMLay->addWidget(floorBox,1+1+1,0,1,1);

    floorSAMs.append(floorBox);

    openseespathEdt = addLineEdit("FEM engine", "", wallSAMLay,1+1+1+1,0);
    connect(openseespathEdt, SIGNAL(textChanged(QString)), this, SLOT(openseespathEdt_textChanged(QString)));


    //wallSAMLay->setRowStretch(1,1);
    //wallSAMLay->setRowStretch(2,1);

    /*
    for (int i=1; i<numFloors; i++){
        floorSAMs[i]->hide();
    }
    floorSAMs[0]->show();
    */

}

void MainWindow::openseespathEdt_textChanged(QString newPath)
{
    openseespath = newPath;
}

std::vector<float> MainWindow::getAIinputs()
{
    //['height', 'length', 'thickness', 'E', 'fc']
    float wallLength = float(theWall->lengthofWall);
    float wallHeight = float(theWall->heightofWall);
    float wallThick = float(theWall->theWallSections.begin()->second->thickness);
    float E_Concrete = float(theWall->E_Concrete);
    float fc_Concrete = float(theWall->fc_Concrete);
    std::vector<float> AIinputsVec{wallLength,wallHeight, wallThick, E_Concrete, fc_Concrete};
    return AIinputsVec;
}

void MainWindow::webRCselector_valueChanged_SAM(QString)
{
    assemble_valueChanged_SAM();
}
void MainWindow::beRCselector_valueChanged_SAM(QString)
{
    assemble_valueChanged_SAM();
}
void MainWindow::assemble_valueChanged_SAM()
{
    //int currentFloorID = idFloorEdt_SAM->value();
}

void MainWindow::showRebar()
{

    //rebarBoxSAMs[1]->hide();
    rebarBoxSAMs[2]->hide();
    rebarBoxSAMs[3]->hide();

    QMap<int, QGroupBox*> rcBoxSAMs;




}

void MainWindow::showSteel()
{
    /*
    for (int i=0; i<steelBoxSAMs.size(); i++){
        steelBoxSAMs[i]->hide();
    }
    if(matIDselector_steel->value()>-1)
        steelBoxSAMs[matIDselector_steel->value()-1]->show();
    else
        steelBoxSAMs[0]->show();
     */
}
void MainWindow::showSteel(int steelID)
{
    for (int i=0; i<steelBoxSAMs.size(); i++){
        steelBoxSAMs[i]->hide();
    }
    if(steelID>-1)
        steelBoxSAMs[steelID-1]->show();
    else
        steelBoxSAMs[0]->show();
}

void MainWindow::rc_valueChanged_SAM(double)
{
    rc_valueChanged_SAM();
}
void MainWindow::rc_valueChanged_SAM(QString)
{
    rc_valueChanged_SAM();
}
void MainWindow::rc_valueChanged_SAM()
{
    int currentrcID =  matIDselector_rc->currentText().toInt();

    QJsonObject rcTmpJ;
    QJsonObject rcTmpJLayer;

    for (auto rcj : rcMaterials)
    {
        int rcName = rcj["name"].toInt();
        QString rctype = rcj["type"].toString();

        if(currentrcID == rcName)
        {
            rcj["name"] = rcName;
            rcj["type"] = rctype;

            QJsonArray rcTmpJArray;
            rcTmpJLayer["thickness"] = rct1Edt[currentrcID]->value();
            rcTmpJLayer["material"] = rcMat1Edt[currentrcID]->currentText().toInt();
            rcTmpJArray.append(rcTmpJLayer);
            rcTmpJLayer["thickness"] = rct2Edt[currentrcID]->value();
            rcTmpJLayer["material"] = rcMat2Edt[currentrcID]->currentText().toInt();
            rcTmpJArray.append(rcTmpJLayer);
            rcTmpJLayer["thickness"] = rct3Edt[currentrcID]->value();
            rcTmpJLayer["material"] = rcMat3Edt[currentrcID]->currentText().toInt();
            rcTmpJArray.append(rcTmpJLayer);

            rcj["layers"] = rcTmpJArray;

            rcMaterials[rcName] = rcj;
        }
    }

    // write out for testing
    QString tmpfile = expDirName + "/rc_SAM_debug.json";
    QFile jsonFile(tmpfile);
    jsonFile.open(QFile::WriteOnly);
    for (auto rcj : rcMaterials)
    {
        QJsonDocument tmpDoc = QJsonDocument(rcj);
        jsonFile.write(tmpDoc.toJson());
    }
    jsonFile.close();

}

void MainWindow::beta_valueChanged_SAM(double){
    beta = concretebetaEdt.first()->value();
    concretebetaEdt.first()->setStyleSheet("");
}

void MainWindow::Ap_valueChanged_SAM(double){Ap = concreteApEdt.first()->value();
                                            concreteApEdt.first()->setStyleSheet("");}
void MainWindow::An_valueChanged_SAM(double){An = concreteAnEdt.first()->value();
                                            concreteAnEdt.first()->setStyleSheet("");}
void MainWindow::Bn_valueChanged_SAM(double){Bn = concreteBnEdt.first()->value();
                                            concreteBnEdt.first()->setStyleSheet("");}

void MainWindow::concrete_valueChanged_SAM(double){
    int currentConcreteID =  matIDselector_concrete->currentText().toInt();

    QJsonObject concreteTmpJ;
    for (auto concretej : concreteMaterials)
    {
        int concreteName = concretej["name"].toInt();
        QString concretetype = concretej["type"].toString();
        int Etmp = concretej["E"].toInt();
        double fpctmp = concretej["fpc"].toDouble();
        if(currentConcreteID == concreteName)
        {
            concretej["name"] = concreteName;
            concretej["type"] = concretetype;
            concretej["E"] = concreteEEdt[currentConcreteID]->value();
            concretej["fpc"] = concretefpcEdt[currentConcreteID]->value();
            concretej["nu"] = concretenuEdt[currentConcreteID]->value();
            concretej["beta"] = concretebetaEdt[currentConcreteID]->value();
            concretej["Ap"] = concreteApEdt[currentConcreteID]->value();
            concretej["An"] = concreteAnEdt[currentConcreteID]->value();
            concretej["Bn"] = concreteBnEdt[currentConcreteID]->value();

            concreteMaterials[concreteName] = concretej;
        }
    }

    // write out for testing
    QString tmpfile = expDirName + "/concrete_SAM_debug.json";
    QFile jsonFile(tmpfile);
    jsonFile.open(QFile::WriteOnly);
    for (auto concretej : concreteMaterials)
    {
        QJsonDocument tmpDoc = QJsonDocument(concretej);
        jsonFile.write(tmpDoc.toJson());
    }
    jsonFile.close();

}
void MainWindow::rebar_valueChanged_SAM()
{

    // when steel panel is edited, update the obj uniaxialMaterials
    int currentFloorID = idFloorEdt_SAM->value();
    int currentRebarID =  matIDselector_rebar->currentText().toInt();
    QJsonObject rebarTmpJ;
    for (auto rebarj : rebarMaterials)
    {
        int rebarName = rebarj["name"].toInt();
        QString rebartype = rebarj["type"].toString();
        int steelIDtmp = rebarj["material"].toInt();
        double angletmp = rebarj["angle"].toDouble();
        if(currentRebarID == rebarName)
        {
            rebarTmpJ["name"] = rebarName;
            rebarTmpJ["type"] = rebartype;
            rebarTmpJ["angle"] = rebarAngEdt[currentRebarID]->value();
            rebarTmpJ["material"] = rebarSteelMatEdt[currentRebarID]->currentText().toInt();
            rebarMaterials[rebarName] = rebarTmpJ;
        }
    }

    // write out for testing
    QString tmpfile = expDirName + "/rebar_SAM_debug.json";
    QFile jsonFile(tmpfile);
    jsonFile.open(QFile::WriteOnly);
    for (auto rebarj : rebarMaterials)
    {
        QJsonDocument tmpDoc = QJsonDocument(rebarj);
        jsonFile.write(tmpDoc.toJson());
    }
    jsonFile.close();

}
void MainWindow::rebar_valueChanged_SAM(double)
{
    rebar_valueChanged_SAM();
}
void MainWindow::rebar_valueChanged_SAM(QString)
{
    rebar_valueChanged_SAM();
}


// when use edit steel properties
void MainWindow::steel_valueChanged_SAM(double value)
{

    // when steel panel is edited, update the obj uniaxialMaterials
    //int currentFloorID = idFloorEdt_SAM->value();
    int currentSteelID = matIDselector_steel->currentText().toInt();//  matIDselector_steel[currentFloorID-1].currentText().toInt();
    QJsonObject steelTmpJ;
    QJsonArray newuniaxialMaterials;
    for (auto steelj : uniaxialMaterials)
    {
        int steelName = steelj.toObject()["name"].toInt();
        QString steeltype = steelj.toObject()["type"].toString();
        double E = steelj.toObject()["E"].toDouble();
        double fy = steelj.toObject()["fy"].toDouble();
        double b = steelj.toObject()["b"].toDouble();
        if(currentSteelID == steelName)
        {
            steelTmpJ["name"] = steelName;
            steelTmpJ["type"] = steeltype;
            steelTmpJ["E"] = double(steelEEdt[currentSteelID-1]->value());
            steelTmpJ["fy"] = steelfyEdt[currentSteelID-1]->value();
            steelTmpJ["b"] = steelbEdt[currentSteelID-1]->value();
            newuniaxialMaterials.append(steelTmpJ);
        } else {
            newuniaxialMaterials.append(steelj);
        }
    }
    uniaxialMaterials = newuniaxialMaterials;

    // write out for debug
    QString tmpfile = expDirName + "/tmpSAM.json";
    QJsonDocument tmpDoc = QJsonDocument(uniaxialMaterials);
    QJsonDocument tmpDoc_ND = QJsonDocument(ndMaterials);
    QFile jsonFile(tmpfile);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(tmpDoc.toJson());
    jsonFile.write(tmpDoc_ND.toJson());
    jsonFile.close();

}

void MainWindow::rcSelector_valueChanged_SAM(QString rcID)
{
    int thisrcID = rcID.toInt();

    QMapIterator<int, QGroupBox*> i(rcBoxSAMs);
    while (i.hasNext()) {
        i.next();
        i.value()->hide();
    }

    rcBoxSAMs[thisrcID]->show();

}
void MainWindow::concreteSelector_valueChanged_SAM(QString concreteID)
{
    int thisConcreteID = concreteID.toInt();

    QMapIterator<int, QGroupBox*> i(concreteBoxSAMs);
    while (i.hasNext()) {
        i.next();
        i.value()->hide();
    }

    concreteBoxSAMs[thisConcreteID]->show();

}
void MainWindow::rebarSelector_valueChanged_SAM(QString rebarID)
{
    int thisRebarID = rebarID.toInt();

    QMapIterator<int, QGroupBox*> i(rebarBoxSAMs);
    while (i.hasNext()) {
        i.next();
        i.value()->hide();

    }

    rebarBoxSAMs[thisRebarID]->show();

}
void MainWindow::steelSelector_valueChanged_SAM(QString steelIDStr)
{
    /*
    int currentFloorID = idFloorEdt_SAM->value();

    int steelName = uniaxialMaterials[steelID-1].toObject()["name"].toInt();

    QString steeltype = uniaxialMaterials[steelID-1].toObject()["type"].toString();
    double E = uniaxialMaterials[steelID-1].toObject()["E"].toDouble();
    double fy = uniaxialMaterials[steelID-1].toObject()["fy"].toDouble();
    double b = uniaxialMaterials[steelID-1].toObject()["b"].toDouble();

    steelEEdt[steelID-1]->setValue(E);
    steelfyEdt[steelID-1]->setValue(fy);
    steelbEdt[steelID-1]->setValue(b);
    */

    int steelID = steelIDStr.toInt();

    //steelBoxSAMs[i]->hide();
    if (steelID>0)
    {
        for (int i=0; i<steelBoxSAMs.size(); i++)
        {
            if(fabs((steelID-1)-i)>0.1 )
                steelBoxSAMs[i]->hide();
            else
                steelBoxSAMs[i]->show();
        }
    }
    else
    {
        for (int i=0; i<steelBoxSAMs.size(); i++){
            steelBoxSAMs[i]->hide();}
        steelBoxSAMs[0]->show();
    }


}

void MainWindow::createBIMui()
{

    // create SAM ui
    QString blank(tr(" "));
    QString inch(tr("inch "));

    numMatsBIM = int(theWall->matsBIM.size());
    map<string, ConcreteRectangularWallSection *>::iterator theWallSections_itr = theWall->theWallSections.begin();
    for(int i=0; i < numFloors; i++)
    {
        // steel01
        // Steel01 (int tag, double fy, double E0, double b, double a1, double a2, double a3, double a4)
        QGroupBox *floorBox = new QGroupBox("Floor "+QString::number(i+1));

        QGridLayout *floorLay = new QGridLayout();




        // Geometry
        QGroupBox *geoBox = new QGroupBox("Geometry ");
        QGridLayout *geoLay = new QGridLayout();

        QDoubleSpinBox *wallLenghtEdt = addDoubleSpin(tr("Length"),&inch,geoLay,0,0);
        wallLenghtEdt->setToolTip(tr("Horizontal dimension"));
        wallLenghtEdt->setValue(theWall->lengthofWall);
        wallLenghtEdt->setDisabled(true);
        QDoubleSpinBox *wallHeightEdt = addDoubleSpin(tr("Height"),&inch,geoLay,1,0);
        wallHeightEdt->setToolTip(tr("Vertical dimension"));
        wallHeightEdt->setValue(theWall->floorHeights[i]);
        wallHeightEdt->setDisabled(true);
        QDoubleSpinBox *wallThicknessEdt = addDoubleSpin(tr("Thickness"),&inch,geoLay,2,0);
        wallThicknessEdt->setToolTip(tr(""));
        wallThicknessEdt->setValue(theWall->theWallSections.begin()->second->thickness);
        wallThicknessEdt->setDisabled(true);
        QDoubleSpinBox *wallbelengEdt = addDoubleSpin(tr("Boundary length"),&inch,geoLay,3,0);
        wallbelengEdt->setToolTip(tr(""));
        wallbelengEdt->setValue(theWall->theWallSections.begin()->second->be_length);
        wallbelengEdt->setDisabled(true);
        geoLay->setColumnStretch(1,1);
        geoBox->setLayout(geoLay);

        wallLength = wallLenghtEdt->value();
        beLength = wallbelengEdt->value();
        webLength = wallLength-beLength*2.0;




        // Material
        QGroupBox *matBIMBox = new QGroupBox("Material");
        QGridLayout *matBIMLay = new QGridLayout();


        QStringList matNamesBIM;
        for (int matIDBIM=0;matIDBIM<numMatsBIM;matIDBIM++)
        {
            matNamesBIM.append(QString::fromStdString(theWall->matsBIM[matIDBIM]["name"]));
        }
        matSelectorBIM.append( addCombo(tr("Material name: "),matNamesBIM,&blank,matBIMLay,0,0,1,2));
        connect(matSelectorBIM[i], SIGNAL(currentIndexChanged(int)), this, SLOT(matSelectorBIM_valueChanged_BIM(int)));

        // adding each material to a box ui
        map<string, string>::iterator itr;
        QVector<QGroupBox*> tmpVecBox;
        for (int matIDBIM=0;matIDBIM<theWall->matsBIM.size();matIDBIM++)
        {

            QGroupBox *matBIMWebBox = new QGroupBox("");
            QGridLayout *matBIMWebLay = new QGridLayout();
            int vIDtmp = 0;
            for (itr = theWall->matsBIM[matIDBIM].begin(); itr != theWall->matsBIM[matIDBIM].end(); ++itr) {
                QLineEdit *expNameEdt = addLineEdit(QString::fromStdString(itr->first), QString::fromStdString(itr->second), matBIMWebLay,vIDtmp,0);
                expNameEdt->setDisabled(true);
                vIDtmp++;
            }
            matBIMWebBox->setLayout(matBIMWebLay);
            tmpVecBox.append(matBIMWebBox);

            matBIMLay->addWidget(matBIMWebBox,matIDBIM+1,1);
        }
        matBIMs.append(tmpVecBox);

        for (int j=1; j<theWall->matsBIM.size(); j++){
            matBIMs[i][j]->hide();
        }
        matBIMs[i][0]->show();







        matBIMBox->setLayout(matBIMLay);


        // Layout
        QGroupBox *layoutBIMBox = new QGroupBox("Rebar layout ");
        QGridLayout *layoutBIMLay = new QGridLayout();


        QGroupBox *layoutBIMWebBox = new QGroupBox("Web layout");
        QGridLayout *layoutBIMWebLay = new QGridLayout();

        QGroupBox *layoutBIMWebBox_long = new QGroupBox("Longitudinal");
        QGridLayout *layoutBIMWebLay_long = new QGridLayout();
        map<string, string>::iterator itrInnertmp;
        int vIDtmp=0;
        for (itrInnertmp = theWallSections_itr->second->longitudinalRebar_map.begin(); itrInnertmp != theWallSections_itr->second->longitudinalRebar_map.end(); ++itrInnertmp) {
            QLineEdit *expNameEdt = addLineEdit(QString::fromStdString(itrInnertmp->first), QString::fromStdString(itrInnertmp->second), layoutBIMWebLay_long,vIDtmp,0);
            cout << itrInnertmp->first << endl;
            expNameEdt->setDisabled(true);
            vIDtmp++;
        }
        //theWallSections_itr++;
        layoutBIMWebBox_long->setLayout(layoutBIMWebLay_long);

        //layoutBIMBox->hide();



        QGroupBox *layoutBIMWebBox_trans = new QGroupBox("Transverse");
        QGridLayout *layoutBIMWebLay_trans = new QGridLayout();
        vIDtmp=0;
        for (itrInnertmp = theWallSections_itr->second->transverseRebar_map.begin(); itrInnertmp != theWallSections_itr->second->transverseRebar_map.end(); ++itrInnertmp) {
            QLineEdit *expNameEdt = addLineEdit(QString::fromStdString(itrInnertmp->first), QString::fromStdString(itrInnertmp->second), layoutBIMWebLay_trans,vIDtmp,0);
            cout << itrInnertmp->first << endl;
            expNameEdt->setDisabled(true);
            vIDtmp++;
        }
        //theWallSections_itr++;
        layoutBIMWebBox_trans->setLayout(layoutBIMWebLay_trans);



        layoutBIMWebLay->addWidget(layoutBIMWebBox_long,1,1);
        layoutBIMWebLay->addWidget(layoutBIMWebBox_trans,1,2);



        //layoutBIMWebLay->setColumnStretch(2,1);
        layoutBIMWebBox->setLayout(layoutBIMWebLay);

        QGroupBox *layoutBIMBEBox = new QGroupBox("Boundary layout");
        QGridLayout *layoutBIMBELay = new QGridLayout();



        QGroupBox *layoutBIMBEBox_long = new QGroupBox("Longitudinal");
        QGridLayout *layoutBIMBELay_long = new QGridLayout();
        vIDtmp=0;
        for (itrInnertmp = theWallSections_itr->second->longitudinalRebar_map.begin(); itrInnertmp != theWallSections_itr->second->longitudinalRebar_map.end(); ++itrInnertmp) {
            QLineEdit *expNameEdt = addLineEdit(QString::fromStdString(itrInnertmp->first), QString::fromStdString(itrInnertmp->second), layoutBIMBELay_long,vIDtmp,0);
            cout << itrInnertmp->first << endl;
            expNameEdt->setDisabled(true);
            vIDtmp++;
        }
        //theWallSections_itr++;
        layoutBIMBEBox_long->setLayout(layoutBIMBELay_long);



        QGroupBox *layoutBIMBEBox_trans = new QGroupBox("Transverse");
        QGridLayout *layoutBIMBELay_trans = new QGridLayout();
        vIDtmp=0;
        for (itrInnertmp = theWallSections_itr->second->transverseRebar_map.begin(); itrInnertmp != theWallSections_itr->second->transverseRebar_map.end(); ++itrInnertmp) {
            QLineEdit *expNameEdt = addLineEdit(QString::fromStdString(itrInnertmp->first), QString::fromStdString(itrInnertmp->second), layoutBIMBELay_trans,vIDtmp,0);
            cout << itrInnertmp->first << endl;
            expNameEdt->setDisabled(true);
            vIDtmp++;
        }
        //theWallSections_itr++;
        layoutBIMBEBox_trans->setLayout(layoutBIMBELay_trans);



        layoutBIMBELay->addWidget(layoutBIMBEBox_long,1,1);
        layoutBIMBELay->addWidget(layoutBIMBEBox_trans,1,2);



        layoutBIMBELay->setColumnStretch(2,1);
        layoutBIMBEBox->setLayout(layoutBIMBELay);



        layoutBIMLay->addWidget(layoutBIMWebBox,1,1);
        layoutBIMLay->addWidget(layoutBIMBEBox,2,1);
        layoutBIMBox->setLayout(layoutBIMLay);


        floorLay->addWidget(geoBox,1,1);
        floorLay->addWidget(matBIMBox,2,1);
        floorLay->addWidget(layoutBIMBox,3,1);

        floorLay->setColumnStretch(1,1);
        floorBox->setLayout(floorLay);

        wallBIMLay->addWidget(floorBox,i+3,0,1,1);

        floorBIMs.append(floorBox);


    }
    for (int j=1; j<numFloors; j++){
        floorBIMs[j]->hide();
    }
    floorBIMs[0]->show();


}

// output panel
void MainWindow::createOutputPanel()
{
    //
    // deformed shape
    //

    /* FMK  moving to the tab widget
    QGroupBox *dispBox = new QGroupBox("Deformed Shape");
    QGridLayout *dispLay = new QGridLayout();
    dPlot = new deformWidget(tr("Length, Lwp"), tr("Deformation"));
    dispLay->addWidget(dPlot,0,0);
    dispBox->setLayout(dispLay);
    */


    //
    // Applied History - loading plot & slider
    //   - placed in a group box
    //

    QGroupBox *tBox = new QGroupBox("Applied Displacement History");
    QVBoxLayout *tLay = new QVBoxLayout();

    // loading plot
    tPlot = new historyWidget(tr("Pseudo-time"), tr("Applied History"));
    tLay->addWidget(tPlot);

    // slider
    slider = new QSlider(Qt::Horizontal);
    tLay->addWidget(slider);

    tBox->setLayout(tLay);

    //
    // hysteretic plot
    //

    QGroupBox *hystBox = new QGroupBox("Hysteretic Response");
    QVBoxLayout *hystLay = new QVBoxLayout();
    hPlot = new hysteresisWidget(tr("Lateral Deformation [in.]"), tr("Lateral Force [kips]"));
    hystLay->addWidget(hPlot,1);
    hystBox->setLayout(hystLay);

    //
    // Tab Widget containing axial, moment & soon to be curvature!
    //

    QGroupBox *dPlotBox = new QGroupBox("Shape");
    QVBoxLayout *dPlotLay = new QVBoxLayout();
    dPlot = new deformWidget(tr("Length, Lwp"), tr("Deformation"));
    dPlot->setExpDir(expDirName);
    dPlotLay->addWidget(dPlot,1);
    dPlotBox->setLayout(dPlotLay);


/*
    QTabWidget *tabWidget = new QTabWidget(this);
    // deformed shape plot
    dPlot = new deformWidget(tr("Length, Lwp"), tr("Deformation"));
    tabWidget->addTab(dPlot, "Shape");
    */

    /*
    // axial force plot
    pPlot = new responseWidget(tr("Length, Lwp"), tr("Axial Force"));
    tabWidget->addTab(pPlot, "Axial Force Diagram");

    // moment plot
    mPlot = new responseWidget(tr("Length, Lwp"), tr("Moment"));
    tabWidget->addTab(mPlot, "Moment Diagram");
*/
    // curvature plot
    // TO DO

    //
    // main output box
    //
    QGroupBox *outBox = new QGroupBox("Output");
    QVBoxLayout *outputLayout = new QVBoxLayout();


    /* FMK - moving disp to tab
    QHBoxLayout *dispAndTabLayout = new QHBoxLayout();
    dispAndTabLayout->addWidget(dispBox,1);
    dispAndTabLayout->addWidget(tabWidget,1);
    outputLayout->addLayout(dispAndTabLayout,0.2);
    */
    outputLayout->addWidget(dPlotBox,4);
    outputLayout->addWidget(hystBox,4);
    outputLayout->addWidget(tBox,2);


  //  outLay = new QGridLayout;
  //  outLay->addWidget(dispBox,0,0);
  //  outLay->addWidget(tabWidget,1,0);
  //  outLay->addWidget(hystBox,0,1,2,1);
  //  outLay->addWidget(tBox,2,0,1,2);

  // outLay->addLayout(buttonLay,3,0);

    progressbar = new QProgressBar(this);
    progressbar->setOrientation(Qt::Vertical);
    progressbar->setValue(1);
    progressbar->hide();
    mainLayout->addWidget(progressbar,1);
    connect( this, SIGNAL( signalProgress(int) ), progressbar, SLOT( setValue(int) ) );


    // add to main layout
    outBox->setLayout(outputLayout);
    mainLayout->addWidget(outBox,2);





    //connect(slider, SIGNAL(sliderPressed()),  this, SLOT(slider_sliderPressed()));
    //connect(slider, SIGNAL(sliderReleased()), this, SLOT(slider_sliderReleased()));
    connect(slider, SIGNAL(valueChanged(int)),this, SLOT(slider_valueChanged(int)));
}

// Create actions for File and Help menus
void MainWindow::createActions() {

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAction = new QAction(tr("&Open"), this);
    openAction->setShortcuts(QKeySequence::Open);
    openAction->setStatusTip(tr("Open an existing file"));
    connect(openAction, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction(openAction);

    QAction *saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcuts(QKeySequence::Save);
    saveAction->setStatusTip(tr("Save the document to disk"));
    connect(saveAction, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(saveAction);

    QAction *saveAsAction = new QAction(tr("&Save As"), this);
    saveAction->setStatusTip(tr("Save the document with new filename to disk"));
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAs);
    fileMenu->addAction(saveAsAction);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *infoAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    QAction *submitAct = helpMenu->addAction(tr("&Provide Feedback"), this, &MainWindow::submitFeedback);
    QAction *aboutAct = helpMenu->addAction(tr("&Version"), this, &MainWindow::version);
    QAction *citeAct = helpMenu->addAction(tr("&How to Cite"), this, &MainWindow::cite);
    QAction *copyrightAct = helpMenu->addAction(tr("&License"), this, &MainWindow::copyright);

}


// label functions
// name(QLabel) + enter(QComboBox) + units(QLabel)
QCheckBox *addCheck(QString text, QString unitText,
                    QGridLayout *gridLay,int row,int col, int nrow, int ncol)
{
    QHBoxLayout *Lay = new QHBoxLayout();
    QLabel *Label = new QLabel(text);
    QCheckBox *res = new QCheckBox();

    // props
    //res->setMinimumWidth(150);

    // layout
    Lay->addWidget(Label);
    Lay->addStretch(0);
    Lay->addWidget(res);

    // unit text
    if (!unitText.isEmpty()) {
        QLabel *unitLabel = new QLabel(unitText);
        Lay->addWidget(unitLabel);
        unitLabel->setMinimumWidth(30);
        //unitLabel->setMaximumWidth(30);
    }

    // main layout
    Lay->setSpacing(2);
    Lay->setMargin(0);
    gridLay->addLayout(Lay,row,col,nrow,ncol);

    return res;
}

// name(QLabel) + enter(QComboBox) + units(QLabel)
QComboBox *addCombo(QString text, QStringList items, QString *unitText,
                    QGridLayout *gridLay,int row,int col, int nrow, int ncol)
{
    QHBoxLayout *Lay = new QHBoxLayout();
    QLabel *Label = new QLabel(text);
    QComboBox *res = new QComboBox();

    // add labels
    for (int i = 0; i < items.size(); i++)
    {
        res->addItem(items.at(i));
    }

    // width
    QRect rec = QApplication::desktop()->screenGeometry();
    //int height = 0.7*rec.height();
    int width = 0.75*rec.width();
    res->setMaximumWidth(0.25*width/2);
    res->setMinimumWidth(0.2*width/2);


    // layout
    Lay->addWidget(Label);
    Lay->addStretch(0);
    Lay->addWidget(res);

    // unit text
    if (unitText != 0) {
        QLabel *unitLabel = new QLabel(*unitText);
        Lay->addWidget(unitLabel);
        unitLabel->setMinimumWidth(30);
        unitLabel->setMaximumWidth(30);
    }

    if (ncol>1)
        Lay->addStretch(0);

    // main layout
    Lay->setSpacing(2);
    Lay->setMargin(0);
    gridLay->addLayout(Lay,row,col,nrow,ncol);

    return res;
}

// name(QLabel) + enter(QDoubleSpinBox) + units(QLabel)
QDoubleSpinBox *addDoubleSpin(QString text,QString *unitText,
                QGridLayout *gridLay,int row,int col, int nrow, int ncol)
{
    QHBoxLayout *Lay = new QHBoxLayout();
    QLabel *Label = new QLabel(text);
    QDoubleSpinBox *res = new QDoubleSpinBox();

    // props
    res->setMinimum(0.);
    res->setKeyboardTracking(0);

    // width
    QRect rec = QApplication::desktop()->screenGeometry();
    //int height = 0.7*rec.height();
    int width = 0.25*rec.width();
    res->setMinimumWidth(0.25*width/2);

    // layout
    Lay->addWidget(Label);
    Lay->addStretch(0);
    Lay->addWidget(res);

    // unit text
    if (unitText != 0) {
        QLabel *unitLabel = new QLabel(*unitText);
        unitLabel->setMinimumWidth(30);
        unitLabel->setMaximumWidth(30);
        Lay->addWidget(unitLabel);
    }

    // main layout
    Lay->setSpacing(2);
    Lay->setMargin(0);
    gridLay->addLayout(Lay,row,col,nrow,ncol);

    return res;
}





QLineEdit *addLineEdit(QString labelText, QString text,
                QGridLayout *gridLay,int row,int col, int nrow, int ncol)
{
    QHBoxLayout *Lay = new QHBoxLayout();
    QLabel *Label = new QLabel(labelText);
    QLineEdit *res = new QLineEdit();



    // width
    QRect rec = QApplication::desktop()->screenGeometry();
    //int height = 0.7*rec.height();
    int width = 0.35*rec.width();
    res->setMinimumWidth(0.25*width/2);

    res->setText(text);

    // layout
    Lay->addWidget(Label);
    Lay->addStretch(0);
    Lay->addWidget(res);


    // main layout
    Lay->setSpacing(2);
    Lay->setMargin(0);
    gridLay->addLayout(Lay,row,col,nrow,ncol);

    return res;
}

// name(QLabel) + enter(QDoubleSpinBox) + units(QLabel)
QSpinBox *addSpin(QString text, QString *unitText,
                  QGridLayout *gridLay,int row,int col, int nrow, int ncol)
{
    QHBoxLayout *Lay = new QHBoxLayout();
    QLabel *Label = new QLabel(text);
    QSpinBox *res = new QSpinBox();

    // props
    res->setKeyboardTracking(0);

    // width
    QRect rec = QApplication::desktop()->screenGeometry();
    //int height = 0.7*rec.height();


    // layout
    Lay->addWidget(Label);
    Lay->addStretch(0);
    Lay->addWidget(res);

    // unit text
    if (unitText != 0) {
        QLabel *unitLabel = new QLabel(*unitText);
        unitLabel->setMinimumWidth(30);
        unitLabel->setMaximumWidth(30);
        Lay->addWidget(unitLabel);
    }

    // main layout
    Lay->setSpacing(2);
    Lay->setMargin(2);
    gridLay->addLayout(Lay,row,col,nrow,ncol);

    return res;
}

void MainWindow::replyFinished(QNetworkReply *pReply)
{
    return;
}

bool MainWindow::copyDir(const QDir& from, const QDir& to, bool cover=true)
{

    if (!to.exists())
        {
        if (!to.mkdir(to.absolutePath()))
            return false;
    } else {

    }

    QFileInfoList fileInfoList = from.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList)
    {
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if (fileInfo.isDir()){
            if (!copyDir(fileInfo.filePath(), to.filePath(fileInfo.fileName())))
                return false;
        }
        else{
            if (cover && to.exists(fileInfo.fileName())){
                //to.remove(fileInfo.fileName());
            }
            if (!QFile::copy(fileInfo.filePath(), to.filePath(fileInfo.fileName()))){
                return false;
            }
        }
    }
    return true;
}
