#pragma once

#include "commands/command.h"
#include "common.h"
#include "scene/objecttreecontext.h"

namespace omm
{

class CopyObjectsCommand : public Command
{
public:
  CopyObjectsCommand(Project& project, std::vector<OwningObjectTreeContext> contextes);

  void undo() override;
  void redo() override;

private:
  std::vector<OwningObjectTreeContext> m_contextes;
};

}  // namespace
