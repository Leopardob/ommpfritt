#pragma once

#include "managers/nodemanager/nodecompiler.h"

namespace omm
{

class NodeCompilerGLSL : public NodeCompiler<NodeCompilerGLSL>
{
public:
  explicit NodeCompilerGLSL(const NodeModel& model);
  static constexpr auto language = AbstractNodeCompiler::Language::GLSL;
  static constexpr bool lazy = false;
  QString generate_header(QStringList& lines) const;
  QString start_program(QStringList& lines) const;
  QString end_program(QStringList& lines) const;
  QString compile_node(const Node& node, QStringList& lines) const;
  QString compile_connection(const OutputPort& op, const InputPort& ip, QStringList& lines) const;
  QString define_node(const QString& node_type, QStringList& lines) const;
  std::set<OutputPort*> uniform_ports() const { return m_uniform_ports; }
  static QString translate_type(const QString& type);

private:
  mutable std::set<OutputPort*> m_uniform_ports;
};

}  // namespace omm
