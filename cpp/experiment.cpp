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


#include <cmath>
#include "experiment.h"
#include <Vector.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>
#include <QString>

double interpolate(QVector<double> &xData, QVector<double> &yData, double x, bool extrapolate);

Experiment::Experiment()
  : testType(QStringLiteral("None"))
{
    numSteps = 1;
    dataD = new QVector<double>(numSteps,0.);
    dataP = new QVector<double>(numSteps,0.);
    time = new QVector<double>(numSteps,0.);
    dt = 0.1;
}

Experiment::~Experiment()
{
    if (dataD != NULL)
        delete dataD;
    if (dataP != NULL)
        delete dataP;
    if (time != NULL)
        delete time;
}

QString Experiment::getTestType() const {
  return testType;
} 

QVector<double>* Experiment::getDataP(void)
{
    dataP->prepend(0.);
    return dataP;
}

QVector<double>* Experiment::getDataD(void)
{
    dataD->prepend(0.);
    return dataD;
}

QVector<double>* Experiment::getTime(void)
{
    time->prepend(0.);
    return time;
}

double Experiment::getdt(void)
{
    return dt;
}

int Experiment::getNumSteps(void)
{
    return numSteps+1;
}

int Experiment::inputFromJSON(QJsonValue &json)
{
    int ok = 0;
    QJsonObject theData = json.toObject();
    
    // load experiment data
    if (json.isNull() || json.isUndefined()) {
        ok = -1;

    // This loads experiment data that includes time steps, so there is no need to interpolate
    } else if (!theData["timeSteps"].isNull() && !theData["timeSteps"].isUndefined()) {
	
	QJsonArray timeData = theData["timeSteps"].toArray();
	time->resize(timeData.size());

	for (int i = 0; i < time->size(); ++i) {
	  (*time)[i] = timeData[i].toDouble();
	}

	if (theData["type"].isNull() || theData["axialDef"].isUndefined()) {
	  ok = -5;
	} else {
	  testType = theData["type"].toString();	  
	}	

        // axial def
        if (theData["axialDef"].isNull() || theData["axialDef"].isUndefined()) {
            ok = -2;

        } else {
            QJsonArray theArray=theData["axialDef"].toArray();
            numSteps = theArray.size();

            dataD->resize(numSteps);
            for (int j=0; j<numSteps; j++)
                (*dataD)[j] = theArray.at(j).toDouble();
        }

        // axial force
        if (theData["axialForce"].isNull() || theData["axialForce"].isUndefined()) {

            dataP->resize(numSteps);
            for (int j=0; j<numSteps; j++)
                (*dataP)[j] = 0.;

            ok =  -3;

        } else {
            QJsonArray theArray=theData["axialForce"].toArray();
            if (numSteps != theArray.size()) {
                numSteps = std::min(numSteps,theArray.size());

                ok = -4;
            }

            dataP->resize(numSteps);
            for (int j=0; j<numSteps; j++)
                (*dataP)[j] = theArray.at(j).toDouble();
        }
	
    } else {

        QJsonObject theData = json.toObject();

	if (theData["type"].isNull() || theData["axialDef"].isUndefined()) {
	  ok = -5;
	} else {
	  testType = theData["type"].toString();	  
	}	

        // axial def
        if (theData["axialDef"].isNull() || theData["axialDef"].isUndefined()) {
            ok = -2;

        } else {
            QJsonArray theArray=theData["axialDef"].toArray();
            numSteps = theArray.size();

            dataD->resize(numSteps);
            for (int j=0; j<numSteps; j++)
                (*dataD)[j] = theArray.at(j).toDouble();
        }

        // axial force
        if (theData["axialForce"].isNull() || theData["axialForce"].isUndefined()) {

            dataP->resize(numSteps);
            for (int j=0; j<numSteps; j++)
                (*dataP)[j] = 0.;

            ok =  -3;

        } else {
            QJsonArray theArray=theData["axialForce"].toArray();
            if (numSteps != theArray.size()) {
                numSteps = std::min(numSteps,theArray.size());

                ok = -4;
            }

            dataP->resize(numSteps);
            for (int j=0; j<numSteps; j++)
                (*dataP)[j] = theArray.at(j).toDouble();
        }
        interpData();
    }

    return ok;
}

void Experiment::setTime(QVector<double> *data)
{   
    time->resize(numSteps);
    (*time)[0] = 0.;

    for (int j=1; j < numSteps; j++) {
        (*time)[j] = (*time)[j-1] + fabs((*data)[j] - (*data)[j-1]);
    }
}

void Experiment::interpData()
{
    // initialize
    setTime(dataD);
    int nstep = ceil((*time)[numSteps-1]/dt);
    QVector<double> *tmpP = new QVector<double>(nstep,0.);
    QVector<double> *tmpD = new QVector<double>(nstep,0.);

    // interpolate
    for (int t=0; t<nstep; t++) {
        (*tmpP)[t] = interpolate(*time,*dataP,t*dt,true);
        (*tmpD)[t] = interpolate(*time,*dataD,t*dt,true);
    }

    // re-size
    numSteps = nstep;
    dataD->resize(numSteps);
    dataP->resize(numSteps);
    dataD = tmpD;
    dataP = tmpP;

    // re-time
    time->resize(numSteps);
    (*time)[0] = 0.;
    for (int j=1; j < numSteps; j++)
        (*time)[j] = (*time)[j-1] + dt;
}
/*
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
*/
