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

#include "hysteresiswidget.h"
#include <qcustomplot/qcustomplot.h>
#include "resp.h"
#include <QLabel>

hysteresisWidget::hysteresisWidget(QString xLabel, QString yLabel, QWidget *parent)
                    : QWidget(parent)
{
    // initialize
    size = 2;
    xi = new QVector<double>(size,0.);
    yi = new QVector<double>(size,0.);
    xj = new QVector<double>(size,0.);
    yj = new QVector<double>(size,0.);

    // setup plot
    thePlot = new QCustomPlot();
    thePlot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    QRect rec = QApplication::desktop()->screenGeometry();
    //int height = 0.7*rec.height();
    int width = 0.7*rec.width();
    //thePlot->setMinimumHeight(0.35*width);
    //thePlot->setMaximumHeight(0.3*width);
    // thePlot->setMinimumWidth(0.35*width);
    //thePlot->setMaximumWidth(0.3*width);
    thePlot->xAxis->setLabel(xLabel);
    thePlot->yAxis->setLabel(yLabel);

    // axes
    thePlot->xAxis2->setVisible(true);
    thePlot->yAxis2->setVisible(true);
    thePlot->xAxis->setTicks(true);
    thePlot->yAxis->setTicks(true);
    thePlot->xAxis->setSubTicks(false);
    thePlot->yAxis->setSubTicks(false);
    thePlot->xAxis2->setTickLabels(false);
    thePlot->yAxis2->setTickLabels(false);
    thePlot->xAxis2->setTicks(false);
    thePlot->yAxis2->setTicks(false);
    thePlot->xAxis2->setSubTicks(false);
    thePlot->yAxis2->setSubTicks(false);
    //thePlot->axisRect()->setAutoMargins(QCP::msNone);
    //thePlot->axisRect()->setMargins(QMargins(1,1,1,1));

    // print current value...
    label1 = new QLabel;
    label2 = new QLabel;
    label1->setAlignment(Qt::AlignLeft);
    label2->setAlignment(Qt::AlignLeft);

    // add to layout
    QVBoxLayout *Lay = new QVBoxLayout(this);
    Lay->addWidget(thePlot,1);
    QHBoxLayout *textLayout = new QHBoxLayout();
    textLayout->addWidget(label1);
    textLayout->addWidget(label2);
   // Lay->addWidget(label1,1,0);
   // Lay->addWidget(label2,1,1);
    Lay->addLayout(textLayout);
    Lay->setMargin(0);
    this->setLayout(Lay);
}

hysteresisWidget::~hysteresisWidget()
{
    //  delete thePlot;
    // delete graph;
    //delete curve1;
    //delete curve2;
    delete xi;
    delete xj;
    delete yi;
    delete yj;
}

void hysteresisWidget::setModel(QVector<double> *data_x, QVector<double> *data_y)
{
    size = int(data_x->size());

    // initialize
    xi->resize(size);
    yi->resize(size);
    xj->resize(size);
    yj->resize(size);

    // set
    for (int i=0; i<size; i++) {
        (*xi)[i] = (*data_x)[i];
        (*yi)[i] = (*data_y)[i];
    }

    // max -X
    maxX = 0.;
    for (int j=0; j < size; j++) {
        double val = (*xi)[j];
        if (fabs(val) > maxX)
            maxX = fabs(val);
    }

    // max -Y
    maxY = 0.;
    for (int j=0; j < size; j++) {
        double val = (*yi)[j];
        if (fabs(val) > maxY)
            maxY = fabs(val);
    }
}

void hysteresisWidget::setResp(QVector<double> *data_p, QVector<double> *data_q)
{
    size = data_p->size();
    responseDataZero = true;

    // re-size
    xj->resize(size);
    yj->resize(size);

    // store
    for (int j=0; j < size; j++) {
        (*xj)[j] = (*data_p)[j];
        (*yj)[j] = (*data_q)[j];
    }

    // max -X
    for (int j=0; j < size; j++) {
        double val = (*xj)[j];
        if (val != 0) responseDataZero = false;
        if (fabs(val) > maxX)
            maxX = fabs(val);
    }

    // max -Y
    for (int j=0; j < size; j++) {
        double val = (*yj)[j];
        if (val != 0) responseDataZero = false;
        if (fabs(val) > maxY)
            maxY = fabs(val);
    }
}

void hysteresisWidget::plotModel()
{
    thePlot->clearPlottables();
    //thePlot->clearGraphs();
    //thePlot->clearItems();
    thePlot->legend->setVisible(true);
    
    // create curves
    curve1 = new QCPCurve(thePlot->xAxis, thePlot->yAxis);
    curve2 = new QCPCurve(thePlot->xAxis, thePlot->yAxis);

    curve1->setName(tr("Experimental"));
    if (responseDataZero == false)
      curve2->setName(tr("Simulation"));
    else
      curve2->setName(tr("No Simulation resuts PRESS \"Analyze\""));

    // create pen
    QPen pen;
    pen.setWidthF(3);
    pen.setColor(QColor(Qt::gray));
    curve1->setPen(pen);
    pen.setColor(QColor(Qt::blue));
    curve2->setPen(pen);

    // set data
    // curve1->setData(*xi,*yi);

    // axes
    thePlot->xAxis->setRange(-maxX-1,maxX+1);
    thePlot->yAxis->setRange(-maxY-10,maxY+10);

    // update plot
    thePlot->replot(QCustomPlot::rpQueuedReplot);
    //thePlot->update();

    // update label
    //label->setText(QString("current = %1 in.").arg(maxY,0,'f',2));
}

void hysteresisWidget::plotResponse(int t)
{
    // re-plot baseline
    plotModel();

    // set data
    curve1->setData(xi->mid(0,t),yi->mid(0,t));    
    curve2->setData(xj->mid(0,t),yj->mid(0,t));

    // update plot
    thePlot->replot(QCustomPlot::rpQueuedReplot);
    //thePlot->update();

    // update label
    QString text1;
    text1.sprintf("Experiment: %+6.2f in %+8.2f kips",(*xi)[t],(*yi)[t]);
    QString text2;
    text2.sprintf("Simulation: %+6.2f in %+8.2f kips",(*xj)[t],(*yj)[t]);
    // text1,sprintf("Experiment %4.2f",(*xi)[t]);
    label1->setText(text1);
    label2->setText(text2);
    // label1->setText(QString("Experiment: (" + QString::number((*xi)[t], 'f', 2)));
    // label1->setText(QString("Experiment: (%1 in., %2 kips)").arg((*xi)[t],0,'f',1).arg((*yi)[t],0,'f',0));
    // label2->setText(QString("Simulation: (%1 in., %2 kips)").arg((*xj)[t],0,'f',1).arg((*yj)[t],0,'f',0));
}
