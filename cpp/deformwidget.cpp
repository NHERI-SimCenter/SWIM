#include "deformwidget.h"
#include <qcustomplot/qcustomplot.h>
#include "resp.h"
#include <QLabel>

deformWidget::deformWidget(QString xLabel, QString yLabel, QWidget *parent)
              : QWidget(parent)
{
    // initialize
    steps = 2;
    size = 2;
    xi = new QVector<double>(size,0.);
    yi = new QVector<double>(size,0.);
    xj = new Resp(size,steps);
    yj = new Resp(size,steps);

    // setup plot
    thePlot = new QCustomPlot();
    thePlot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    //  QRect rec = QApplication::desktop()->screenGeometry();
    //  int height = 0.7*rec.height();
    //  thePlot->setMinimumHeight(0.2*height);
    //  thePlot->setMaximumHeight(0.2*height);
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

    // print current value...
    label = new QLabel;

    // add to layout
    QVBoxLayout *Lay = new QVBoxLayout(this);
    Lay->addWidget(thePlot,1);
    Lay->addWidget(label);
    Lay->setMargin(0);
    this->setLayout(Lay);
}

deformWidget::~deformWidget()
{
   // delete thePlot;  Qt deletes these, causing seg fault
   // delete graph;
    delete xi;
    delete xj;
    delete yi;
    delete yj;
}

void deformWidget::setModel(QVector<double> *data_x, QVector<double> *data_y)
{
    size = data_x->size();

    if(size>0)
    {
        // initialize
        xi->resize(size);
        yi->resize(size);
        xj->reSize(size,steps);
        yj->reSize(size,steps);


        // set
        for (int i=0; i < size; i++) {
            (*xi)[i] = (*data_x)[i];
            (*yi)[i] = (*data_y)[i];
        }
        //xi = data_x; seg fault on destructor as just setting pointer to point to something else
        //yi = data_y;

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
}

void deformWidget::setResp(Resp *data_x, Resp *data_y)
{
    //size = data_x->size;
    steps = data_x->steps;

    // add to coordinates
    xj->reSize(size,steps);
    yj->reSize(size,steps);
/*
    for (int t=0; t<steps; t++)
        for (int j=0; j<size; j++)
        {
            //(*xj->data[j])[t] = (*xi)[j] + (*data_x->data[j])[t];
            //(*yj->data[j])[t] = (*yi)[j] + (*data_y->data[j])[t];
            (*xj->data[j])[t] = (*data_x->data[j])[t];
            (*yj->data[j])[t] = (*data_y->data[j])[t];
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
*/
    //minX = 0; maxX = 20;
    //minY = 0; maxY = 20;
}

void deformWidget::setResp(std::vector<std::vector<double>> *dispxtmp, std::vector<std::vector<double>> *dispytmp)
{
    //dispx->clear(); dispy->clear();
    dispx = dispxtmp;
    dispy = dispytmp;
}

void deformWidget::plotModel()
{
    thePlot->clearPlottables();
    thePlot->clearGraphs();
    //thePlot->clearItems();
    /*
    // add graph
    graph = thePlot->addGraph();

    // create pen
    QPen pen;
    pen.setWidthF(3);

    // line color
    pen.setColor(QColor(Qt::black));
    thePlot->graph(0)->setPen(pen);
    thePlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 7));

    // set data
    thePlot->graph(0)->setData(*xi,*yi,true);
    */

    QString expDirName = "/Users/simcenter/Codes/SimCenter/SWIM/data/wallDemo";
    QString samFileName = expDirName + "/SAM.json";
    QString in;
    QFile inputFile(samFileName);
    if(inputFile.open(QFile::ReadOnly)) {
    //inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    in = inputFile.readAll();
    inputFile.close();
    }else{
        // do nothing.
    }

    loc.clear();
    std::vector<double> xs;
    std::vector<double> ys;
    QJsonObject samRoot, SAM, geometry;
    QJsonArray nodesJ;
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
        nodesJ = geometry["nodes"].toArray();

        for (auto nodej : nodesJ) {
            int thisNodeName = nodej.toObject()["name"].toInt();
            QJsonArray thisNodeCoorJ = nodej.toObject()["crd"].toArray();
            double thisNodex = thisNodeCoorJ[0].toDouble();
            double thisNodey = thisNodeCoorJ[1].toDouble();
            loc.push_back({thisNodex,thisNodey});
            xs.push_back(thisNodex);
            ys.push_back(thisNodey);
        }
    }

    size = int(loc.size());



    std::sort(xs.begin(), xs.end());
    xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
    for (auto it = xs.cbegin(); it != xs.cend(); ++it)
        qDebug() << it[0] << " ";
    std::sort(ys.begin(), ys.end());
    ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
    for (auto it = ys.cbegin(); it != ys.cend(); ++it)
        qDebug() << it[0] << " ";

    double x,y;
    horizontalIndex.clear();
    for (auto ity = ys.cbegin(); ity != ys.cend(); ++ity)
    {
        y = ity[0];
        std::vector<int> hindInner;
        for (auto itx = xs.cbegin(); itx != xs.cend(); ++itx)
        {
            x = itx[0];
            auto itr = std::find_if(loc.cbegin(), loc.cend(), compare({x,y}));

            if (itr != loc.cend()) {
                hindInner.push_back(int(std::distance(loc.cbegin(), itr)));
                qDebug() << "Element present at index " << std::distance(loc.cbegin(), itr);
            }
            else {
                qDebug() << "Element not found";
            }
        }
        horizontalIndex.push_back(hindInner);
    }
    verticalIndex.clear();
    for (auto itx = xs.cbegin(); itx != xs.cend(); ++itx)
    {
        x = itx[0];
        std::vector<int> vindInner;
        for (auto ity = ys.cbegin(); ity != ys.cend(); ++ity)
        {
            y= ity[0];
            auto itr = std::find_if(loc.cbegin(), loc.cend(), compare({x,y}));

            if (itr != loc.cend()) {
                vindInner.push_back(int(std::distance(loc.cbegin(), itr)));
                qDebug() << "Element present at index " << std::distance(loc.cbegin(), itr);
            }
            else {
                qDebug() << "Element not found";
            }
        }
        verticalIndex.push_back(vindInner);
    }



    // create pen
    QPen pen;
    pen.setWidthF(1);
    pen.setColor(QColor(Qt::black));



    for(int i=0; i<int(verticalIndex.size());i++)
    {
        QVector<double> xh,yh;
        for (int j=0;j< verticalIndex[i].size(); j++)
        {
            xh.append(loc[verticalIndex[i][j]][0]);
            yh.append(loc[verticalIndex[i][j]][1]);
        }
        graph = thePlot->addGraph();
        thePlot->graph()->setPen(pen);
        thePlot->graph()->setData(xh,yh,true);
    }
    for(int i=0; i<int(horizontalIndex.size());i++)
    {
        QVector<double> xh,yh;
        for (int j=0;j< horizontalIndex[i].size(); j++)
        {
            xh.append(loc[horizontalIndex[i][j]][0]);
            yh.append(loc[horizontalIndex[i][j]][1]);
        }
        graph = thePlot->addGraph();
        thePlot->graph()->setPen(pen);
        //thePlot->graph()->setBrush(QBrush(QColor(0,0,255,100)));
        thePlot->graph()->setData(xh,yh,true);
    }

    //putSomeColorInMesh();



    /*
    // axes
    thePlot->xAxis->setRange(minX-10,maxX+10);
    thePlot->yAxis->setRange(minY-1,maxY+1);
    */
    // axes
    thePlot->xAxis->setRange(0-10,0+200);
    thePlot->yAxis->setRange(0-10,0+200);

    // update plot
    thePlot->replot(QCustomPlot::rpQueuedReplot);
    thePlot->update();

    // update label
    label->setText(QString("current = %1 in.").arg(maxY,0,'f',2));
}

void deformWidget::putSomeColorInMesh()
{

    // put some color
    QVector<double> xh,yh;

    int i = 0;
    for (int j=0;j< verticalIndex[i].size(); j++)
    {
        xh.append(loc[verticalIndex[i][j]][0]);
        yh.append(loc[verticalIndex[i][j]][1]);
    }

    i = horizontalIndex.size()-1;
    for (int j=0;j< horizontalIndex[i].size(); j++)
    {
        xh.append(loc[horizontalIndex[i][j]][0]);
        yh.append(loc[horizontalIndex[i][j]][1]);
    }
    i = verticalIndex.size()-1;
    for (int j=verticalIndex[i].size()-1;j>=0 ; j--)
    {
        xh.append(loc[verticalIndex[i][j]][0]);
        yh.append(loc[verticalIndex[i][j]][1]);
    }

    graph = thePlot->addGraph();
    //thePlot->graph()->setPen(pen);
    thePlot->graph()->setBrush(QBrush(QColor(0,0,255,100)));
    thePlot->graph()->setData(xh,yh,true);
}

void deformWidget::plotResponse_old(int t)
{
    // setup system plot
    thePlot->clearPlottables();
    thePlot->clearGraphs();

    // add graph
    graph = thePlot->addGraph();
    // create pen
    QPen pen;
    pen.setWidthF(3);
    // line color
    pen.setColor(QColor(Qt::black));
    thePlot->graph(0)->setPen(pen);
    thePlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 7));

    // set data
    QVector<double> *xt = new QVector<double>(size,0.);
    QVector<double> *yt = new QVector<double>(size,0.);
    double maxYt = 0.;
    for (int j=0; j<size; j++) {
        (*xt)[j] = (*xj->data[j])[t];
        (*yt)[j] = (*yj->data[j])[t];
        if (fabs((*yt)[j]) > fabs(maxYt))
            maxYt = (*yt)[j];
    }
    thePlot->graph(0)->setData(*xt,*yt,true);

    // axes
    thePlot->xAxis->setRange(minX-10,maxX+10);
    thePlot->yAxis->setRange(minY-1,maxY+1);

    // update plot
    thePlot->replot(QCustomPlot::rpQueuedReplot);
    thePlot->update();

    // update label
    label->setText(QString("current = %1 in.").arg(maxYt,0,'f',2));

    delete xt;
    delete yt;
}


void deformWidget::plotResponse(int t)
{
    // setup system plot
    thePlot->clearPlottables();
    thePlot->clearGraphs();

    // add graph
    graph = thePlot->addGraph();

    // create pen
    QPen pen;
    pen.setWidthF(1);
    pen.setColor(QColor(Qt::black));


    double x0,y0;
    double maxXt=0.;
    if(verticalIndex.size()>0 && dispx->size()>0)
    {
        for(int i=0; i<int(verticalIndex.size());i++)
        {
            QVector<double> xh,yh;
            for (int j=0;j< verticalIndex[i].size(); j++)
            {
                x0 = loc[verticalIndex[i][j]][0];
                y0 = loc[verticalIndex[i][j]][1];
                if(fabs(x0)>fabs(maxXt))
                    maxXt = x0;
                xh.append(x0+(*dispx)[t][verticalIndex[i][j]]);
                yh.append(y0+(*dispy)[t][verticalIndex[i][j]]);
            }
            graph = thePlot->addGraph();
            thePlot->graph()->setPen(pen);
            thePlot->graph()->setData(xh,yh,true);
        }
        for(int i=0; i<int(horizontalIndex.size());i++)
        {
            QVector<double> xh,yh;
            for (int j=0;j< horizontalIndex[i].size(); j++)
            {
                x0 = loc[horizontalIndex[i][j]][0];
                y0 = loc[horizontalIndex[i][j]][1];
                if(fabs(x0)>fabs(maxXt))
                    maxXt = x0;
                xh.append(x0+(*dispx)[t][horizontalIndex[i][j]]);
                yh.append(y0+(*dispy)[t][horizontalIndex[i][j]]);
            }
            graph = thePlot->addGraph();
            thePlot->graph()->setPen(pen);
            //thePlot->graph()->setBrush(QBrush(QColor(0,0,255,100)));
            thePlot->graph()->setData(xh,yh,true);
        }
    }

    //putSomeColorInMesh();



    /*
    // axes
    thePlot->xAxis->setRange(minX-10,maxX+10);
    thePlot->yAxis->setRange(minY-1,maxY+1);
    */
    // axes
    thePlot->xAxis->setRange(0-10,0+200);
    thePlot->yAxis->setRange(0-10,0+200);

    // update plot
    thePlot->replot(QCustomPlot::rpQueuedReplot);
    thePlot->update();

    // update label
    label->setText(QString("current = %1 in.").arg(maxXt,0,'f',2));

    //delete xt;
    //delete yt;
}
