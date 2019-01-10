#pragma once

#include <QWidget>
#include "properties/typedproperty.h"
#include "common.h"
#include "scene/scene.h"
#include "commands/propertycommand.h"

namespace omm
{

class AbstractPropertyWidget
  : public QWidget
  , public AbstractFactory< std::string, AbstractPropertyWidget,
                            Scene&, const std::set<Property*>& >
  , public AbstractPropertyObserver
{
public:
  explicit AbstractPropertyWidget(Scene& scene, const std::set<Property*>& properties);
  virtual ~AbstractPropertyWidget();
  void on_property_value_changed(Property& property) override;

protected:
  virtual std::string label() const;
  void set_default_layout(std::unique_ptr<QWidget> other);
  std::unique_ptr<QWidget> make_label_widget() const;
  Scene& scene;
  virtual void update_edit() = 0;

private:
  const std::string m_label;
};

template<typename PropertyT>
class PropertyWidget : public AbstractPropertyWidget
{
public:
  using property_type = PropertyT;
  using value_type = typename property_type::value_type;
  explicit PropertyWidget(Scene& scene, const std::set<Property*>& properties)
    : AbstractPropertyWidget(scene, properties)
    , m_properties(properties)
  {
    for (auto&& property : m_properties) {
      property->Observed<AbstractPropertyObserver>::register_observer(*this);
    }
  }

  virtual ~PropertyWidget()
  {
    for (auto&& property : m_properties) {
      property->Observed<AbstractPropertyObserver>::unregister_observer(*this);
    }
  }

protected:
  void set_properties_value(const value_type& value)
  {
    scene.template submit<PropertiesCommand<value_type>>(m_properties, value);
  }

  auto get_properties_values() const
  {
    return ::transform<value_type>(m_properties, [](const auto* property) {
      return property->template value<value_type>();
    });
  }

  const std::set<Property*>& properties() const { return m_properties; }

private:
  std::set<Property*> m_properties;
};

void register_propertywidgets();

}  // namespace omm
