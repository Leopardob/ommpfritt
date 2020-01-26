#pragma once

#include "managers/manager.h"
#include "managers/itemmanager.h"
#include "managers/stylemanager/stylelistview.h"
#include "keybindings/commandinterface.h"

namespace omm
{

class StyleManager : public Manager::Registrar<StyleManager, ItemManager<StyleListView>>
{
  Q_OBJECT
public:
  explicit StyleManager(Scene& scene);

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "StyleManager");
  QString type() const override;
  bool perform_action(const QString& action_name) override;
};

}  // namespace omm
