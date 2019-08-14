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


#include "historywidget.h"
#include <qcustomplot/qcustomplot.h>

historyWidget::historyWidget(QString xLabel, QString yLabel, QWidget *parent)
              : QWidget(parent)
{   
    steps = 2;
    data = new QVector<double>(steps,0.);
    time = new QVector<double>(steps,0.);

    thePlot = new QCustomPlot();
    thePlot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    /*
    QRect rec = QApplication::desktop()->screenGeometry();
    int height = 0.7*rec.height();
    thePlot->setMinimumHeight(0.2*height);
    thePlot->setMaximumHeight(0.2*height);
    */
    thePlot->xAxis->setLabel(xLabel);
    thePlot->yAxis->setLabel(yLabel);

    // axes
    thePlot->xAxis2->setVisible(true);
    thePlot->yAxis2->setVisible(true);
    thePlot->xAxis->setTicks(false);
    thePlot->yAxis->setTicks(false);
    thePlot->xAxis->setSubTicks(false);
    thePlot->yAxis->setSubTicks(false);
    thePlot->xAxis2->setTickLabels(false);
    thePlot->yAxis2->setTickLabels(false);
    thePlot->xAxis2->setTicks(false);
    thePlot->yAxis2->setTicks(false);
    thePlot->xAxis2->setSubTicks(false);
    thePlot->yAxis2->setSubTicks(false);
    thePlot->axisRect()->setAutoMargins(QCP::msNone);
    thePlot->axisRect()->setMargins(QMargins(1,1,1,1));

    // aff to layout
    QGridLayout *Lay = new QGridLayout(this);
    Lay->addWidget(thePlot,0,0);
    Lay->setMargin(0);
    this->setLayout(Lay);
}

historyWidget::~historyWidget()
{
    // delete thePlot;
    //delete graph;

    delete data;
    delete time;
}

void historyWidget::setData(QVector<double> *inData, QVector<double> *inTime)
{
    // resize
    steps = inData->size();
    data->resize(steps);
    time->resize(steps);

    // set data
    for (int i=0; i<steps; i++) {
        (*data)[i] = (*inData)[i];
        (*time)[i] = (*inTime)[i];
    }

    // max
    maxVal = 0.;
    for (int j=0; j < steps; j++) {
        double val = (*data)[j];
        if (fabs(val) > maxVal)
            maxVal = fabs(val);
    }

    if (maxVal==0)
        maxVal = 1;

    plotModel();
}

void historyWidget::plotModel()
{
    // setup system plot
    thePlot->clearPlottables();
    thePlot->clearGraphs();
    //thePlot->clearItems();

    // add graph
    graph = thePlot->addGraph();

    // create pen
    QPen pen;
    pen.setWidthF(2);

    // line color
    pen.setColor(QColor(Qt::gray));
    thePlot->graph(0)->setPen(pen);
    thePlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 1));
    thePlot->graph(0)->setLineStyle(QCPGraph::lsNone);

    // set data
    thePlot->graph(0)->setData(*time,*data,true);

    // axes
    thePlot->xAxis->setRange(0,(*time)[steps-1]);
    thePlot->yAxis->setRange(-maxVal, maxVal);

    // tracer
    thePlot->addGraph();
    pen.setColor(QColor(Qt::red));
    thePlot->graph(1)->setPen(pen);
    thePlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 10));

    // trace
    //QVector<double> z = {0.};
    //thePlot->graph(1)->setData(z,z);

    // update plot
    thePlot->replot(QCustomPlot::rpQueuedReplot);
    thePlot->update();
}

void historyWidget::moveDot(double xi, double yi)
{
    // new coords
    QVector<double> x = {xi};
    QVector<double> y = {yi};

    // create graph
    plotModel();
    thePlot->graph(1)->setData(x,y,true);

    // plot
    thePlot->replot(QCustomPlot::rpQueuedReplot);
    thePlot->update();
}
