#include "tools/rotatetool.h"
#include <memory>
#include "tools/handles/circlehandle.h"
#include "scene/scene.h"
#include "commands/objectstransformationcommand.h"

namespace
{

class RotateHandle : public omm::CircleHandle
{
public:
  RotateHandle(omm::RotateTool& tool) : m_tool(tool) { }

  void mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override
  {
    Handle::mouse_move(delta, pos, allow_hover);
    if (status() == Status::Active) {
      const arma::vec2 origin = pos - delta;
      double origin_angle = atan2(origin(1), origin(0));
      double pos_angle = atan2(pos(1), pos(0));
      m_tool.transform_objects(omm::ObjectTransformation().rotated(pos_angle - origin_angle));
    }
  }

private:
  omm::RotateTool& m_tool;
};

auto make_handles(omm::RotateTool& tool)
{
  const auto make_style = [](const omm::Color& color) {
    omm::Style style;
    style.property(omm::Style::PEN_IS_ACTIVE_KEY).set(true);
    style.property(omm::Style::BRUSH_IS_ACTIVE_KEY).set(false);
    style.property(omm::Style::PEN_COLOR_KEY).set(color);
    style.property(omm::Style::PEN_WIDTH_KEY).set(2.0);
    return style;
  };

  using Status = omm::Handle::Status;
  auto rh = std::make_unique<RotateHandle>(tool);
  rh->set_style(Status::Active, make_style(omm::Color(1.0, 1.0, 1.0)));
  rh->set_style(Status::Hovered, make_style(omm::Color(0.0, 0.0, 1.0)));
  rh->set_style(Status::Inactive, make_style(omm::Color(0.3, 0.3, 1.0)));
  rh->set_radius(100);

  std::vector<std::unique_ptr<omm::Handle>> handles;
  handles.push_back(std::move(rh));
  return handles;
}

}  // namespace

namespace omm
{

RotateTool::RotateTool(Scene& scene)
  : ObjectTransformationTool(scene, make_handles(*this))
{

}

std::string RotateTool::type() const
{
  return TYPE;
}

QIcon RotateTool::icon() const
{
  return QIcon();
}

}  // namespace omm
