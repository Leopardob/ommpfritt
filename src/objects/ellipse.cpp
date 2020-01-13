#include "objects/ellipse.h"

#include <QObject>
#include "properties/floatproperty.h"
#include "objects/path.h"
#include "properties/vectorproperty.h"
#include "properties/integerproperty.h"
#include "properties/boolproperty.h"
#include "scene/scene.h"

namespace omm
{

class Style;

Ellipse::Ellipse(Scene* scene) : AbstractProceduralPath(scene)
{
  static const auto category = QObject::tr("ellipse");
  create_property<FloatVectorProperty>(RADIUS_PROPERTY_KEY, Vec2f(100.0, 100.0) )
    .set_label(QObject::tr("r")).set_category(category);
  create_property<IntegerProperty>(CORNER_COUNT_PROPERTY_KEY, 12)
    .set_range(3, IntegerProperty::highest_possible_value)
    .set_label(QObject::tr("n")).set_category(category);
  create_property<BoolProperty>(SMOOTH_PROPERTY_KEY, true)
    .set_label(QObject::tr("smooth")).set_category(category);
  update();
}

QString Ellipse::type() const { return TYPE; }
std::unique_ptr<Object> Ellipse::clone() const { return std::make_unique<Ellipse>(*this); }
bool Ellipse::is_closed() const { return true; }

std::vector<Point> Ellipse::points() const
{
  const auto n_raw = property(CORNER_COUNT_PROPERTY_KEY)->value<int>();
  const auto n = static_cast<std::size_t>(std::max(3, n_raw));
  const auto r = property(RADIUS_PROPERTY_KEY)->value<Vec2f>();
  const bool smooth = property(SMOOTH_PROPERTY_KEY)->value<bool>();
  std::vector<Point> points;
  points.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    const double theta = i * 2.0/n * M_PI;
    const double x = std::cos(theta) * r.x;
    const double y = std::sin(theta) * r.y;
    if (smooth) {
      const Vec2f d(std::sin(theta) * r.x, -std::cos(theta) * r.y);
      points.push_back(Point(Vec2f(x, y), d.arg(), 2.0 * d.euclidean_norm()/n));
    } else {
      points.push_back(Point(Vec2f(x, y)));
    }
  }
  return points;
}

Flag Ellipse::flags() const
{
  return Object::flags() | Flag::Convertible | Flag::IsPathLike;
}

void Ellipse::on_property_value_changed(Property *property)
{
  if (   property == this->property(RADIUS_PROPERTY_KEY)
      || property == this->property(CORNER_COUNT_PROPERTY_KEY)
      || property == this->property(SMOOTH_PROPERTY_KEY))
  {
    update();
  } else {
    AbstractProceduralPath::on_property_value_changed(property);
  }
}

}  // namespace omm
