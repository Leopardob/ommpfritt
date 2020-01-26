#include "managers/nodemanager/node.h"

namespace omm
{

class MathNode : public Node
{
  Q_OBJECT
public:
  explicit MathNode(NodeModel& model);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "MathNode");
  QString type() const override { return TYPE; }

  static constexpr auto A_PROPERTY_KEY = "a";
  static constexpr auto B_PROPERTY_KEY = "b";
  static constexpr auto OPERATION_PROPERTY_KEY = "op";

  QString output_data_type(const OutputPort& port) const override;
  bool accepts_input_data_type(const QString& type, const InputPort& port) const override;
  QString title() const override;
  static StaticNodeInfo static_info();

private:
  OutputPort* m_result_port;
};

}  // namespace omm
