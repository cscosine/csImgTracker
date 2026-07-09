#include "Window.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[]) {
  Q_INIT_RESOURCE(vis);

  QApplication app(argc, argv);

  Window window(nullptr);
  app.exec();

  return 0;
}
