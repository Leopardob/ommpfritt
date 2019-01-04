#pragma once

#include "tools/objectstools/objectstool.h"
#include "objects/object.h"

namespace omm
{

class ScaleTool : public ObjectsTool
{
public:
  explicit ScaleTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "ScaleTool";
};

}  // namespace omm