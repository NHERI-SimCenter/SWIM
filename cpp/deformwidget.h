#ifndef DEFORMWIDGET_H
#define DEFORMWIDGET_H

#include <QWidget>
#include <QVector>
#include <QLabel>

class QCustomPlot;
class QCPGraph;
class Resp;

class deformWidget : public QWidget
{
    Q_OBJECT

private:
    QCustomPlot *thePlot;
    QCPGraph *graph;
    QLabel *label;
    QVector<double> *xi;
    QVector<double> *yi;
    Resp *xj;
    Resp *yj;
    int steps;
    int size;
    double maxX;
    double minX;
    double maxY;
    double minY;

public:
    deformWidget(QString xLabel, QString yLabel, QWidget *parent = 0);
    ~deformWidget();

    void setModel(QVector<double> *data_x, QVector<double> *data_y);
    void setResp(Resp *data_x, Resp *data_y);
    void plotModel();
    void plotResponse(int t = 0);
};

#endif // DEFORMWIDGET_H
