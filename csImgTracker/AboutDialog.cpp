#include "AboutDialog.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QtGlobal>

namespace {

const char *kProjectUrl = "https://github.com/cscosine/csImgTracker";
const char *kReadmeQtUrl = "https://github.com/cscosine/csImgTracker#replacing-the-qt-libraries";
const char *kLicensesUrl = "https://github.com/cscosine/csImgTracker/tree/main/licenses";

// licenses/ of the deployment (next to bin/), when it exists
QString licensesDir() {
  const QDir dir(QCoreApplication::applicationDirPath() + "/../licenses");
  return dir.exists() ? dir.absolutePath() : QString();
}

QString link(const QString &url, const QString &text) {
  return QString("<a href=\"%1\">%2</a>").arg(url, text.toHtmlEscaped());
}

} // namespace

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("About csImgTracker");

  auto *icon = new QLabel;
  icon->setPixmap(QApplication::windowIcon().pixmap(64, 64));
  icon->setAlignment(Qt::AlignTop);

  auto *title = new QLabel(QString("<b style=\"font-size:14pt\">csImgTracker "
                                   "%1</b><br>%2")
                               .arg(QCoreApplication::applicationVersion(), link(kProjectUrl, kProjectUrl)));
  title->setTextFormat(Qt::RichText);
  title->setOpenExternalLinks(true);
  title->setTextInteractionFlags(Qt::TextBrowserInteraction);

  auto *header = new QHBoxLayout;
  header->addWidget(icon);
  header->addWidget(title, 1);

  // the same list as licenses/THIRD-PARTY-NOTICES.md: keep them aligned
  const QString components = "<p>Mark points on the frames of a video and export their "
                             "coordinates.<br>&copy; 2026 cscosine, released under the MIT "
                             "License.</p>"
                             "<p><b>Built with</b></p>"
                             "<table cellspacing=\"0\" cellpadding=\"2\">"
                             "<tr><td>" +
                             link("https://www.qt.io", "Qt") + " " + QT_VERSION_STR +
                             "</td><td>LGPL-3.0</td></tr>"
                             "<tr><td>" +
                             link("https://opencv.org", "OpenCV") +
                             " 5.0</td><td>Apache-2.0</td></tr>"
                             "<tr><td>" +
                             link("https://eigen.tuxfamily.org", "Eigen") +
                             " 5.0</td><td>MPL-2.0</td></tr>"
                             "<tr><td>" +
                             link("https://github.com/jeremy-rifkin/libassert", "libassert") + ", " +
                             link("https://github.com/jeremy-rifkin/cpptrace", "cpptrace") +
                             "</td><td>MIT</td></tr>"
                             "<tr><td>" +
                             link("https://github.com/davea42/libdwarf-code", "libdwarf") +
                             " (Linux, macOS)</td><td>LGPL-2.1</td></tr>"
                             "<tr><td>" +
                             link("https://github.com/facebook/zstd", "zstd") +
                             " (Linux, macOS)</td><td>BSD-3-Clause</td></tr>"
                             "<tr><td>" +
                             link("https://github.com/cscosine/csfoundation", "csfoundation") +
                             "</td><td>MIT</td></tr>"
                             "<tr><td>Big Buck Bunny (sample video)</td><td>CC BY 3.0, &copy; "
                             "Blender Foundation</td></tr>"
                             "</table>";

  const QString qt =
      QString("<p>csImgTracker uses Qt under the GNU LGPL version 3, as "
              "separate libraries you can replace (running: Qt %1): %2. "
              "libdwarf is linked into the executable under the GNU LGPL "
              "version 2.1: the source of csImgTracker, to rebuild it with a "
              "modified libdwarf, is at the address above.</p>"
              "<p>The licenses, notices and texts: the <i>licenses</i> folder "
              "of the application, also %3.</p>")
          .arg(qVersion(), link(kReadmeQtUrl, "how to replace the Qt libraries"), link(kLicensesUrl, "on GitHub"));

  auto *body = new QLabel(components + qt);
  body->setTextFormat(Qt::RichText);
  body->setWordWrap(true);
  body->setOpenExternalLinks(true);
  body->setTextInteractionFlags(Qt::TextBrowserInteraction);
  body->setMinimumWidth(460);

  auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
  QObject::connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  auto *licenses = buttons->addButton("Licenses folder", QDialogButtonBox::ActionRole);
  licenses->setToolTip("Open the licenses folder of the application (the "
                       "licenses on GitHub when it is not there)");
  QObject::connect(licenses, &QPushButton::clicked, this, [] {
    const QString dir = licensesDir();
    QDesktopServices::openUrl(dir.isEmpty() ? QUrl(kLicensesUrl) : QUrl::fromLocalFile(dir));
  });

  auto *aboutQt = buttons->addButton("About Qt", QDialogButtonBox::ActionRole);
  QObject::connect(aboutQt, &QPushButton::clicked, this, [] { QApplication::aboutQt(); });

  auto *layout = new QVBoxLayout(this);
  layout->addLayout(header);
  layout->addWidget(body);
  layout->addWidget(buttons);
  layout->setSizeConstraint(QLayout::SetFixedSize);
}
