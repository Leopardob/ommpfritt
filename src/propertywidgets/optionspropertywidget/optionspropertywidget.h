#pragma once

#include "properties/optionsproperty.h"
#include "propertywidgets/propertywidget.h"

namespace omm
{

class OptionsEdit;

class OptionsPropertyWidget
    : public AbstractPropertyWidget::Registrar<OptionsPropertyWidget, PropertyWidget<OptionsProperty>>
{
public:
  explicit OptionsPropertyWidget(Scene& scene, const std::set<Property*>& properties);

protected:
  void update_edit() override;
  QString type() const override;

private:
  OptionsEdit* m_options_edit;
};

}  // namespace omm
