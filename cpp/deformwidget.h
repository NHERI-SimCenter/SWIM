#ifndef DEFORMWIDGET_H
#define DEFORMWIDGET_H

#include <QWidget>
#include <QVector>
#include <QLabel>
#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_set>

class QCustomPlot;
class QCPGraph;
class Resp;


struct compare
{
    double tol = 1e-10;
    std::vector<double> key;
    compare(std::vector<double> const &i): key(i) { }

    bool operator()(std::vector<double> const &i)
    {
        return (fabs(i[0] - key[0])<tol && fabs(i[1] - key[1])<tol);
    }
};



/*
namespace SimCenterMath
{
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>( new T(std::forward<Args>(args)...));
}
}

class fastMatrix2D {
  size_t _rows;
  size_t _columns;
  std::unique_ptr<int[]> data;

public:
  fastMatrix2D(size_t rows, size_t columns)
      : _rows{rows},
        _columns{columns},
        data{SimCenterMath::make_unique<int[]>(rows * columns)} {}

  size_t rows() const { return _rows; }

  size_t columns() const { return _columns; }

  int *operator[](size_t row) { return row * _columns + data.get(); }

  int &operator()(size_t row, size_t column) {
    return data[row * _columns + column];
  }
};
*/

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

    std::vector<std::vector<int>> horizontalIndex;
    std::vector<std::vector<int>> verticalIndex;
    std::vector<int> outLineIndex;
    std::vector<std::vector<double>> loc;

    std::vector<std::vector<double>> *dispx;
    std::vector<std::vector<double>> *dispy;


public:
    deformWidget(QString xLabel, QString yLabel, QWidget *parent = 0);
    ~deformWidget();

    void setModel(QVector<double> *data_x, QVector<double> *data_y);
    void setResp(std::vector<std::vector<double>> *dispx, std::vector<std::vector<double>> *dispy);
    void setResp(Resp *data_x, Resp *data_y);
    void plotModel();
    void plotResponse(int t = 0);
    void plotResponse_old(int t = 0);
    void putSomeColorInMesh();
    void setExpDir(QString tmDir){expDirName = tmDir;}
    void setColor(QColor c){theColor=c;}
    QString expDirName = "";
    double wallw = 0.0;
    double wallh = 0.0;
    QColor theColor = QColor(96,96,96,150);
};

#endif // DEFORMWIDGET_H
