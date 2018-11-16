#pragma once

#include "objects/object.h"

namespace omm
{

class Property;
class Tag;
class Scene;

class Ellipse : public Object
{
public:
  explicit Ellipse();
  void render(AbstractRenderer& renderer) const override;
  BoundingBox bounding_box() const override;
  std::string type() const override;

private:
};

}  // namespace omm