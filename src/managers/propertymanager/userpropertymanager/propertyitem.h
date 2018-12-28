#pragma once

#include <memory>
#include <QListWidgetItem>
#include "properties/property.h"

namespace omm
{

class Property;
class PropertyItem : public QListWidgetItem
{
public:
  explicit PropertyItem();
  void set_property_type(const std::string& type);

  template<typename PropertyT> PropertyT& property() const
  {
    return static_cast<PropertyT&>(*m_property);
  }

  Property& property() const
  {
    return *m_property;
  }

  std::string property_type() const;
  QVariant data(int role) const override;
  static constexpr auto UNNAMED_PROPERTY_PLACEHOLDER = "<unnamed property>";

private:
  std::unique_ptr<Property> m_property;

};

}  // namespace omm