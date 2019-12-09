#pragma once

#include "managers/manager.h"

namespace Ui
{
class NodeManager;
}

namespace omm
{

class NodeModel;
class NodeManager : public Manager
{
  Q_OBJECT
public:
  explicit NodeManager(Scene& scene);
  ~NodeManager();

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "NodeManager");
  QString type() const override;
  bool perform_action(const QString &name) override;
  void set_model(NodeModel* model);

private:
  std::unique_ptr<Ui::NodeManager> m_ui;
};

}  // namespace