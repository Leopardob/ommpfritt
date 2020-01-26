#pragma once

#include "propertywidgets/propertywidget.h"
#include "properties/boolproperty.h"

class QLineEdit;

namespace omm
{

class CheckBox;

class BoolPropertyWidget
    : public AbstractPropertyWidget::Registrar<BoolPropertyWidget, PropertyWidget<BoolProperty>>
{
public:
  explicit BoolPropertyWidget(Scene& scene,const std::set<Property*>& properties);

protected:
  void update_edit() override;
  QString type() const override;

private:
  CheckBox* m_checkbox;
};

}  // namespace omm
