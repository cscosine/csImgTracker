#pragma once

#include <QDialog>

// About csImgTracker: the project, its version and license, the components it
// is built with and their licenses (as licenses/THIRD-PARTY-NOTICES.md), Qt
// under the LGPL (the version running, how to replace it), the licenses
// folder of the deployment and About Qt.
class AboutDialog : public QDialog {
  Q_OBJECT

public:
  explicit AboutDialog(QWidget *parent = nullptr);
};
