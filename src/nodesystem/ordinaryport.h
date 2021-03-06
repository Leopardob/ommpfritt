#pragma once

#include "properties/property.h"
#include "nodesystem/port.h"

namespace omm
{

template<PortType port_type_> class OrdinaryPort : public ConcretePortSelector<port_type_>::T
{
public:
  using BasePort = typename ConcretePortSelector<port_type_>::T;
  OrdinaryPort(Node& node, std::size_t index, const QString& label)
    : BasePort(PortFlavor::Ordinary, node, index)
    , m_label(label)
  {
  }

  QString data_type() const override;
  QString label() const override { return m_label; }
  void set_label(const QString& label) { m_label = label; }

private:
  QString m_label;
};

}  // namespace omm
