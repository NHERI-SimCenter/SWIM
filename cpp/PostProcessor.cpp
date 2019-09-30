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
    calcForce();
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
