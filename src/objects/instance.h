#pragma once

#include "objects/object.h"
#include <Qt>
#include "logging.h"

namespace omm
{

class Property;
class Tag;
class Scene;

class Instance : public Object
{
  Q_OBJECT
public:
  explicit Instance(Scene* scene);
  Instance(const Instance& other);
  void draw_object(Painter& renderer, const Style& style, Painter::Options options) const override;
  BoundingBox bounding_box(const ObjectTransformation& transformation) const override;
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Instance");
  static constexpr auto REFERENCE_PROPERTY_KEY = "reference";
  static constexpr auto IDENTICAL_PROPERTY_KEY = "identical";
  std::unique_ptr<Object> convert() const override;
  Flag flags() const override;
  void post_create_hook() override;
  void update() override;

protected:
  void on_property_value_changed(Property *property) override;

private:
  Object* illustrated_object() const;
  Object* referenced_object() const;
  std::unique_ptr<Object> m_reference;
  bool m_cyclic_dependency = false;
  void polish();

private Q_SLOTS:
  void update_tags();
};

}  // namespace omm
