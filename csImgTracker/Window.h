#pragma once
#include "Visualizer.h"
#include <QMainWindow>

#include "opencv2/opencv.hpp"

#include "Common.h"

namespace Ui {
class Window;
}

struct VideoInfo {
  int numFrames;
  double fps;
  int width;
  int height;
};

class QTreeWidgetItem;

class Window : public QMainWindow {
  Q_OBJECT;

  std::unique_ptr<Ui::Window> ui;

  std::unique_ptr<Visualizer> vis;

  std::unique_ptr<cv::VideoCapture> video;
  VideoInfo videoInfo;
  std::map<int, TimestampAndPoints> _pointsPerFrame;
  std::map<int, QTreeWidgetItem*> _itemRootPerFrame;

  QImage _curimg;
  int _curFrame;
  double _curMsec;

  bool _first;
  int _rotated;

  void resetNoVideoData();

  void noVideoUpdate();
  void okVideoUpdate();

  void updateRotateCoord();

  QString trackedPoint2Text();
  Eigen::Vector2f toImageCoord(Eigen::Vector2f world);

  void closeEvent(QCloseEvent* e) override;

public:
  Window(QWidget* parent = 0);
  virtual ~Window();

  void on_imgDoubleClick(QMouseEvent* e);

public slots:

  void on_horizontalSlider_valueChanged(int n);

  void on_pushButtonRotateLeft_clicked();
  void on_pushButtonRotateRight_clicked();
  void on_pushButtonFit_clicked();

  void on_pushButtonPlay_clicked();

  void on_pushButtonCopy_clicked();
  void on_pushButtonSave_clicked();

  void on_pushButtonDeleteSelected_clicked();

  void on_pushButtonOpen_clicked();

  void on_treeWidget_itemDoubleClicked(QTreeWidgetItem* item, int column);
};
