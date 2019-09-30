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
    PostProcessor(QString outDir) : m_outputDir(outDir){init();}
    int getLineCount();
    void update();
    void calcForce();
    std::vector<double> getForce(){return force;}

    void init();


    int totalLines;

signals:
    void updateFinished();
private:
    QString rootDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);//qApp->applicationDirPath();
    QString analysisName = "analysis";
    QString analysisDir = QDir(rootDir).filePath(analysisName);


    QString m_outputDir;
    // opensees results
    QString forceFileName = QDir(m_outputDir).filePath("resultsOpenSees.out");

    std::vector<double> force;

};

#endif // POSTPROCESSOR_H
