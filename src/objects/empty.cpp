#include "objects/empty.h"
#include "properties/boolproperty.h"

namespace omm
{

Empty::Empty(Scene* scene)
  : Object(scene)
{
  static const auto category = QObject::tr("Empty");
  create_property<BoolProperty>(JOIN_PROPERTY_KEY, false)
    .set_label(QObject::tr("join")).set_category(category);
}

BoundingBox Empty::bounding_box(const ObjectTransformation &transformation) const
{
  const Vec2f o = transformation.apply_to_position(Vec2f::o());
  return BoundingBox({ o });
}

QString Empty::type() const { return TYPE; }

Geom::PathVector Empty::paths() const
{
  if (property(JOIN_PROPERTY_KEY)->value<bool>()) {
    return join(tree_children());
  } else {
    return Geom::PathVector();
  }
}

}  // namespace omm
