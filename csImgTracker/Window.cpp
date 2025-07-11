#include "Window.h"

#include <QClipboard>
#include <QImage>
#include <QLabel>
#include <QObject>

#include "ui_Window.h"

#include "csVisOpenGL/Camera.h"
#include "csVisOpenGL/OrbitCameraController.h"
#include <QMouseEvent>

#include "TreeWidgetItem.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTreeWidgetItem>

#include "CvMatAndQImage.h"

#include <iostream>

#include <QTimer>
#include <qt6/QtCore/qvariant.h>

Window::Window(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::Window())
    , vis(new Visualizer())
    , _first(true)
    , _rotated(0)
    , video(std::unique_ptr<cv::VideoCapture>()) {
  ui->setupUi(this);

  this->showMaximized();

  ui->preview->addVisualizer(vis.get());
  // static_cast<csVisOpenGL::OrbitCameraController*>(ui->preview->getCameraController())->setRadius(2);
  static_cast<csVisOpenGL::OrbitCameraController*>(ui->preview->getCameraController())
      ->setYpr(Eigen::Vector3f(-M_PI / 2, M_PI / 2, 0), false);

  csVisOpenGL::OrbitCameraController::Settings enabled;
  enabled.enableRot = false;
  enabled.enableScale = false;
  enabled.enableTrasl = false;

  auto controller = static_cast<csVisOpenGL::OrbitCameraController*>(ui->preview->getCameraController());
  controller->setEnabledControls(enabled);
  controller->setCameraOrthogonal();
  controller->setTraslationLimits(-Eigen::Vector3f::Ones(), Eigen::Vector3f::Ones());
  controller->setMinRadius(2);
  controller->setMaxRadius(2);
  controller->setRadius(2, false);
  // controller->setMinScale(0.25);
  // controller->setMaxScale(4.0);
  controller->setZNearFar(-1, 1);

  ui->treeWidget->setColumnCount(4);
  ui->treeWidget->setHeaderLabels({"Frame #", "Time [s]", "X [px]", "Y [px]"});
  ui->treeWidget->header()->setStretchLastSection(false);
  ui->treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->treeWidget->setExpandsOnDoubleClick(false);
  ui->treeWidget->setSortingEnabled(true);
  ui->treeWidget->sortByColumn(0, Qt::SortOrder::AscendingOrder);
  ui->treeWidget->header()->setSortIndicatorShown(false);

  ui->splitter->setStretchFactor(1, 0);
  ui->splitter->setSizes(QList{700, 100});

  ui->preview->setCursor(Qt::CrossCursor);

  QObject::connect(controller, &csVisOpenGL::OrbitCameraController::leftDoubleClicked, this, &Window::on_imgDoubleClick);

  resetNoVideoData();
  noVideoUpdate();

  QTimer::singleShot(200, this, &Window::on_pushButtonOpen_clicked);
}

Window::~Window() {}

void Window::updateRotateCoord() {
  for (const auto& f : _pointsPerFrame) {
    auto bi = _itemRootPerFrame[f.first];
    for (int i = 0; i < f.second.points.size(); i++) {
      auto imgCoord = toImageCoord(f.second.points[i]);
      bi->child(i)->setText(2, QString("%1").arg(int(imgCoord.x())));
      bi->child(i)->setText(3, QString("%1").arg(int(imgCoord.y())));
    }
  }
}

void Window::closeEvent(QCloseEvent* e) {
  auto res = QMessageBox::question(this, "Quit?", "Do you really want to close?", QMessageBox::Yes | QMessageBox::No);
  if (res != QMessageBox::Yes) {
    e->ignore();
  }
}

void Window::resetNoVideoData() {
  _curimg = QImage();
  _curFrame = -1;
  _curMsec = -1;
  _first = true;
  _rotated = 0;
  _pointsPerFrame.clear();
}

void Window::noVideoUpdate() {
  auto controller = static_cast<csVisOpenGL::OrbitCameraController*>(ui->preview->getCameraController());
  csVisOpenGL::OrbitCameraController::Settings enabled;
  enabled.enableRot = false;
  enabled.enableScale = false;
  enabled.enableTrasl = false;
  controller->setEnabledControls(enabled);

  ui->pushButtonPlay->setEnabled(false);
  ui->horizontalSlider->setEnabled(false);

  ui->horizontalSlider->blockSignals(true);
  ui->horizontalSlider->setValue(0);
  ui->horizontalSlider->blockSignals(false);

  ui->labelVideoPos->setText("No Video");
  ui->widgetControls->setEnabled(false);
  ui->widgetCopySave->setEnabled(false);
  ui->plainTextEditVideoInfo->setPlainText("No Video");

  ui->treeWidget->clear();

  ui->preview->makeCurrent();
  vis->setImage(QImage());
  vis->showPoints({});
}

void Window::okVideoUpdate() {

  if (ui->horizontalSlider->maximum() > 0) {
    ui->pushButtonPlay->setEnabled(true);
    ui->horizontalSlider->setEnabled(true);
  } else {
    ui->horizontalSlider->setEnabled(false);
    ui->pushButtonPlay->setEnabled(false);
  }

  ui->treeWidget->clear();

  ui->labelVideoPos->setText("");
  ui->widgetControls->setEnabled(true);
  ui->widgetCopySave->setEnabled(true);
  ui->plainTextEditVideoInfo->setPlainText(QString("Image width: %1 px\n"
                                                   "Image height: %2 px\n"
                                                   "Num frames: %3\n"
                                                   "FPS: %4 1/s\n")
                                               .arg(videoInfo.width)
                                               .arg(videoInfo.height)
                                               .arg(videoInfo.numFrames)
                                               .arg(videoInfo.fps));
}

// #define AUTOLOAD

void Window::on_pushButtonOpen_clicked() {
  // open the new video
#ifdef AUTOLOAD
  filename = "D:/OneDrive/Desktop/vertical.mp4";
#else
  QString selFilter = "Video File (*.mp4 *.avi *.mkv)";
  QString filter = selFilter + ";; all files (*.*)";

  auto videoLocations = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::MoviesLocation);
  QString basedir = videoLocations.size() > 0 ? videoLocations[0] : "";

  QString filename = QFileDialog::getOpenFileName(this, "Load a video file", basedir, filter, &selFilter);

  if (filename.size() == 0) {
    // cancelled, return
    return;
  }

#endif

  std::unique_ptr<cv::VideoCapture> newVideo(new cv::VideoCapture());
  bool ok = newVideo->open(filename.toStdString());
  // video open correctly?
  if (!ok) {
    QMessageBox::critical(this, "Error", "Error opening video from " + filename);
  } else {
    // swap video and newVideo
    video.swap(newVideo);

    resetNoVideoData();

    videoInfo.width = video->get(cv::CAP_PROP_FRAME_WIDTH);
    videoInfo.height = video->get(cv::CAP_PROP_FRAME_HEIGHT);
    videoInfo.fps = video->get(cv::CAP_PROP_FPS);
    videoInfo.numFrames = video->get(cv::CAP_PROP_FRAME_COUNT);
    if (videoInfo.numFrames < 0) {
      videoInfo.numFrames = 1;
    }

    ui->horizontalSlider->blockSignals(true);
    ui->horizontalSlider->setMinimum(0);
    ui->horizontalSlider->setValue(0);
    ui->horizontalSlider->setMaximum(videoInfo.numFrames - 1);
    ui->horizontalSlider->blockSignals(false);

    okVideoUpdate();

    on_horizontalSlider_valueChanged(ui->horizontalSlider->value());
  }
}

Eigen::Vector2f Window::toImageCoord(Eigen::Vector2f world) {
  double ir = double(videoInfo.width) / double(videoInfo.height);

  // 0 to 1
  world.y() += 1.0;
  world.y() /= 2.0;

  world.x() += ir;
  world.x() /= 2.0 * ir;
  world.x() = 1.0 - world.x();

  if (_rotated == 0) {
    Eigen::Vector2f world_r(world.x(), world.y());
    world_r.y() = 1 - world_r.y(); // y cartesian
    return world_r.array() * Eigen::Vector2f(videoInfo.width, videoInfo.height).array();
  } else if (_rotated == 1) {
    Eigen::Vector2f world_r(world.y(), 1 - world.x());
    world_r.y() = 1 - world_r.y(); // y cartesian
    return world_r.array() * Eigen::Vector2f(videoInfo.height, videoInfo.width).array();
  } else if (_rotated == 2) {
    Eigen::Vector2f world_r(1 - world.x(), 1 - world.y());
    world_r.y() = 1 - world_r.y(); // y cartesian
    return world_r.array() * Eigen::Vector2f(videoInfo.width, videoInfo.height).array();
  } else if (_rotated == 3) {
    Eigen::Vector2f world_r(1 - world.y(), world.x());
    world_r.y() = 1 - world_r.y(); // y cartesian
    return world_r.array() * Eigen::Vector2f(videoInfo.height, videoInfo.width).array();
  }
  return world;
}

QString Window::trackedPoint2Text() {
  QString ret;
  ret += QString("#ID timestamp[sec] X[px] Y[px]\n");
  for (const auto& f : this->_pointsPerFrame) {
    for (const auto& v : f.second.points) {
      auto ic = toImageCoord(v);
      ret +=
          QString("%1 %2 %3 %4\n").arg(f.first).arg(QLocale().toString(f.second.timestamp, 'f', 3)).arg(int(ic.x())).arg(int(ic.y()));
    }
    ret += QString("\n");
  }
  return ret;
}

void Window::on_pushButtonCopy_clicked() {
  auto s = trackedPoint2Text();
  QClipboard* clipboard = QGuiApplication::clipboard();
  clipboard->setText(s);
}

void Window::on_pushButtonSave_clicked() {
  QString selFilter = "Text File (*.txt)";
  QString filter = selFilter;

  auto docLocations = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::DocumentsLocation);
  QString basedir = docLocations.size() > 0 ? docLocations[0] : "";

  auto fname = QFileDialog::getSaveFileName(this, "Save", basedir, filter, &selFilter);
  if (fname.size() > 0) {
    QFile file(fname);
    if (file.open(QIODevice::WriteOnly)) {
      QTextStream stream(&file);
      auto s = trackedPoint2Text();
      stream << s << Qt::endl;
    } else {
      QMessageBox::critical(this, "Error", QString("Error saving to %1").arg(fname));
    }
  }
}

void Window::on_horizontalSlider_valueChanged(int n) {

  if (video == nullptr)
    return; // defensive

  _curFrame = int(video->get(cv::CAP_PROP_POS_FRAMES));
  if (_curFrame != n) {
    bool b = video->set(cv::CAP_PROP_POS_FRAMES, n);
    if (!b) {
      QMessageBox::critical(this, "Error", QString("Error in video seek while moving to frame %1").arg(_curFrame));
      resetNoVideoData();
      noVideoUpdate();
      return;
    }

    _curFrame = video->get(cv::CAP_PROP_POS_FRAMES);
    if (_curFrame != n) {
      QMessageBox::critical(this, "Error", QString("Error in video seek after moving to frame %1").arg(_curFrame));
      resetNoVideoData();
      noVideoUpdate();
      return;
    }
  }

  _curMsec = video->get(cv::CAP_PROP_POS_MSEC);

  cv::Mat frame;
  *video >> frame; // get a new frame from camera

  if (frame.empty()) {
    QMessageBox::critical(this, "Error", QString("Error in video acquisition of frame %1").arg(_curFrame));
    resetNoVideoData();
    noVideoUpdate();
    return;
  }

  _curimg = mat2Image(frame);

  if (_curimg.isNull()) {
    QMessageBox::critical(this, "Error", QString("Error converting frame %1 to image").arg(_curFrame));
    resetNoVideoData();
    noVideoUpdate();
    return;
  }

  _curimg = _curimg.convertedTo(QImage::Format::Format_RGB32);

  ui->labelVideoPos->setText(QString("#%1 - %2s").arg(_curFrame + 1).arg(QLocale().toString(_curMsec / 1000, 'f', 3)));

  ui->preview->makeCurrent();
  vis->setImage(_curimg);

  if (_pointsPerFrame.find(_curFrame) != _pointsPerFrame.end()) {
    vis->showPoints(_pointsPerFrame[_curFrame].points);
  } else {
    vis->showPoints({});
  }

  if (_first) {
    _first = false;
    // reset camera
    auto controller = static_cast<csVisOpenGL::OrbitCameraController*>(ui->preview->getCameraController());
    controller->setYpr(Eigen::Vector3f(-M_PI / 2, M_PI / 2, 0), false);
    controller->setPivotPosition(Eigen::Vector3f::Zero(), false);
    csVisOpenGL::OrbitCameraController::Settings enabled;
    enabled.enableRot = false;
    enabled.enableScale = true;
    enabled.enableTrasl = true;
    controller->setEnabledControls(enabled);

    on_pushButtonFit_clicked();
  }
  if (ui->pushButtonPlay->isChecked()) {
    if (ui->horizontalSlider->value() < ui->horizontalSlider->maximum()) {
      QTimer::singleShot(10, this, [this]() {
        if (!ui->horizontalSlider->isEnabled()) {
          ui->horizontalSlider->setValue(ui->horizontalSlider->value() + 1);
        } else {
          // was aborted
        }
      });
    } else {
      ui->pushButtonPlay->setChecked(false);
      ui->horizontalSlider->setValue(0);
      ui->horizontalSlider->setEnabled(true);
      ui->pushButtonOpen->setEnabled(true);
    }
  }
}

void Window::on_pushButtonRotateLeft_clicked() {
  if (_curimg.isNull())
    return;

  auto controller = static_cast<csVisOpenGL::OrbitCameraController*>(ui->preview->getCameraController());
  auto ypr = controller->getYprDest();
  ypr(0) -= M_PI / 2;
  _rotated = _rotated + 1;
  _rotated = _rotated % 4;
  controller->setYpr(ypr, true);

  updateRotateCoord();
}

void Window::on_pushButtonRotateRight_clicked() {
  if (_curimg.isNull())
    return;

  auto controller = static_cast<csVisOpenGL::OrbitCameraController*>(ui->preview->getCameraController());
  auto ypr = controller->getYprDest();
  ypr(0) += M_PI / 2;
  _rotated = _rotated - 1;
  if (_rotated < 0)
    _rotated = 3;
  controller->setYpr(ypr, true);

  updateRotateCoord();
}

void Window::on_imgDoubleClick(QMouseEvent* e) {
  if (_curimg.isNull())
    return;
  if (ui->pushButtonPlay->isChecked())
    return;

  // Get clicking position in OpenGL screen coordinates ([-1, 1], [-1, 1])
  const auto& camera = ui->preview->getCamera();

  Eigen::Vector2f clickPos = Eigen::Vector2f(e->pos().x(), camera.getViewSize().y() - 1 - e->pos().y());
  clickPos.x() /= camera.getViewSize().x();
  clickPos.y() /= camera.getViewSize().y();
  clickPos = (clickPos * 2) - Eigen::Vector2f::Ones();

  // Compute texture coordinates assuming (-1, -1) is the top-left corner
  Eigen::Vector2f world2DCoords = (camera.getViewProjection().inverse() * Eigen::Vector4f(clickPos.x(), clickPos.y(), 0, 1)).head<2>();

  // std::cout << world2DCoords.transpose() << std::endl;

  double ir = double(videoInfo.width) / double(videoInfo.height);
  if (world2DCoords.y() < -1.0)
    return;
  if (world2DCoords.y() > 1.0)
    return;
  if (world2DCoords.x() < -ir)
    return;
  if (world2DCoords.x() > +ir)
    return;

  _pointsPerFrame[_curFrame].points.push_back(world2DCoords);
  if (_pointsPerFrame[_curFrame].points.size() == 1) {
    _pointsPerFrame[_curFrame].timestamp = _curMsec / 1000;
    // add root element
    TreeWidgetItem* treeItem = new TreeWidgetItem(ui->treeWidget);
    treeItem->setText(0, QString("%1").arg(_curFrame + 1));
    treeItem->setData(0, Qt::UserRole, _curFrame);
    treeItem->setData(0, Qt::UserRole + 1, -1);
    treeItem->setText(1, QLocale().toString(_curMsec / 1000, 'f', 3));
    treeItem->setExpanded(true);
    _itemRootPerFrame[_curFrame] = treeItem;
  }
  // add this element
  TreeWidgetItem* childItem = new TreeWidgetItem(nullptr);
  childItem->setData(0, Qt::UserRole, _curFrame);
  childItem->setData(0, Qt::UserRole + 1, static_cast<int>(_pointsPerFrame[_curFrame].points.size()) - 1);

  auto imgCoord = toImageCoord(world2DCoords);

  childItem->setText(2, QString("%1").arg(int(imgCoord.x())));
  childItem->setText(3, QString("%1").arg(int(imgCoord.y())));
  _itemRootPerFrame[_curFrame]->addChild(childItem);

  // update visualization
  vis->showPoints(_pointsPerFrame[_curFrame].points);
}

void Window::on_pushButtonFit_clicked() {
  if (_curimg.isNull())
    return;

  auto controller = static_cast<csVisOpenGL::OrbitCameraController*>(ui->preview->getCameraController());
  auto viewSize = ui->preview->getCamera().getViewSize();
  float sr = float(viewSize.x()) / float(viewSize.y());
  float ir = 1.0f;
  if (_rotated % 2 == 0) {
    ir = float(_curimg.width()) / float(_curimg.height());
    if (ir > sr) {
      float scale = ir / sr;
      controller->setScale(scale, false);
      controller->setScaleLimits(scale / 4.0f, scale * 4.0f);
    } else {
      controller->setScale(1.0, false);
      controller->setScaleLimits(0.25f, 4.0f);
    }
  } else {
    ir = float(_curimg.width()) / float(_curimg.height());
    float scale1;
    if (ir > 1) {
      scale1 = ir;
    } else {
      scale1 = 1 / ir;
    }

    float scale2;
    if (sr > 1) {
      scale2 = 1;
    } else {
      scale2 = 1 / sr;
    }

    float scale = std::max(scale1, scale2);

    controller->setScale(scale, false);
    controller->setScaleLimits(scale / 4.0f, scale * 4.0f);
  }
  controller->setPivotPosition(Eigen::Vector3f::Zero(), false);
}

void Window::on_pushButtonPlay_clicked() {

  if (ui->pushButtonPlay->isChecked()) {
    ui->horizontalSlider->setEnabled(false);
    ui->pushButtonOpen->setEnabled(false);
    on_horizontalSlider_valueChanged(ui->horizontalSlider->value());
  } else {
    ui->horizontalSlider->setEnabled(true);
    ui->pushButtonOpen->setEnabled(true);
  }
}

void Window::on_treeWidget_itemDoubleClicked(QTreeWidgetItem* item, int column) {
  int curFrame = item->data(0, Qt::UserRole).toInt();
  ui->horizontalSlider->setValue(curFrame);
}

void Window::on_pushButtonDeleteSelected_clicked() {
  auto item = ui->treeWidget->currentItem();
  if (item == nullptr)
    return;
  int curFrame = item->data(0, Qt::UserRole).toInt();

  // check if it is not current frame
  if (curFrame != ui->horizontalSlider->value()) {
    auto ans = QMessageBox::question(this, "Confirm",
                                     "You are deleting a point that is not in current frame, are you sure you want to continue?",
                                     QMessageBox::Yes | QMessageBox::No);
    if (ans == QMessageBox::No) {
      return;
    }
    ui->horizontalSlider->setValue(curFrame);
  }

  int curId = item->data(0, Qt::UserRole + 1).toInt();
  if (curId == -1) {
    auto ans = QMessageBox::question(this, "Confirm", "You are deleting ALL points in this frame, are you sure you want to continue?",
                                     QMessageBox::Yes | QMessageBox::No);
    if (ans == QMessageBox::No) {
      return;
    }
  }

  // should delete all points in this frame?
  if (curId == -1) {
    _pointsPerFrame.erase(curFrame);
    delete _itemRootPerFrame[curFrame];
    _itemRootPerFrame.erase(curFrame);
  } else {

    // delete from memory
    _pointsPerFrame[curFrame].points.erase(_pointsPerFrame[curFrame].points.begin() + curId);

    // update indexes of remaining
    auto parent = _itemRootPerFrame[curFrame];
    // delete from tree
    int n = parent->childCount();
    parent->removeChild(item);

    assert(parent->childCount() == n - 1);
    n = parent->childCount();
    for (int i = 0; i < n; i++) {
      auto child = parent->child(i);
      int childId = child->data(0, Qt::UserRole + 1).toInt();
      if (childId > curId) {
        child->setData(0, Qt::UserRole + 1, childId - 1);
      }
    }

    // check if no more items
    if (_pointsPerFrame[curFrame].points.size() == 0) {
      _pointsPerFrame.erase(curFrame);
      _itemRootPerFrame.erase(curFrame);
      delete parent;
    }
  }

  // update visualizer
  vis->showPoints(_pointsPerFrame[_curFrame].points);
}
