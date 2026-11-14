#include "Window.h"
#include <QApplication>
#include <QMessageBox>
#include <QOpenGLContext>
#include <QOpenGLWidget>
#include <QTimer>

#include <iostream>

// --smoke: start, then after a while check that every OpenGL view got its
// context and quit (exit code 0), else exit code 1; for the automatic runs of
// the deployed application (CI, a machine without the build)
static void checkAndQuit(QApplication &app, const Window &window) {
  const auto views = window.findChildren<QOpenGLWidget *>();
  for (const QOpenGLWidget *view : views) {
    if (!view->isValid() || view->context() == nullptr) {
      std::cerr << "smoke: failed, no OpenGL context for "
                << view->objectName().toStdString() << std::endl;
      app.exit(1);
      return;
    }
  }
  const QSurfaceFormat format =
      views.isEmpty() ? QSurfaceFormat() : views.front()->format();
  std::cout << "smoke: ok, platform "
            << QApplication::platformName().toStdString() << ", "
            << views.size() << " OpenGL view(s), OpenGL "
            << format.majorVersion() << "." << format.minorVersion()
            << std::endl;
  app.exit(0);
}

int main(int argc, char *argv[]) {
  Q_INIT_RESOURCE(vis);

  QApplication app(argc, argv);

  Window window(nullptr);
  if (QApplication::arguments().contains("--smoke")) {
    QTimer::singleShot(2000, &app,
                       [&app, &window] { checkAndQuit(app, window); });
  }
  return app.exec();
}
