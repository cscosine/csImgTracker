#pragma once

class QApplication;

// The look of the application: the Fusion style (the same on Windows, Linux
// and macOS) with a click on a slider moving it to the click, a light palette
// (independent of the dark mode of the system) and the stylesheet of the
// widgets.
void applyAppStyle(QApplication &app);
