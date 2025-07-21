#include "Visualizer.h"
#include <iostream>

#include <csVisOpenGL/Camera.hpp>
#include <csVisOpenGL/Colors.hpp>

#include <math.h>

Visualizer::Visualizer()
    : csVisOpenGL::Visualizer()
    , shaderFactory(nullptr)
    , tid(-1) {}
Visualizer::~Visualizer() {}

void Visualizer::initialize(csVisOpenGL::ShaderFactory* shaderFactory,
                            std::shared_ptr<QOpenGLExtraFunctions> const& glExtraFunctions) {
  bkgRenderer.initialize(shaderFactory, glExtraFunctions);

  {
    /*
    QImage img(2, 2, QImage::Format_RGB32);
    img.fill(Qt::red);

    tid = shaderFactory->uploadTexture(img);
    */
    imageRenderer.initialize(shaderFactory, glExtraFunctions);
    csVisOpenGL::SingleTextureRenderer::Matrix3x4f vectCoords;
    vectCoords.col(0) = Eigen::Vector3f(-1, -1, 0);
    vectCoords.col(1) = Eigen::Vector3f(+1, -1, 0);
    vectCoords.col(2) = Eigen::Vector3f(+1, +1, 0);
    vectCoords.col(3) = Eigen::Vector3f(-1, +1, 0);
    csVisOpenGL::SingleTextureRenderer::Matrix2x4f texCoords;
    texCoords.col(0) = Eigen::Vector2f(0, 0);
    texCoords.col(1) = Eigen::Vector2f(1, 0);
    texCoords.col(2) = Eigen::Vector2f(1, 1);
    texCoords.col(3) = Eigen::Vector2f(0, 1);
    // imageRenderer.setTexture(tid, vectCoords, texCoords, csVisOpenGL::SingleTextureRenderer::TextureInterpolation::Linear);

    pointsRederer.initialize(shaderFactory, glExtraFunctions);
    pointsRederer.setLineWidth(3);
    pointsRederer.setUniformColor(Eigen::Vector3f(1, 0, 0));
    pointsRederer.setLines(Eigen::Matrix3Xf());
  }

#ifdef SHOW_AXES
  axesRenderer.initialize(shaderFactory, glExtraFunctions);
  axesRenderer.setPose(Eigen::Isometry3f::Identity(), 1);
#endif

  this->shaderFactory = shaderFactory;
}

void Visualizer::paintQt(const csVisOpenGL::Camera& camera, QPainter& painter) {
  pointsRederer.draw(camera);
}

void Visualizer::paintBackground(const csVisOpenGL::Camera& camera) {
  bkgRenderer.draw(camera);
}

void Visualizer::paint(const csVisOpenGL::Camera& camera) {
#ifdef SHOW_AXES
  axesRenderer.draw(camera);
#endif
}

void Visualizer::paintTransparent(const csVisOpenGL::Camera& camera) {
  if (tid != -1) {
    imageRenderer.draw(camera);
  } else {
    int x = 32;
  }
}

void Visualizer::showPoints(const std::vector<Eigen::Vector2f>& points) {
  Eigen::Matrix3Xf ps(3, 4 * points.size());
  float dx = 0.05;
  for (int i = 0; i < points.size(); i++) {
    Eigen::Vector3f p(points[i].x(), points[i].y(), 0);
    ps.col(4 * i + 0) = p + Eigen::Vector3f(-dx, -dx, 0);
    ps.col(4 * i + 1) = p + Eigen::Vector3f(+dx, +dx, 0);
    ps.col(4 * i + 2) = p + Eigen::Vector3f(-dx, +dx, 0);
    ps.col(4 * i + 3) = p + Eigen::Vector3f(+dx, -dx, 0);
  }
  pointsRederer.setLines(ps);
}

void Visualizer::setImage(const QImage& img) {

  if (tid != -1)
    shaderFactory->releaseTexture(tid);

  if (!img.isNull()) {
    tid = shaderFactory->uploadTexture(img);

    csVisOpenGL::SingleTextureRenderer::Matrix3x4f vectCoords;
    float ratio = float(img.width()) / float(img.height());
    vectCoords.col(0) = Eigen::Vector3f(+ratio, -1, 0);
    vectCoords.col(1) = Eigen::Vector3f(-ratio, -1, 0);
    vectCoords.col(2) = Eigen::Vector3f(-ratio, +1, 0);
    vectCoords.col(3) = Eigen::Vector3f(+ratio, +1, 0);

    csVisOpenGL::SingleTextureRenderer::Matrix2x4f texCoords;
    texCoords.col(0) = Eigen::Vector2f(0, 0);
    texCoords.col(1) = Eigen::Vector2f(1, 0);
    texCoords.col(2) = Eigen::Vector2f(1, 1);
    texCoords.col(3) = Eigen::Vector2f(0, 1);
    imageRenderer.setTexture(tid, vectCoords, texCoords, csVisOpenGL::SingleTextureRenderer::TextureInterpolation::Linear);
  } else {
    tid = -1;
    imageRenderer.resetTexture();
  }
}
