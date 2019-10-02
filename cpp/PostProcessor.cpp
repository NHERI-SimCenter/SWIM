#ifdef _MSC_VER
    #define _USE_MATH_DEFINES
    #include <cmath>
#endif

#include "PostProcessor.h"

PostProcessor::PostProcessor(QWidget *parent) : QDialog(parent)
{

}

PostProcessor::PostProcessor(QTabWidget *tab,QWidget *parent) : QDialog(parent)
{

}

int PostProcessor::getLineCount()
{
//    /QFile file("out_tcl/elementInfo.dat");
    QFile file(forceFileName);
    int lineCount = 0;
    file.open(QIODevice::ReadOnly); //| QIODevice::Text)
    QTextStream in(&file);
    while( !in.atEnd())
    {
        in.readLine();
        lineCount++;
    }
    return lineCount;
}

void PostProcessor::init()
{
    totalLines = getLineCount();
    if(totalLines==m_numSteps)
    {
        calcForce();
        calcDeform();
    }
}


void PostProcessor::update()
{
    totalLines = getLineCount();
    calcForce();

    emit updateFinished();
}



void PostProcessor::calcForce()
{
    QFile forceFile(forceFileName);
    force.clear();
    double aLine;
    int numofNodes = 1;

    if(forceFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&forceFile);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            thisLine.removeAll("");
            if (thisLine.size()<numofNodes)
                break;
            else
            {
                numofNodes = thisLine.size();
                aLine = 0.0;
                for (int i=0; i<thisLine.size(); i++)
                {
                    aLine += thisLine[i].trimmed().toDouble();
                }
                force.push_back(aLine);
            }
        }
        forceFile.close();
    }
}

void PostProcessor::calcDeform()
{
    calcDeformx();
    calcDeformy();
}
void PostProcessor::calcDeformx()
{
    QFile xFile(xFileName);
    dispx.clear();
    std::vector<double> aLine;
    int numofNodes = 1;

    if(xFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&xFile);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            thisLine.removeAll("");
            if (thisLine.size()<numofNodes)
                break;
            else
            {
                numofNodes = thisLine.size();
                aLine.clear();
                for (int i=0; i<thisLine.size(); i++)
                {
                    aLine.push_back(thisLine[i].trimmed().toDouble());
                }
                dispx.push_back(aLine);
            }
        }
        xFile.close();
    }
}
void PostProcessor::calcDeformy()
{
    QFile yFile(yFileName);
    dispy.clear();
    std::vector<double> aLine;
    int numofNodes = 1;

    if(yFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&yFile);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList thisLine = line.split(" ");
            thisLine.removeAll("");
            if (thisLine.size()<numofNodes)
                break;
            else
            {
                numofNodes = thisLine.size();
                aLine.clear();
                for (int i=0; i<thisLine.size(); i++)
                {
                    aLine.push_back(thisLine[i].trimmed().toDouble());
                }
                dispy.push_back(aLine);
            }
        }
        yFile.close();
    }
}
