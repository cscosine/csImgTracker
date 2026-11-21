#include "Style.h"

#include <QApplication>
#include <QFont>
#include <QPalette>
#include <QProxyStyle>
#include <QStyle>
#include <QStyleFactory>

namespace {

// Fusion, with a left click on a slider moving it to the click (seek), not by
// a page step.
class AppStyle : public QProxyStyle {
public:
  using QProxyStyle::QProxyStyle;

  int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
                QStyleHintReturn *returnData) const override {
    if (hint == QStyle::SH_Slider_AbsoluteSetButtons) {
      return Qt::LeftButton;
    }
    return QProxyStyle::styleHint(hint, option, widget, returnData);
  }
};

// Colors: neutral greys, one blue accent.
const char *kStyleSheet = R"(
QMainWindow, QWidget#centralwidget {
  background: #f5f6f8;
}

QWidget#sidePanel {
  background: #f5f6f8;
}

QSplitter::handle {
  background: #dfe2e7;
}

QLabel {
  color: #1f2937;
}

QLabel[heading="true"] {
  color: #6b7280;
  font-weight: 600;
  font-size: 8pt;
  padding-top: 8px;
}

QLabel#labelHint {
  color: #6b7280;
}

QLabel#labelVideoInfo {
  background: #ffffff;
  border: 1px solid #dfe2e7;
  border-radius: 6px;
  padding: 8px 10px;
  color: #374151;
}

QLabel#labelVideoPos {
  color: #374151;
  font-weight: 600;
}

/* Buttons */

QPushButton {
  background: #ffffff;
  border: 1px solid #cfd4dc;
  border-radius: 6px;
  padding: 6px 12px;
  min-height: 20px;
  color: #1f2937;
}
QPushButton:hover {
  background: #eef2ff;
  border-color: #a5b4fc;
}
QPushButton:pressed {
  background: #e0e7ff;
}
QPushButton:checked {
  background: #2563eb;
  border-color: #2563eb;
  color: #ffffff;
}
QPushButton:disabled {
  background: #f3f4f6;
  border-color: #e5e7eb;
  color: #a0a6b0;
}

QPushButton[accent="true"] {
  background: #2563eb;
  border-color: #2563eb;
  color: #ffffff;
  font-weight: 600;
  padding: 8px 12px;
}
QPushButton[accent="true"]:hover {
  background: #1d4ed8;
  border-color: #1d4ed8;
}
QPushButton[accent="true"]:pressed {
  background: #1e40af;
}
QPushButton[accent="true"]:disabled {
  background: #93c5fd;
  border-color: #93c5fd;
}

/* flat buttons (About): a link */
QPushButton:flat {
  background: transparent;
  border: none;
  color: #2563eb;
  padding: 4px 0;
}
QPushButton:flat:hover {
  color: #1d4ed8;
  text-decoration: underline;
}

QPushButton[transport="true"] {
  padding: 6px 10px;
  min-width: 20px;
}
QPushButton[transport="true"]:checked {
  background: #e0e7ff;
  border-color: #2563eb;
  color: #1e3a8a;
}

/* Points table */

QTreeWidget {
  background: #ffffff;
  alternate-background-color: #f8fafc;
  border: 1px solid #dfe2e7;
  border-radius: 6px;
  color: #1f2937;
  selection-background-color: #dbeafe;
  selection-color: #1e3a8a;
}
QTreeWidget::item {
  padding: 3px 0;
}
QHeaderView::section {
  background: #f3f4f6;
  color: #374151;
  font-weight: 600;
  border: none;
  border-bottom: 1px solid #dfe2e7;
  padding: 5px 6px;
}

/* Bottom bar with the timeline */

QWidget#transportBar {
  background: #ffffff;
  border-top: 1px solid #dfe2e7;
}

QSlider::groove:horizontal {
  height: 6px;
  background: #e5e7eb;
  border-radius: 3px;
}
QSlider::sub-page:horizontal {
  background: #2563eb;
  border-radius: 3px;
}
QSlider::handle:horizontal {
  background: #ffffff;
  border: 2px solid #2563eb;
  width: 14px;
  height: 14px;
  margin: -6px 0;
  border-radius: 9px;
}
QSlider::handle:horizontal:hover {
  background: #eef2ff;
}
QSlider::sub-page:horizontal:disabled {
  background: #cbd5e1;
}
QSlider::handle:horizontal:disabled {
  border-color: #cbd5e1;
}

QStatusBar {
  background: #ffffff;
  border-top: 1px solid #dfe2e7;
  color: #6b7280;
}
QStatusBar QLabel {
  color: #6b7280;
  padding: 0 6px;
}

QToolTip {
  background: #1f2937;
  color: #ffffff;
  border: none;
  padding: 4px 6px;
}
)";

} // namespace

void applyAppStyle(QApplication &app) {
  // the proxy owns the Fusion style (nullptr: the default style of the system)
  auto *style = new AppStyle(QStyleFactory::create("Fusion"));
  app.setStyle(style);
  // the light palette of Fusion, also when the system is in dark mode
  app.setPalette(style->standardPalette());

  QFont font = app.font();
  if (font.pointSizeF() < 10) {
    font.setPointSizeF(10);
  }
  app.setFont(font);

  app.setStyleSheet(kStyleSheet);
}
