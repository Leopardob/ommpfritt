#pragma once

#include "managers/nodemanager/nodecompiler.h"

namespace omm
{

class NodeCompilerPython : public NodeCompiler<NodeCompilerPython>
{
public:
  explicit NodeCompilerPython(const NodeModel& model);
  static constexpr auto language = AbstractNodeCompiler::Language::Python;
  static constexpr bool lazy = true;

  QString generate_header(QStringList& lines) const;
  QString start_program(QStringList& lines) const;
  QString end_program(QStringList& lines) const;
  QString compile_node(const Node& node, QStringList& lines) const;
  QString compile_connection(const OutputPort& op, const InputPort& ip, QStringList& lines) const;
  QString define_node(const QString& node_type, QStringList& lines) const;
};

}  // namespace omm
