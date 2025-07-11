#pragma once
#include "csVisOpenGL/BackgroundRenderer.h"
#include "csVisOpenGL/PainterAxes.h"
#include "csVisOpenGL/SingleTextureRenderer.h"
#include "csVisOpenGL/UniformLineRenderer.h"
#include "csVisOpenGL/Visualizer.h"

#include "Common.h"

#include <Eigen/Geometry>

// #define SHOW_AXES

class Visualizer : public csVisOpenGL::Visualizer {

  csVisOpenGL::ShaderFactory* shaderFactory;

  csVisOpenGL::SingleTextureRenderer imageRenderer;
  csVisOpenGL::BackgroundRenderer bkgRenderer;

  csVisOpenGL::UniformLineRenderer pointsRederer;

#ifdef SHOW_AXES
  csVisOpenGL::PainterAxes axesRenderer;
#endif

  GLuint tid;

public:
  Visualizer();
  virtual ~Visualizer();

  void initialize(csVisOpenGL::ShaderFactory* shaderFactory) override;
  void paintBackground(const csVisOpenGL::Camera& camera) override;
  void paint(const csVisOpenGL::Camera& camera) override;
  void paintTransparent(const csVisOpenGL::Camera& camera) override;
  void paintQt(const csVisOpenGL::Camera& camera, QPainter& painter);

  void setImage(const QImage& img);
  void showPoints(const std::vector<Eigen::Vector2f>& points);
};
