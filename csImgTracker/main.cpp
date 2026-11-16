#include "Window.h"
#include <QApplication>
#include <QMessageBox>
#include <QOpenGLContext>
#include <QOpenGLWidget>
#include <QTimer>

#include <cstdlib>
#include <iostream>

// --smoke: start, then after a while check that every OpenGL view got its
// context and end (exit code 0), else exit code 1; for the automatic runs of
// the deployed application (CI, a machine without the build). The process
// ends right after the check (std::_Exit, the output flushed): no teardown,
// which the software OpenGL of Mesa on Windows can hang at the unload of its
// threads, and nothing left to check.
static void checkAndQuit(const Window &window) {
  const auto views = window.findChildren<QOpenGLWidget *>();
  for (const QOpenGLWidget *view : views) {
    if (!view->isValid() || view->context() == nullptr) {
      std::cerr << "smoke: failed, no OpenGL context for "
                << view->objectName().toStdString() << std::endl;
      std::_Exit(1);
    }
  }
  const QSurfaceFormat format =
      views.isEmpty() ? QSurfaceFormat() : views.front()->format();
  std::cout << "smoke: ok, platform "
            << QApplication::platformName().toStdString() << ", "
            << views.size() << " OpenGL view(s), OpenGL "
            << format.majorVersion() << "." << format.minorVersion()
            << std::endl;
  std::_Exit(0);
}

int main(int argc, char *argv[]) {
  Q_INIT_RESOURCE(vis);

  QApplication app(argc, argv);

  Window window(nullptr);
  if (QApplication::arguments().contains("--smoke")) {
    QTimer::singleShot(2000, &app, [&window] { checkAndQuit(window); });
  }
  return app.exec();
}
