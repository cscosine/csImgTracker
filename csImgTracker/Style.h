#pragma once

class QApplication;

// The look of the application: the Fusion style (the same on Windows, Linux
// and macOS), a light palette (independent of the dark mode of the system) and
// the stylesheet of the widgets.
void applyAppStyle(QApplication &app);
