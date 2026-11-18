#include "Window.h"

#include <QClipboard>
#include <QImage>
#include <QLabel>
#include <QObject>

#include "ui_Window.h"

#include <QMouseEvent>
#include <csVisOpenGL/Camera.hpp>
#include <csVisOpenGL/FrameSignals.hpp>
#include <csVisOpenGL/OrbitCameraController.hpp>

#include "TreeWidgetItem.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMessageBox>
#include <QShortcut>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStyle>
#include <QTreeWidgetItem>

#include "AboutDialog.h"
#include "CvMatAndQImage.h"

#include <iostream>

#include <QTimer>

namespace {
// the column of the points list with the delete icon
constexpr int kDeleteColumn = 4;
} // namespace

Window::Window(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::Window()),
      vis(std::make_shared<Visualizer>()),
      m_cameraController(
          std::make_shared<csVisOpenGL::OrbitCameraController>()),
      _first(true), _rotated(0), video(std::unique_ptr<cv::VideoCapture>()) {
  ui->setupUi(this);

  // add camera controller before show
  ui->preview->connectSlotsInterface(*m_cameraController);
  ui->preview->connectCameraControllerSignals(
      m_cameraController->getCameraControllerSignals());

  // show before adding visualizer
  this->showMaximized();

  // now can add
  ui->preview->addVisualizer(vis);

  m_cameraController->setYpr(Eigen::Vector3f(-M_PI / 2, M_PI / 2, 0), false);

  csVisOpenGL::OrbitCameraController::Settings enabled;
  enabled.enableRot = false;
  enabled.enableScale = false;
  enabled.enableTrasl = false;

  m_cameraController->setEnabledControls(enabled);
  m_cameraController->setCameraOrthogonal();
  m_cameraController->setTraslationLimits(-Eigen::Vector3f::Ones(),
                                          Eigen::Vector3f::Ones());
  m_cameraController->setMinRadius(2);
  m_cameraController->setMaxRadius(2);
  m_cameraController->setRadius(2, false);
  // m_cameraController->setMinScale(0.25);
  // m_cameraController->setMaxScale(4.0);
  m_cameraController->setZNearFar(-1, 1);

  ui->treeWidget->setColumnCount(5);
  ui->treeWidget->setHeaderLabels(
      {"Frame", "Time [s]", "X [px]", "Y [px]", ""});
  ui->treeWidget->header()->setStretchLastSection(false);
  ui->treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->treeWidget->setExpandsOnDoubleClick(false);
  ui->treeWidget->setSortingEnabled(true);
  ui->treeWidget->sortByColumn(0, Qt::SortOrder::AscendingOrder);
  ui->treeWidget->header()->setSortIndicatorShown(false);

  ui->splitter->setStretchFactor(0, 1);
  ui->splitter->setStretchFactor(1, 0);
  ui->splitter->setSizes(QList{900, 300});

  // transport bar: icons, play/pause follows the state of the button
  ui->pushButtonPrev->setIcon(
      style()->standardIcon(QStyle::SP_MediaSeekBackward));
  ui->pushButtonNext->setIcon(
      style()->standardIcon(QStyle::SP_MediaSeekForward));
  updatePlayButton(false);
  QObject::connect(ui->pushButtonPlay, &QPushButton::toggled, this,
                   &Window::updatePlayButton);
  for (QPushButton *b :
       {ui->pushButtonPrev, ui->pushButtonPlay, ui->pushButtonNext}) {
    b->setFocusPolicy(Qt::NoFocus);
  }

  // keyboard shortcuts (also written in the tooltips)
  ui->pushButtonOpen->setShortcut(QKeySequence::Open);
  ui->pushButtonSave->setShortcut(QKeySequence::Save);
  // Del in the points list: delete the selected point or frame
  auto *deleteShortcut = new QShortcut(QKeySequence::Delete, ui->treeWidget);
  deleteShortcut->setContext(Qt::WidgetWithChildrenShortcut);
  QObject::connect(deleteShortcut, &QShortcut::activated, this,
                   [this] { deleteItem(ui->treeWidget->currentItem()); });
  ui->pushButtonPlay->setShortcut(QKeySequence(Qt::Key_Space));
  ui->pushButtonPrev->setShortcut(QKeySequence(Qt::Key_Left));
  ui->pushButtonNext->setShortcut(QKeySequence(Qt::Key_Right));
  ui->pushButtonFit->setShortcut(QKeySequence(Qt::Key_F));

  // how to use the image view, always visible
  ui->statusbar->addWidget(
      new QLabel(QString("Double-click: mark a point     Mouse wheel: zoom     "
                         "Drag: pan     Space: play / pause     "
                         "%1 %2: previous / next frame")
                     .arg(QChar(0x2190))
                     .arg(QChar(0x2192))));
  ui->statusbar->setSizeGripEnabled(false);

  // Space plays / pauses wherever the focus is in this window, the points
  // list keeps its arrow keys (eventFilter)
  qApp->installEventFilter(this);

  ui->preview->setCursor(Qt::CrossCursor);
  // The image view takes no keyboard focus: zoom and pan are by mouse (wheel
  // and drag work without focus), the keys go to the shortcuts of the window.
  // With the focus, the camera controller moves while it believes a key held
  // (W/S/A/D, arrows), and a key release lost to a dialog or a shortcut left
  // it zooming without end.
  ui->preview->setFocusPolicy(Qt::NoFocus);

  QObject::connect(&ui->preview->getFrameSignals(),
                   &csVisOpenGL::FrameSignals::mouseDoubleClick, this,
                   &Window::on_imgDoubleClick);

  resetNoVideoData();
  noVideoUpdate();

  QTimer::singleShot(200, this, &Window::on_pushButtonOpen_clicked);
}

Window::~Window() {}

void Window::updateRotateCoord() {
  for (const auto &f : _pointsPerFrame) {
    auto bi = _itemRootPerFrame[f.first];
    for (int i = 0; i < f.second.points.size(); i++) {
      auto imgCoord = toImageCoord(f.second.points[i]);
      bi->child(i)->setText(2, QString("%1").arg(int(imgCoord.x())));
      bi->child(i)->setText(3, QString("%1").arg(int(imgCoord.y())));
    }
  }
}

void Window::closeEvent(QCloseEvent *e) {
  if (_pointsPerFrame.empty()) {
    return;
  }
  auto res = QMessageBox::question(
      this, "Quit?",
      QString("Close csImgTracker? The %1 tracked point(s) are lost if not "
              "saved.")
          .arg(countPoints()),
      QMessageBox::Yes | QMessageBox::No);
  if (res != QMessageBox::Yes) {
    e->ignore();
  }
}

// Before a shortcut, Qt lets the focused widget claim the key
// (ShortcutOverride): Space always goes to the play / pause shortcut, the
// arrows stay with the points list when it has the focus (to open and close
// the frames). Only for the widgets of this window, not for its dialogs.
bool Window::eventFilter(QObject *obj, QEvent *e) {
  if (e->type() == QEvent::ShortcutOverride) {
    auto *w = qobject_cast<QWidget *>(obj);
    auto *ke = static_cast<QKeyEvent *>(e);
    if (w != nullptr && w->window() == this &&
        ke->modifiers() == Qt::NoModifier) {
      if (ke->key() == Qt::Key_Space) {
        e->ignore(); // not claimed: the shortcut runs
        return true;
      }
      if ((ke->key() == Qt::Key_Left || ke->key() == Qt::Key_Right) &&
          (w == ui->treeWidget || ui->treeWidget->isAncestorOf(w))) {
        e->accept(); // claimed: the points list gets the key
        return true;
      }
    }
  }
  return QMainWindow::eventFilter(obj, e);
}

int Window::countPoints() const {
  int n = 0;
  for (const auto &f : _pointsPerFrame) {
    n += static_cast<int>(f.second.points.size());
  }
  return n;
}

void Window::resetNoVideoData() {
  _curimg = QImage();
  _curFrame = -1;
  _curMsec = -1;
  _first = true;
  _rotated = 0;
  _pointsPerFrame.clear();
  // the items are deleted with the tree (noVideoUpdate, okVideoUpdate)
  _itemRootPerFrame.clear();
}

void Window::noVideoUpdate() {
  csVisOpenGL::OrbitCameraController::Settings enabled;
  enabled.enableRot = false;
  enabled.enableScale = false;
  enabled.enableTrasl = false;
  m_cameraController->setEnabledControls(enabled);

  ui->pushButtonPlay->setEnabled(false);
  setSeekEnabled(false);

  ui->horizontalSlider->blockSignals(true);
  ui->horizontalSlider->setValue(0);
  ui->horizontalSlider->blockSignals(false);

  ui->labelVideoPos->setText("No video");
  setWindowTitle("csImgTracker");
  ui->widgetControls->setEnabled(false);
  ui->widgetCopySave->setEnabled(false);
  ui->labelVideoInfo->setTextFormat(Qt::PlainText);
  ui->labelVideoInfo->setText("No video loaded");

  ui->treeWidget->clear();

  ui->preview->makeCurrent();
  vis->setImage(QImage());
  vis->showPoints({});
}

void Window::okVideoUpdate() {

  if (ui->horizontalSlider->maximum() > 0) {
    ui->pushButtonPlay->setEnabled(true);
    setSeekEnabled(true);
  } else {
    setSeekEnabled(false);
    ui->pushButtonPlay->setEnabled(false);
  }

  ui->treeWidget->clear();

  ui->labelVideoPos->setText("");
  ui->widgetControls->setEnabled(true);
  ui->widgetCopySave->setEnabled(true);
  updateVideoInfoCard();
}

void Window::updateVideoInfoCard() {
  const QLocale locale;
  QString duration = "-";
  if (videoInfo.fps > 0) {
    duration =
        locale.toString(videoInfo.numFrames / videoInfo.fps, 'f', 1) + " s";
  }
  ui->labelVideoInfo->setTextFormat(Qt::RichText);
  ui->labelVideoInfo->setText(QString("<b>%1</b><br>"
                                      "%2 &times; %3 px<br>"
                                      "%4 frames, %5 fps<br>"
                                      "Duration: %6")
                                  .arg(_videoName.toHtmlEscaped())
                                  .arg(videoInfo.width)
                                  .arg(videoInfo.height)
                                  .arg(videoInfo.numFrames)
                                  .arg(locale.toString(videoInfo.fps, 'g', 4))
                                  .arg(duration));
}

// The folder the open dialog starts in: the sample video of the deployment
// (data/ next to bin/, where cs_deployment puts the executable), when it
// exists; else the Movies folder of the user.
static QString defaultVideoDir() {
  const QDir data(QCoreApplication::applicationDirPath() + "/../data");
  if (data.exists()) {
    return data.absolutePath();
  }
  auto videoLocations = QStandardPaths::standardLocations(
      QStandardPaths::StandardLocation::MoviesLocation);
  return videoLocations.size() > 0 ? videoLocations[0] : "";
}

void Window::on_pushButtonOpen_clicked() {
  if (!_pointsPerFrame.empty()) {
    auto res = QMessageBox::question(
        this, "Open another video?",
        QString("Opening another video discards the %1 tracked point(s) of "
                "this one, if not saved. Continue?")
            .arg(countPoints()),
        QMessageBox::Yes | QMessageBox::No);
    if (res != QMessageBox::Yes) {
      return;
    }
  }

  // open the new video
  QString selFilter = "Video File (*.mp4 *.avi *.mkv)";
  QString filter = selFilter + ";; all files (*.*)";

  QString basedir = defaultVideoDir();

  QString filename = QFileDialog::getOpenFileName(this, "Load a video file",
                                                  basedir, filter, &selFilter);

  if (filename.size() == 0) {
    // cancelled, return
    return;
  }

  std::unique_ptr<cv::VideoCapture> newVideo(new cv::VideoCapture());
  bool ok = newVideo->open(filename.toUtf8().constData());
  // video open correctly?
  if (!ok) {
    QMessageBox::critical(this, "Error",
                          "Error opening video from " + filename);
  } else {
    // swap video and newVideo
    video.swap(newVideo);

    resetNoVideoData();

    _videoName = QFileInfo(filename).fileName();
    setWindowTitle(_videoName + " - csImgTracker");

    videoInfo.width = video->get(cv::CAP_PROP_FRAME_WIDTH);
    videoInfo.height = video->get(cv::CAP_PROP_FRAME_HEIGHT);
    videoInfo.fps = video->get(cv::CAP_PROP_FPS);
    videoInfo.numFrames = video->get(cv::CAP_PROP_FRAME_COUNT);
    if (videoInfo.numFrames <= 0) {
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
    return world_r.array() *
           Eigen::Vector2f(videoInfo.width, videoInfo.height).array();
  } else if (_rotated == 1) {
    Eigen::Vector2f world_r(world.y(), 1 - world.x());
    world_r.y() = 1 - world_r.y(); // y cartesian
    return world_r.array() *
           Eigen::Vector2f(videoInfo.height, videoInfo.width).array();
  } else if (_rotated == 2) {
    Eigen::Vector2f world_r(1 - world.x(), 1 - world.y());
    world_r.y() = 1 - world_r.y(); // y cartesian
    return world_r.array() *
           Eigen::Vector2f(videoInfo.width, videoInfo.height).array();
  } else if (_rotated == 3) {
    Eigen::Vector2f world_r(1 - world.y(), world.x());
    world_r.y() = 1 - world_r.y(); // y cartesian
    return world_r.array() *
           Eigen::Vector2f(videoInfo.height, videoInfo.width).array();
  }
  return world;
}

QString Window::trackedPoint2Text() {
  QString ret;
  ret += QString("#ID timestamp[sec] X[px] Y[px]\n");
  for (const auto &f : this->_pointsPerFrame) {
    for (const auto &v : f.second.points) {
      auto ic = toImageCoord(v);
      // the frame number as in the list and the transport bar (from 1)
      ret += QString("%1 %2 %3 %4\n")
                 .arg(f.first + 1)
                 .arg(QLocale().toString(f.second.timestamp, 'f', 3))
                 .arg(int(ic.x()))
                 .arg(int(ic.y()));
    }
    ret += QString("\n");
  }
  return ret;
}

void Window::on_pushButtonCopy_clicked() {
  auto s = trackedPoint2Text();
  QClipboard *clipboard = QGuiApplication::clipboard();
  clipboard->setText(s);
}

void Window::on_pushButtonSave_clicked() {
  QString selFilter = "Text File (*.txt)";
  QString filter = selFilter;

  auto docLocations = QStandardPaths::standardLocations(
      QStandardPaths::StandardLocation::DocumentsLocation);
  QString basedir = docLocations.size() > 0 ? docLocations[0] : "";

  auto fname =
      QFileDialog::getSaveFileName(this, "Save", basedir, filter, &selFilter);
  if (fname.size() > 0) {
    QFile file(fname);
    if (file.open(QIODevice::WriteOnly)) {
      QTextStream stream(&file);
      auto s = trackedPoint2Text();
      stream << s << Qt::endl;
    } else {
      QMessageBox::critical(this, "Error",
                            QString("Error saving to %1").arg(fname));
    }
  }
}

void Window::on_horizontalSlider_valueChanged(int n) {

  if (video == nullptr)
    return; // defensive

  // the frame shown now (-1: none yet), to stay on it after an error
  const int shownFrame = _curimg.isNull() ? -1 : _curFrame;

  _curFrame = int(video->get(cv::CAP_PROP_POS_FRAMES));
  if (_curFrame != n) {
    bool b = video->set(cv::CAP_PROP_POS_FRAMES, n);
    if (!b) {
      frameError(QString("Error in video seek to frame %1").arg(n + 1),
                 shownFrame);
      return;
    }

    _curFrame = video->get(cv::CAP_PROP_POS_FRAMES);
    if (_curFrame != n) {
      frameError(QString("Error in video seek to frame %1 (at frame %2)")
                     .arg(n + 1)
                     .arg(_curFrame + 1),
                 shownFrame);
      return;
    }
  }

  const double msec = video->get(cv::CAP_PROP_POS_MSEC);

  cv::Mat frame;
  *video >> frame; // get a new frame from camera

  if (frame.empty() && n > 0) {
    // the frame count of a video is an estimate: the video ends before frame
    // n, the last frame is n - 1 (setMaximum moves the slider there, showing
    // that frame, and stops a playback); the tracked points are kept
    videoInfo.numFrames = n;
    updateVideoInfoCard();
    _curFrame = shownFrame;
    ui->horizontalSlider->setMaximum(n - 1);
    return;
  }

  if (frame.empty()) {
    frameError(QString("Error in video acquisition of frame %1").arg(n + 1),
               shownFrame);
    return;
  }

  QImage img = mat2Image(frame);

  if (img.isNull()) {
    frameError(QString("Error converting frame %1 to image").arg(n + 1),
               shownFrame);
    return;
  }

  _curimg = img.convertedTo(QImage::Format::Format_RGB32);
  _curMsec = msec;

  ui->labelVideoPos->setText(
      QString("Frame %1 / %2     %3 s")
          .arg(_curFrame + 1)
          .arg(videoInfo.numFrames)
          .arg(QLocale().toString(_curMsec / 1000, 'f', 3)));

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
    m_cameraController->setYpr(Eigen::Vector3f(-M_PI / 2, M_PI / 2, 0), false);
    m_cameraController->setPivotPosition(Eigen::Vector3f::Zero(), false);
    csVisOpenGL::OrbitCameraController::Settings enabled;
    enabled.enableRot = false;
    enabled.enableScale = true;
    enabled.enableTrasl = true;
    m_cameraController->setEnabledControls(enabled);

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
      // end of the video: stop on the last frame
      stopPlayback();
    }
  }
}

void Window::stopPlayback() {
  ui->pushButtonPlay->setChecked(false);
  setSeekEnabled(true);
  ui->pushButtonOpen->setEnabled(true);
}

// A frame cannot be shown: with a frame shown, stay on it (the video and the
// tracked points are kept); without, the video is closed.
void Window::frameError(const QString &message, int shownFrame) {
  if (shownFrame < 0) {
    QMessageBox::critical(this, "Error", message);
    resetNoVideoData();
    noVideoUpdate();
    return;
  }
  if (ui->pushButtonPlay->isChecked()) {
    stopPlayback();
  }
  _curFrame = shownFrame;
  ui->horizontalSlider->blockSignals(true);
  ui->horizontalSlider->setValue(shownFrame);
  ui->horizontalSlider->blockSignals(false);
  QMessageBox::critical(this, "Error", message);
}

void Window::on_pushButtonRotateLeft_clicked() {
  if (_curimg.isNull())
    return;

  auto ypr = m_cameraController->getYprDest();
  ypr(0) -= M_PI / 2;
  _rotated = _rotated + 1;
  _rotated = _rotated % 4;
  m_cameraController->setYpr(ypr, true);

  updateRotateCoord();
  on_pushButtonFit_clicked();
}

void Window::on_pushButtonRotateRight_clicked() {
  if (_curimg.isNull())
    return;

  auto ypr = m_cameraController->getYprDest();
  ypr(0) += M_PI / 2;
  _rotated = _rotated - 1;
  if (_rotated < 0)
    _rotated = 3;
  m_cameraController->setYpr(ypr, true);

  updateRotateCoord();
  on_pushButtonFit_clicked();
}

void Window::on_imgDoubleClick(QMouseEvent &e) {
  if (_curimg.isNull())
    return;
  if (ui->pushButtonPlay->isChecked()) {
    // during the playback a double-click stops it (no point added)
    stopPlayback();
    return;
  }

  // Get clicking position in OpenGL screen coordinates ([-1, 1], [-1, 1])
  const auto &camera = ui->preview->getCamera();

  Eigen::Vector2f clickPos =
      Eigen::Vector2f(e.pos().x(), camera.getViewSize().y() - 1 - e.pos().y());
  // normalize to -1:1
  clickPos.x() /= camera.getViewSize().x();
  clickPos.y() /= camera.getViewSize().y();
  clickPos = (clickPos * 2) - Eigen::Vector2f::Ones();

  // Compute texture coordinates assuming (-1, -1) is the top-left corner
  Eigen::Vector2f world2DCoords =
      (camera.getViewProjection().inverse() *
       Eigen::Vector4f(clickPos.x(), clickPos.y(), 0, 1))
          .head<2>();

  double ir = double(videoInfo.width) / double(videoInfo.height);
  if (world2DCoords.y() < -1.0)
    return;
  if (world2DCoords.y() > 1.0)
    return;
  if (world2DCoords.x() < -ir)
    return;
  if (world2DCoords.x() > +ir)
    return;

  // a point of this frame at the same pixel (the displayed integer image
  // coordinates) already exists: nothing to add
  const auto imgCoord = toImageCoord(world2DCoords);
  if (const auto it = _pointsPerFrame.find(_curFrame);
      it != _pointsPerFrame.end()) {
    for (const auto &p : it->second.points) {
      const auto ic = toImageCoord(p);
      if (int(ic.x()) == int(imgCoord.x()) &&
          int(ic.y()) == int(imgCoord.y())) {
        return;
      }
    }
  }

  _pointsPerFrame[_curFrame].points.push_back(world2DCoords);
  if (_pointsPerFrame[_curFrame].points.size() == 1) {
    _pointsPerFrame[_curFrame].timestamp = _curMsec / 1000;
    // add root element
    TreeWidgetItem *treeItem = new TreeWidgetItem(ui->treeWidget);
    treeItem->setText(0, QString("%1").arg(_curFrame + 1));
    treeItem->setData(0, Qt::UserRole, _curFrame);
    treeItem->setData(0, Qt::UserRole + 1, -1);
    treeItem->setText(1, QLocale().toString(_curMsec / 1000, 'f', 3));
    treeItem->setExpanded(true);
    setDeleteIcon(treeItem, "Delete all the points of this frame");
    _itemRootPerFrame[_curFrame] = treeItem;
  }
  // add this element
  TreeWidgetItem *childItem = new TreeWidgetItem(nullptr);
  childItem->setData(0, Qt::UserRole, _curFrame);
  childItem->setData(
      0, Qt::UserRole + 1,
      static_cast<int>(_pointsPerFrame[_curFrame].points.size()) - 1);

  childItem->setText(2, QString("%1").arg(int(imgCoord.x())));
  childItem->setText(3, QString("%1").arg(int(imgCoord.y())));
  setDeleteIcon(childItem, "Delete this point");
  _itemRootPerFrame[_curFrame]->addChild(childItem);

  // update visualization
  vis->showPoints(_pointsPerFrame[_curFrame].points);
}

void Window::on_pushButtonFit_clicked() {
  if (_curimg.isNull())
    return;
  const auto &camera = ui->preview->getCamera();

  // The image is [-ir, ir] x [-1, 1] in world coordinates; the view shows
  // [-scale * sr, scale * sr] x [-scale, scale]. Rotated by 90 degrees the
  // image is [-1, 1] x [-ir, ir] on screen. The fit is the smallest scale
  // showing the whole image.
  auto viewSize = camera.getViewSize();
  float sr = float(viewSize.x()) / float(viewSize.y());
  float ir = float(_curimg.width()) / float(_curimg.height());
  float scale =
      (_rotated % 2 == 0) ? std::max(1.0f, ir / sr) : std::max(ir, 1.0f / sr);

  m_cameraController->setScale(scale, false);
  m_cameraController->setScaleLimits(scale / 4.0f, scale * 4.0f);
  m_cameraController->setPivotPosition(Eigen::Vector3f::Zero(), false);
}

void Window::on_pushButtonPlay_clicked() {

  if (ui->pushButtonPlay->isChecked()) {
    setSeekEnabled(false);
    ui->pushButtonOpen->setEnabled(false);
    if (ui->horizontalSlider->value() == ui->horizontalSlider->maximum()) {
      // stopped on the last frame: play again from the start (setValue shows
      // frame 0 and continues the playback)
      ui->horizontalSlider->setValue(0);
    } else {
      on_horizontalSlider_valueChanged(ui->horizontalSlider->value());
    }
  } else {
    setSeekEnabled(true);
    ui->pushButtonOpen->setEnabled(true);
  }
}

void Window::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item,
                                             int column) {
  if (ui->pushButtonPlay->isChecked())
    return;
  int curFrame = item->data(0, Qt::UserRole).toInt();
  ui->horizontalSlider->setValue(curFrame);
}

// the delete icon of a row of the points list: delete that point or frame
void Window::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column) {
  if (column == kDeleteColumn) {
    deleteItem(item);
  }
}

void Window::setDeleteIcon(QTreeWidgetItem *item, const QString &toolTip) {
  item->setIcon(kDeleteColumn, style()->standardIcon(QStyle::SP_TrashIcon));
  item->setToolTip(kDeleteColumn, toolTip);
}

// Delete a point (a child row) or all the points of a frame (a frame row),
// asking first when it is not the current frame or it is a whole frame.
void Window::deleteItem(QTreeWidgetItem *item) {
  if (ui->pushButtonPlay->isChecked())
    return;
  if (item == nullptr)
    return;
  int curFrame = item->data(0, Qt::UserRole).toInt();

  // check if it is not current frame
  if (curFrame != ui->horizontalSlider->value()) {
    auto ans =
        QMessageBox::question(this, "Confirm",
                              "You are deleting a point that is not in current "
                              "frame, are you sure you want to continue?",
                              QMessageBox::Yes | QMessageBox::No);
    if (ans == QMessageBox::No) {
      return;
    }
    ui->horizontalSlider->setValue(curFrame);
  }

  int curId = item->data(0, Qt::UserRole + 1).toInt();
  if (curId == -1) {
    auto ans =
        QMessageBox::question(this, "Confirm",
                              "You are deleting ALL points in this frame, are "
                              "you sure you want to continue?",
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
    _pointsPerFrame[curFrame].points.erase(
        _pointsPerFrame[curFrame].points.begin() + curId);

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
  if (const auto it = _pointsPerFrame.find(_curFrame);
      it != _pointsPerFrame.end()) {
    vis->showPoints(it->second.points);
  } else {
    vis->showPoints({});
  }
}

void Window::setSeekEnabled(bool enabled) {
  ui->horizontalSlider->setEnabled(enabled);
  ui->pushButtonPrev->setEnabled(enabled);
  ui->pushButtonNext->setEnabled(enabled);
}

void Window::updatePlayButton(bool playing) {
  ui->pushButtonPlay->setIcon(style()->standardIcon(
      playing ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay));
  ui->pushButtonPlay->setText(playing ? "Pause" : "Play");
}

void Window::on_pushButtonPrev_clicked() {
  ui->horizontalSlider->setValue(ui->horizontalSlider->value() - 1);
}

void Window::on_pushButtonNext_clicked() {
  ui->horizontalSlider->setValue(ui->horizontalSlider->value() + 1);
}

void Window::on_pushButtonAbout_clicked() {
  AboutDialog dialog(this);
  dialog.exec();
}
