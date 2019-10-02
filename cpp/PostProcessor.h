#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include <QObject>
#include <QDialog>
#include <QTabWidget>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <math.h>
#include <QApplication>
#include <QStandardPaths>

class PostProcessor : public QDialog
{
    Q_OBJECT
public:
    explicit PostProcessor(QWidget *parent = nullptr);
    PostProcessor(QTabWidget *tab,QWidget *parent = nullptr);
    PostProcessor(QString outDir, int numSteps) : m_outputDir(outDir),m_numSteps(numSteps) {init();}
    int getLineCount();
    void update();
    void calcForce();
    void calcDeformx();
    void calcDeformy();
    void calcDeform();
    std::vector<double> getForce(){return force;}
    std::vector<std::vector<double>> getDispx(){return dispx;}
    std::vector<std::vector<double>> getDispy(){return dispy;}

    void init();


    int totalLines;

signals:
    void updateFinished();
private:
    QString rootDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);//qApp->applicationDirPath();
    QString analysisName = "analysis";
    QString analysisDir = QDir(rootDir).filePath(analysisName);


    QString m_outputDir;
    int m_numSteps;
    // opensees results
    QString forceFileName = QDir(m_outputDir).filePath("resultsOpenSees.out");
    QString xFileName = QDir(m_outputDir).filePath("dispx.out");
    QString yFileName = QDir(m_outputDir).filePath("dispy.out");
    QString xyFileName = QDir(m_outputDir).filePath("disp.out");

    std::vector<double> force;
    std::vector<std::vector<double>> dispx;
    std::vector<std::vector<double>> dispy;
    std::vector<std::vector<double>> dispxy;

};

#endif // POSTPROCESSOR_H
