#pragma once

#include "objects/abstractproceduralpath.h"

namespace omm
{

class Scene;

class Ellipse : public AbstractProceduralPath
{
public:
  explicit Ellipse(Scene* scene);
  std::string type() const override;
  static constexpr auto TYPE = "Ellipse";
  std::unique_ptr<Object> clone() const override;
  BoundingBox bounding_box() override;
  Flag flags() const override;

private:
  std::vector<Point> points() override;
  bool is_closed() const override;
};

}  // namespace omm
