#pragma once

#include <string>
#include "aspects/typed.h"

namespace omm
{

class CommandInterface : virtual public Typed
{
public:
  virtual ~CommandInterface() = default;
  virtual bool perform_action(const QString& action_name) = 0;
};

}  // namespace
