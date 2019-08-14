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

#include "responsewidget.h"
#include <qcustomplot/qcustomplot.h>
#include "resp.h"
#include <QLabel>

responseWidget::responseWidget(QString xLabel, QString yLabel, QWidget *parent)
              : QWidget(parent)
{
    // initialize
    steps = 2;
    els = 4;
    size = els+3;

    xi = new QVector<double>(size,0.);
    yi = new QVector<double>(size,0.);
    p = new Resp(els,steps);
    q = new Resp(els,steps);

    // setup plot
    thePlot = new QCustomPlot();
    thePlot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    /*
    QRect rec = QApplication::desktop()->screenGeometry();
    int height = 0.7*rec.height();
    int width = 0.7*rec.width();
    thePlot->setMinimumHeight(0.2*height);
    thePlot->setMaximumHeight(0.2*height);
    thePlot->setMinimumWidth(0.4*width);
    */
    //thePlot->setMaximumWidth(0.4*width);
    thePlot->xAxis->setLabel(xLabel);
    thePlot->yAxis->setLabel(yLabel);

    // axes
    thePlot->xAxis->setVisible(false);
    thePlot->yAxis->setVisible(false);
    thePlot->xAxis2->setVisible(false);
    thePlot->yAxis2->setVisible(false);
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

    // print current value...
    label = new QLabel;

    // add to layout
    QVBoxLayout *Lay = new QVBoxLayout(this);
    Lay->addWidget(thePlot,1);
    Lay->addWidget(label);
    Lay->setMargin(0);
    this->setLayout(Lay);
}

responseWidget::~responseWidget()
{
    //delete thePlot;
    //delete graph;
    delete xi;
    delete yi;
    delete p;
    delete q;
}

void responseWidget::setModel(QVector<double> *data_x)
{
    size = data_x->size();

    // initialize
    xi->resize(size);
    yi->resize(size);
    p->reSize(size,steps);
    q->reSize(size,steps);

    // set
    for (int i=0; i<size; i++) {
        (*xi)[i] = (*data_x)[i];
    }
    yi->fill(0.);

    // max -X
    maxX = 0.;
    minX = 0.;
    for (int j=0; j < size; j++) {
        double val = (*xi)[j];
        if (val > maxX)
            maxX = val;
        if (val < minX)
            minX = val;
    }

    // max -Y
    maxY = 0.;
    minY = 0.;
    for (int j=0; j < size; j++) {
        double val = (*yi)[j];
        if (val > maxY)
            maxY = val;
        if (val < minY)
            minY = val;
    }
}

/*
void responseWidget::setShape(Resp *data_x, Resp *data_y, double scale)
{
    //size = data_x->size;
    steps = data_x->steps;

    // add to coordinates
    xj->reSize(size,steps);
    yj->reSize(size,steps);

    for (int t=0; t<steps; t++)
        for (int j=0; j<size; j++)
        {
            (*xj->data[j])[t] = (*xi)[j] + (*data_x->data[j])[t]*scale;
            (*yj->data[j])[t] = (*yi)[j] + (*data_y->data[j])[t]*scale;
        }

    // max -X
    for (int t=0; t<steps; t++) {
        for (int j=0; j < size; j++) {
            double val = (*xj->data[j])[t];
            if (val > maxX)
                maxX = val;
            if (val < minX)
                minX = val;
        }
    }

    // max -Y
    for (int t=0; t<steps; t++) {
        for (int j=0; j < size; j++) {
            double val = (*yj->data[j])[t];
            if (val > maxY)
                maxY = val;
            if (val < minY)
                minY = val;
        }
    }
}
*/

void responseWidget::setResp(Resp *data_p, Resp *data_q)
{
    els = data_p->size;
    steps = data_p->steps;

    // response
    q->reSize(els,steps);
    p->reSize(els,steps);

    // set
    for (int t=0; t<steps; t++)
        for (int j=0; j<els; j++)
        {
            (*p->data[j])[t] = (*data_p->data[j])[t];
            (*q->data[j])[t] = (*data_q->data[j])[t];
        }

    // max -q
    for (int t=0; t<steps; t++) {
        for (int j=0; j < els; j++) {
            double val = (*q->data[j])[t];
            if (val > maxY)
                maxY = val;
            if (val < minY)
                minY = val;
        }
    }

    // max -p
    for (int t=0; t<steps; t++) {
        for (int j=0; j < els; j++) {
            double val = (*p->data[j])[t];
            if (val > maxY)
                maxY = val;
            if (val < minY)
                minY = val;
        }
    }
}

void responseWidget::plotModel()
{
    // setup system plot
    thePlot->clearPlottables();
    thePlot->clearGraphs();
    //thePlot->clearItems();

    // add graph
    graph = thePlot->addGraph();

    // create pen
    QPen pen;
    pen.setWidthF(3);

    // line color
    pen.setColor(QColor(Qt::gray));
    thePlot->graph(0)->setPen(pen);
    thePlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));

    // set data
    thePlot->graph(0)->setData(*xi,*yi,true);

    // axes
    thePlot->xAxis->setRange(minX-10,maxX+10);
    thePlot->yAxis->setRange(minY-1,maxY+1);

    // update plot
    thePlot->replot(QCustomPlot::rpQueuedReplot);
    thePlot->update();

    // update label
    //label->setText(QString("current = %1").arg(maxY,0,'f',2));
}

void responseWidget::plotResponse(int t)
{
    // add graph
    plotModel();

    // create pen
    QPen pen;
    pen.setWidthF(3);

    // set data
    QVector<double> *pt = new QVector<double>(els,0.);
    QVector<double> *qt = new QVector<double>(els,0.);
    double maxYt = 0.;
    for (int j=0; j<els; j++) {
        (*pt)[j] = (*p->data[j])[t];
        (*qt)[j] = (*q->data[j])[t];

        // grab max
        if (fabs((*pt)[j]) > fabs(maxYt))
            maxYt = (*pt)[j];
        if (fabs((*qt)[j]) > fabs(maxYt))
            maxYt = (*qt)[j];
    }

    for (int j=0; j<els; j++) {
        graph = thePlot->addGraph();
        pen.setColor(QColor(Qt::blue));
        thePlot->graph()->setPen(pen);
        thePlot->graph()->setBrush(QBrush(QColor(0,0,255,20)));
        thePlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 7));

        QVector<double> x(2), y(2);
        x[0] = (*xi)[j+2]; x[1] = (*xi)[j+3];
        y[0] = (*pt)[j];  y[1] = (*qt)[j];
        thePlot->graph()->setData(x,y,true);

        /*
        QCPCurve *polygon = new QCPCurve(thePlot->xAxis, thePlot->yAxis);
        polygon->addData(x[0],y[0]);
        polygon->addData(x[1],y[0]);
        polygon->addData(x[1],y[1]);
        polygon->addData(x[0],y[1]);
        polygon->addData(x[0],y[0]);

        polygon->setPen(pen);
        pen.setColor(QColor(255,0,0));
        polygon->setBrush(QBrush(QColor(255,0,0,20)));
        polygon->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 10));
        */
    }

    // axes
    thePlot->xAxis->setRange(minX-10,maxX+10);
    thePlot->yAxis->setRange(minY-1,maxY+1);

    // update plot
    thePlot->replot(QCustomPlot::rpQueuedReplot);
    thePlot->update();

    // update label
    label->setText(QString("current = %1").arg(maxYt,0,'f',2));

    delete pt;
    delete qt;
}
