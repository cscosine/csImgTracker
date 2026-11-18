// Render an SVG with the SVG renderer of Qt (the one of the application) to
// PNG files of the given sizes, and optionally to an ICO file with all of them:
//   render_svg <svg> <output folder> [--ico <ico file>] <size>...
// writes <output folder>/<size>.png and the ICO.
//
// The ICO entries are 32-bit bitmaps (BGRA and the AND mask), also the 256 px
// one (valid since Windows Vista): a PNG-compressed entry, as ImageMagick
// writes for 256 px, is decoded as garbage by the ICO loader of gdk-pixbuf
// (the file managers and image viewers of Linux).
#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>
#include <QtEndian>

#include <cstdio>
#include <vector>

namespace {

void append16(QByteArray &b, quint16 v) {
  v = qToLittleEndian(v);
  b.append(reinterpret_cast<const char *>(&v), 2);
}

void append32(QByteArray &b, quint32 v) {
  v = qToLittleEndian(v);
  b.append(reinterpret_cast<const char *>(&v), 4);
}

// a BITMAPINFOHEADER, the pixels bottom-up (BGRA), the AND mask (1:
// transparent), rows of the mask padded to 32 bits
QByteArray iconBitmap(const QImage &image) {
  const QImage img = image.convertToFormat(QImage::Format_ARGB32);
  const int w = img.width();
  const int h = img.height();
  const int maskStride = ((w + 31) / 32) * 4;

  QByteArray b;
  append32(b, 40);                 // header size
  append32(b, quint32(w));         // width
  append32(b, quint32(2 * h));     // height: pixels and mask
  append16(b, 1);                  // planes
  append16(b, 32);                 // bits per pixel
  append32(b, 0);                  // no compression
  append32(b, quint32(w * h * 4)); // size of the pixels
  append32(b, 0);                  // horizontal resolution
  append32(b, 0);                  // vertical resolution
  append32(b, 0);                  // colors used
  append32(b, 0);                  // important colors

  for (int y = h - 1; y >= 0; --y) {
    const auto *line = reinterpret_cast<const QRgb *>(img.constScanLine(y));
    for (int x = 0; x < w; ++x) {
      const QRgb p = line[x];
      b.append(char(qBlue(p)));
      b.append(char(qGreen(p)));
      b.append(char(qRed(p)));
      b.append(char(qAlpha(p)));
    }
  }
  for (int y = h - 1; y >= 0; --y) {
    const auto *line = reinterpret_cast<const QRgb *>(img.constScanLine(y));
    QByteArray row(maskStride, '\0');
    for (int x = 0; x < w; ++x) {
      if (qAlpha(line[x]) == 0) {
        row[x / 8] = char(row[x / 8] | (0x80 >> (x % 8)));
      }
    }
    b.append(row);
  }
  return b;
}

bool writeIco(const QString &path, const std::vector<QImage> &images) {
  QByteArray dir;
  QByteArray data;
  append16(dir, 0); // reserved
  append16(dir, 1); // type: icon
  append16(dir, quint16(images.size()));
  quint32 offset = 6 + 16 * quint32(images.size());
  for (const QImage &img : images) {
    const QByteArray bmp = iconBitmap(img);
    dir.append(char(img.width() >= 256 ? 0 : img.width()));
    dir.append(char(img.height() >= 256 ? 0 : img.height()));
    dir.append(char(0)); // colors in the palette
    dir.append(char(0)); // reserved
    append16(dir, 1);    // planes
    append16(dir, 32);   // bits per pixel
    append32(dir, quint32(bmp.size()));
    append32(dir, offset);
    offset += quint32(bmp.size());
    data.append(bmp);
  }
  QFile f(path);
  if (!f.open(QIODevice::WriteOnly)) {
    return false;
  }
  return f.write(dir + data) == dir.size() + data.size();
}

} // namespace

int main(int argc, char **argv) {
  QGuiApplication app(argc, argv);
  if (argc < 4) {
    std::fprintf(stderr, "usage: render_svg <svg> <output folder> "
                         "[--ico <ico file>] <size>...\n");
    return 2;
  }
  QSvgRenderer svg{QString(argv[1])};
  if (!svg.isValid()) {
    std::fprintf(stderr, "invalid svg %s\n", argv[1]);
    return 1;
  }
  int first = 3;
  QString ico;
  if (QString(argv[3]) == "--ico" && argc > 5) {
    ico = argv[4];
    first = 5;
  }

  std::vector<QImage> images;
  for (int i = first; i < argc; ++i) {
    const int s = QString(argv[i]).toInt();
    QImage img(s, s, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    svg.render(&p);
    p.end();
    if (!img.save(QString("%1/%2.png").arg(argv[2]).arg(s))) {
      std::fprintf(stderr, "cannot write the %d px png\n", s);
      return 1;
    }
    images.push_back(img);
  }
  if (!ico.isEmpty() && !writeIco(ico, images)) {
    std::fprintf(stderr, "cannot write %s\n", qPrintable(ico));
    return 1;
  }
  return 0;
}
