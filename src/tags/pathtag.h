#pragma once

#include "tags/tag.h"
#include <Qt>

namespace omm
{

class PathTag : public Tag::Registrar<PathTag>
{
public:
  explicit PathTag(Object& owner);
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "PathTag");
  static constexpr auto PATH_REFERENCE_PROPERTY_KEY = "path";
  static constexpr auto ALIGN_REFERENCE_PROPERTY_KEY = "align";
  static constexpr auto POSITION_PROPERTY_KEY = "t";
  std::unique_ptr<Tag> clone() const override;
  void evaluate() override;
  Flag flags() const override;
};

}  // namespace omm
