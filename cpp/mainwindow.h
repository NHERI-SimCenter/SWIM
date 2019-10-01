#ifndef MAINWINDOW_H
#define MAINWINDOW_H


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


#include <QMainWindow>
#include <QStandardItemModel>
#include <math.h>
#include <map>

#include <QtGui>
#include <QtWidgets>
#include <QtCore>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <qcustomplot/qcustomplot.h>

#include <Vector.h>

#include <HeaderWidget.h>
#include <FooterWidget.h>
#include "ai.h"
#include "ConcreteShearWall.h"
#include "OpenSeesTclBuilder.h"
#include "experimentWall.h"
#include "PostProcessor.h"


class Experiment;
class Resp;
class historyWidget;
class deformWidget;
class responseWidget;
class hysteresisWidget;
class QPushButton;

// organization of sxnShape
enum class sxnShape {
    W,
    M,
    S,
    HP,
    C,
    MC,
    L,
    dL,
    WT,
    MT,
    ST,
    HSS,
    RND,
    PIPE
};

// struct for materials
struct steel {
    double fy;
    double Es;
    // kinematic hardening - tension
    double bk;
    double R0k;
    double r1;
    double r2;
    // kinematic hardening - compression
    double bkc;
    double R0kc;
    double r1c;
    double r2c;
    // isotropic hardening - tension
    double bi;
    double rhoi;
    double bl;
    double Ri;
    double lyp;
    // isotropic hardening - compression
    double bic;
    double rhoic;
    double blc;
    double Ric;
    // asymmetric inputs
    double a1;
    double a2;
    double a3;
    double a4;
};

// fatigue structure
struct fat {
    double m;
    double e0;
    double emin;
    double emax;
};

// section struct
struct section {
    double A;
    double Ix;
    double Zx;
    double Sx;
    double rx;
    double Iy;
    double Zy;
    double Sy;
    double ry;
    double I;
    double Z;
    double S;
    double r;
    // sxn geom
    double d;
    double bf;
    double tw;
    double tf;
    // combactness
    double bftf;
    double htw;
};

// connection struct
struct connection { // see Chambers, J. J., & Bartley, T. C. (2009). Erratum: Geometric formulas for gusset plate design ((2007), (258)). Engineering Journal.
    double fy;   // gusset material
    double Es;
    double tg;   // gusset plate thickness
    double H;    // total gusset height
    double W;    // total gusset width
    double lb;   // gusset length along beam
    double lc;   // gusset length along column
    double lbr;  // gusset length along brace
    double eb;   // depth of column
    double ec;   // depth of beam
    double L;    // length of gusset
    double rigA; // length of gusset
    double rigI; // length of gusset
};

// size
struct windowSize {
    int width;
    int height;
};

namespace Ui {
class MainWindow;
}

class QNetworkAccessManager;
class QNetworkReply;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    // analysis
    void buildModel();

    // Actions for open and saving files
    void open();
    bool save();
    bool saveAs();
    
    // Options for Help menu
    void about();
    void submitFeedback();
    void version();
    void copyright();
    void cite();

    // createSAM  // adding wall
    void createSAM();
    void createSAMui();
    void createBIMui();
    void preprocess();


    // custom plots
    //void dPlot(QCustomPlot *plot, int Fig);

private slots:
    // pushButton
    void addExp_clicked();
    void addAISC_clicked();
    void theAISC_sectionClicked(int row);
    void reset();
    void doAnalysis();
    void doWallAnalysis();
    //void stop_clicked();
    void play_clicked();
    void pause_clicked();
    void restart_clicked();
    void exit_clicked();

    //connect(play,SIGNAL(clicked()), this, SLOT(play_clicked()));
    //connect(stop,SIGNAL(clicked()), this, SLOT(stop_clicked()));
    //connect(reset,SIGNAL(clicked()), this, SLOT(reset_clicked()));

    // adding wall
    void nofEdt_valueChanged(int nof);
    void idFloorEdt_valueChanged_BIM(int idFloor);
    void idFloorEdt_valueChanged_SAM(int idFloor);
    void matSelectorBIM_valueChanged_BIM(int matID);
    void rcSelector_valueChanged_SAM(QString matID);
    void concreteSelector_valueChanged_SAM(QString matID);
    void rebarSelector_valueChanged_SAM(QString matID);
    void steelSelector_valueChanged_SAM(QString matID);
    void getSAM();
    void rc_valueChanged_SAM(double);
    void rc_valueChanged_SAM(QString);
    void rc_valueChanged_SAM();
    void concrete_valueChanged_SAM(double);
    void beta_valueChanged_SAM(double);
    void Bn_valueChanged_SAM(double);
    void Ap_valueChanged_SAM(double);
    void An_valueChanged_SAM(double);
    void rebar_valueChanged_SAM(double);
    void rebar_valueChanged_SAM(QString);
    void rebar_valueChanged_SAM();
    void steel_valueChanged_SAM(double);
    void showSteel();
    void showRebar();
    void showSteel(int steelID);
    void webRCselector_valueChanged_SAM(QString);
    void beRCselector_valueChanged_SAM(QString);
    void assemble_valueChanged_SAM();
    std::vector<float> getAIinputs();
    void ESize_valueChanged_SAM(double);
    void webESize_valueChanged_SAM(double);
    void beESize_valueChanged_SAM(double);
    void updateSAMFile();
    void doWallAnalysisOpenSees();
    void onOpenSeesFinished();
    void loadNew();
    void loadNewBtn_clicked();
    void deletePanels();
    void openseespathEdt_textChanged(QString);
    bool copyDir(const QDir& from, const QDir& to, bool cover);
    void AIbtn_clicked();


    // Combo Box
    void inSxn_currentIndexChanged(int row);
    void inOrient_currentIndexChanged(int row);
    void inElType_currentIndexChanged(int row);
    void inElDist_currentIndexChanged(int row);
    void inIM_currentIndexChanged(int row);
    void inShape_currentIndexChanged(int row);
    void inMat_currentIndexChanged(int row);
    void in_conn1_currentIndexChanged(int row);
    void in_conn2_currentIndexChanged(int row);
    void inExp_currentIndexChanged(int row);

    // Spin box
    void inNe_valueChanged(int var);
    void inNIP_valueChanged(int var);
    void inNbf_valueChanged(int var);
    void inNtf_valueChanged(int var);
    void inNd_valueChanged(int var);
    void inNtw_valueChanged(int var);

    // double spin box
    void inLwp_valueChanged(double var);
    void inL_valueChanged(double var);
    void inDelta_valueChanged(double var);
    void inEs_valueChanged(double var);
    void infy_valueChanged(double var);
    //
    void inb_valueChanged(double var);
    //
    void ina1_valueChanged(double var);
    void ina2_valueChanged(double var);
    void ina3_valueChanged(double var);
    void ina4_valueChanged(double var);
    //
    void inR0_valueChanged(double var);
    void inR1_valueChanged(double var);
    void inR2_valueChanged(double var);
    //
    void inbk_valueChanged(double var);
    void inR0k_valueChanged(double var);
    void inr1_valueChanged(double var);
    void inr2_valueChanged(double var);
    void inbkc_valueChanged(double var);
    void inR0kc_valueChanged(double var);
    void inr1c_valueChanged(double var);
    void inr2c_valueChanged(double var);
    //
    void inbi_valueChanged(double var);
    void inrhoi_valueChanged(double var);
    void inbl_valueChanged(double var);
    void inRi_valueChanged(double var);
    void inlyp_valueChanged(double var);
    void inbic_valueChanged(double var);
    void inrhoic_valueChanged(double var);
    void inblc_valueChanged(double var);
    void inRic_valueChanged(double var);
    //
    void inm_valueChanged(double var);
    void ine0_valueChanged(double var);
    void inemin_valueChanged(double var);
    void inemax_valueChanged(double var);
    //
    void inl_conn1_valueChanged(double var);
    void inRigA_conn1_valueChanged(double var);
    void inRigI_conn1_valueChanged(double var);
    //
    void inl_conn2_valueChanged(double var);
    void inRigA_conn2_valueChanged(double var);
    void inRigI_conn2_valueChanged(double var);

    // check box
    //void includePDeltaChanged(int);
    void matDefault_checked(int);
    void matAsymm_checked(int);
    void matFat_checked(int);
    void connSymm_checked(int);

    // slider
    void slider_valueChanged(int value);
    //void slider_sliderPressed();
    //void slider_sliderReleased();

    void replyFinished(QNetworkReply*);

private:
    Ui::MainWindow *ui;

    // main layouts
    void createHeaderBox();
    void createFooterBox();
    void createInputPanel();
    void createOutputPanel();
    void createActions();    

    // load information
    void loadAISC();
    void setExp(ExperimentWall *exp);
    void setExp(Experiment *exp);
    void initialize();
    void zeroResponse();

    // plot
    void repaint();

    // methods for loading and saving files given filename
    bool saveFile(const QString &fileName);
    void loadFile(const QString &fileName);

    void loadExperimentalFile(const QString &fileName);
    void loadWallExperimentalFile(const QString &fileName);

    // initialize

    // created main layouts
    QHBoxLayout *mainLayout;
    QVBoxLayout *largeLayout;
    //QHBoxLayout *headerLayout;
    //QHBoxLayout *footerLayout;
    QGridLayout *outLay;
    QVBoxLayout *inLay;

    // Experiment image
    QLabel * experimentImage;

    // Button
    QPushButton *addExp;
    QPushButton *addAISC;

    // Combo Box
    QComboBox *inExp;
    QComboBox *inSxn;
    QComboBox *inElType;
    QComboBox *inElDist;
    QComboBox *inIM;
    QComboBox *inShape;
    QComboBox *inOrient;
    QComboBox *inMat;
    QComboBox *in_conn1;
    QComboBox *in_conn2;

    // spin box    
    QSpinBox *inNe;
    QSpinBox *inNIP;
    QSpinBox *inNbf;
    QSpinBox *inNtf;
    QSpinBox *inNd;
    QSpinBox *inNtw;

    // double spin box
    QDoubleSpinBox *inLwp;
    QDoubleSpinBox *inL;
    QDoubleSpinBox *inDelta;
    //
    QDoubleSpinBox *inEs;
    QDoubleSpinBox *infy;
    //
    QDoubleSpinBox *inb;
    //
    QDoubleSpinBox *ina1;
    QDoubleSpinBox *ina2;
    QDoubleSpinBox *ina3;
    QDoubleSpinBox *ina4;
    //
    QDoubleSpinBox *inR0;
    QDoubleSpinBox *inR1;
    QDoubleSpinBox *inR2;
    //
    QDoubleSpinBox *inbk;
    QDoubleSpinBox *inR0k;
    QDoubleSpinBox *inr1;
    QDoubleSpinBox *inr2;
    QDoubleSpinBox *inbkc;
    QDoubleSpinBox *inR0kc;
    QDoubleSpinBox *inr1c;
    QDoubleSpinBox *inr2c;
    //
    QDoubleSpinBox *inbi;
    QDoubleSpinBox *inrhoi;
    QDoubleSpinBox *inbl;
    QDoubleSpinBox *inRi;
    QDoubleSpinBox *inlyp;
    QDoubleSpinBox *inbic;
    QDoubleSpinBox *inrhoic;
    QDoubleSpinBox *inblc;
    QDoubleSpinBox *inRic;
    //
    QDoubleSpinBox *inm;
    QDoubleSpinBox *ine0;
    QDoubleSpinBox *inemin;
    QDoubleSpinBox *inemax;
    // connection-1
    //QDoubleSpinBox *infy_conn1;
    //QDoubleSpinBox *inEs_conn1;
    //QDoubleSpinBox *inb_conn1;
    //QDoubleSpinBox *intg_conn1;
    QDoubleSpinBox *inl_conn1;
    //QDoubleSpinBox *inlw_conn1;
    QDoubleSpinBox *inRigA_conn1;
    QDoubleSpinBox *inRigI_conn1;
    // connection-2
    //QDoubleSpinBox *infy_conn2;
    //QDoubleSpinBox *inEs_conn2;
    //QDoubleSpinBox *inb_conn2;
    //QDoubleSpinBox *intg_conn2;
    QDoubleSpinBox *inl_conn2;
    //QDoubleSpinBox *inlw_conn2;
    QDoubleSpinBox *inRigA_conn2;
    QDoubleSpinBox *inRigI_conn2;

    // check box
    QCheckBox *matDefault;
    QCheckBox *matAsymm;
    QCheckBox *matFat;
    QCheckBox *connSymm;

    // group box
    QGroupBox *bBox;
    QGroupBox *steel01Box;
    QGroupBox *steel02Box;
    QGroupBox *kinBox;
    QGroupBox *isoBox;
    QGroupBox *cKinBox;
    QGroupBox *cIsoBox;
    QGroupBox *fatBox;

    // frame
    QFrame *steel4Frame;

    // labels
    QLabel *deltaL;
    QLabel *Alabel;
    QLabel *Ilabel;
    QLabel *Zlabel;
    QLabel *Slabel;
    QLabel *rlabel;
    //QLabel *tlabel;
    QLabel *dlabel;
    QLabel *bflabel;
    QLabel *twlabel;
    QLabel *tflabel;

    // QList
    QList<QStandardItem *> sxnProp; // huh?? want to grab row of QStandardItemModel
    // AISC lists
    QStandardItemModel *AISCshapes;
    QStringList propList;
    QStringList sxnList;

    // QStrings
    QString sxn;
    QString orient;
    QString elType;
    QString elDist;
    QString IM;
    QString shape;
    QString mat;

    // enums
    sxnShape sxnType;

    // QSlider
    QSlider *slider;

    // ints
    int ne; // # of elements
    int nn; // # of nodes
    int NIP; // # of integration points
    int nbf; // # of fibers across flange width
    int ntf; // # of fibers across flange thickness
    int nd; // # of fibers across depth
    int ntw; // # of fibers across web depth
    int stepCurr;
    int type_conn1;
    int type_conn2;

    // doubles
    // constants
    double pi;
    // geom
    double Lwp;
    double L;
    double delta;
    double angle;
    // mat
    steel theSteel;
    fat theFat;
    // sxn prop
    section theSxn;
    // connection
    connection conn1;
    connection conn2;

    // bools
    bool inclDefault;
    bool inclAsymm;
    bool inclFat;
    bool inclConnSymm;
    //bool movSlider;
    bool pause;
    bool stop;
    QPushButton *playButton;

    // coordinates
    QVector<double> xc;
    QVector<double> yc;

    // experiment
    int numSteps;
    QVector<double> *expD;
    QVector<double> *expP;
    QVector<double> *time;
    QString experimentType;
    double dt;

    // response
    Resp *Ux;
    Resp *Uy;
    Resp *q1;
    Resp *q2;
    Resp *q3;

    // output widgets
    deformWidget *dPlot;
    historyWidget *tPlot;
    responseWidget *pPlot;
    responseWidget *mPlot;
    responseWidget *kPlot;
    hysteresisWidget *hPlot;

    // Name of current file
    QString currentFile;
    double braceHeight;
    double braceWidth;

    QNetworkAccessManager *manager;

    // adding wall AI
    QString rootDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation); // qApp->applicationDirPath();//
    bool tmpVar1 = QDir(rootDir).exists() ? false : QDir().mkdir(rootDir);
    QString modelPath = QDir(rootDir).filePath("model.pt");
    bool tmpVar2 = QFile::copy(":/MyResources/model.pt", modelPath); // TODO: check is succeed
    AI ai = AI(modelPath.toStdString());

    // adding wall
    ConcreteShearWall *theWall;
    QGridLayout *wallConfigLay;
    QGridLayout *wallSAMLay;
    QGridLayout *wallBIMLay;
    QVector<QGroupBox*> floorSAMs;
    QVector<QGroupBox*> floorBIMs;
    QVector<QGroupBox*> steelBoxSAMs;
    QMap<int, QGroupBox*> rebarBoxSAMs;
    QMap<int, QGroupBox*> concreteBoxSAMs;
    QMap<int, QGroupBox*> rcBoxSAMs;
    QVector<QVector<QGroupBox*>> matBIMs;
    QString expDirName = "";
    QString openseespath = "";

    QDir expDir = QDir(expDirName);
    QVector<QComboBox*> matSelectorBIM;
    int numMatsBIM=0;
    int numFloors=0;
    int currentFloorIDbim = 0;
    double wallLength ;
    double beLength ;
    double webLength ;

    OpenSeesTclBuilder *thePreprocessor = new OpenSeesTclBuilder();

    QJsonObject samRoot;

    QJsonObject SAM ;
    QJsonObject geometry;
    QJsonObject properties;
    QJsonObject nodeMapping;
    QJsonArray uniaxialMaterials;
    QJsonArray ndMaterials;

    QMap<int, QJsonObject> rebarMaterials;
    QMap<int, QJsonObject> concreteMaterials;
    QMap<int, QJsonObject> rcMaterials;

    QMap<int, QDoubleSpinBox*> rct1Edt;
    QMap<int, QDoubleSpinBox*> rct2Edt;
    QMap<int, QDoubleSpinBox*> rct3Edt;
    QMap<int, QComboBox*> rcMat1Edt;
    QMap<int, QComboBox*> rcMat2Edt;
    QMap<int, QComboBox*> rcMat3Edt;

    QMap<int, QDoubleSpinBox*> concreteEEdt;
    QMap<int, QDoubleSpinBox*> concretefpcEdt;
    QMap<int, QDoubleSpinBox*> concretenuEdt;
    QMap<int, QDoubleSpinBox*> concretebetaEdt;
    QMap<int, QDoubleSpinBox*> concreteApEdt;
    QMap<int, QDoubleSpinBox*> concreteAnEdt;
    QMap<int, QDoubleSpinBox*> concreteBnEdt;

    QMap<int, QComboBox*> rebarSteelMatEdt;
    QMap<int, QDoubleSpinBox *> rebarAngEdt;
    //QMap<int, QSpinBox *> rebarIdEdt;

    QComboBox *matIDselector_rc ;
    QComboBox *matIDselector_concrete ;
    QComboBox *matIDselector_rebar ;// index is rebarID
    QComboBox *matIDselector_steel ;// index is steelID

    QStringList matIDList_rc;
    QStringList matIDList_concrete;
    QStringList matIDList_rebar;
    QStringList matIDList_steel;

    QStringList rcMatStrList;

    QStringList steelModelTypes = {"Steel01"};

    QMap<int, QStringList> matIDList_rc_mat1;
    QMap<int, QStringList> matIDList_rc_mat2;
    QMap<int, QStringList> matIDList_rc_mat3;


    QVector<QComboBox *> steelTyeEdt;
    QVector<QDoubleSpinBox *> steelEEdt;
    QVector<QDoubleSpinBox *> steelfyEdt;
    QVector<QDoubleSpinBox *> steelbEdt;

    QVector<QComboBox *> webRCselector;
    QVector<QComboBox *> beRCselector;


    QSpinBox *idFloorEdt_BIM ;
    QSpinBox *idFloorEdt_SAM ;

    QGridLayout *webLay;

    QDoubleSpinBox *eleSizeWebEdt;
    QDoubleSpinBox *eleSizeBEEdt;

    QProgressBar * progressbar;

    QProcess* openseesProcess;

    int openseesErrCount = 0;

    ExperimentWall *expWall = new ExperimentWall();

    std::vector<std::vector<double>> dispx ;
    std::vector<std::vector<double>> dispy ;

    QGroupBox *inBox;

    QLineEdit *openseespathEdt;

    bool hasResult = false;

    QStringList expNamesList={"wallDemo1", "wallDemo2"};
    QStringList expDirList={QDir(rootDir).filePath("wallDemo1"),QDir(rootDir).filePath("wallDemo2")};
    int currentExpInd = 0;

    int wwidth, wheight;


    double beta = 0.5;
    double An = 0.5;
    double Ap = 0.5;
    double Bn = 0.5;

    QPushButton *AIbtn;

    double betaAI = 0.5;
    double AnAI = 0.5;
    double ApAI = 0.5;
    double BnAI = 0.5;
    double nLAI = 2;

    int stepOpenSees = 0;



signals:
    void signalProgress(int);

};

#endif // MAINWINDOW_H
