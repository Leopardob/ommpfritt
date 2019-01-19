#include "tools/selecttool.h"
#include <memory>
#include <algorithm>
#include "tools/handles/axishandle.h"
#include "tools/handles/circlehandle.h"
#include "tools/handles/particlehandle.h"
#include "scene/scene.h"
#include "commands/objectstransformationcommand.h"
#include "objects/path.h"
#include "menuhelper.h"
#include "mainwindow/pathmenu.h"
#include "properties/floatproperty.h"
#include "properties/optionsproperty.h"

namespace
{

template<typename ToolT, typename HandlesT>
void make_move_handles(HandlesT& handles, ToolT& tool)
{
  using Status = omm::Handle::Status;
  auto particle = std::make_unique<omm::MoveParticleHandle<ToolT>>(tool);

  auto x_axis = std::make_unique<omm::MoveAxisHandle<ToolT>>(tool);
  x_axis->set_style(Status::Active, omm::ContourStyle(omm::Color(1.0, 1.0, 1.0)));
  x_axis->set_style(Status::Hovered, omm::ContourStyle(omm::Color(1.0, 0.0, 0.0)));
  x_axis->set_style(Status::Inactive, omm::ContourStyle(omm::Color(1.0, 0.3, 0.3)));
  x_axis->set_direction({100, 0});

  auto y_axis = std::make_unique<omm::MoveAxisHandle<ToolT>>(tool);
  y_axis->set_style(Status::Active, omm::ContourStyle(omm::Color(1.0, 1.0, 1.0)));
  y_axis->set_style(Status::Hovered, omm::ContourStyle(omm::Color(0.0, 1.0, 0.0)));
  y_axis->set_style(Status::Inactive, omm::ContourStyle(omm::Color(0.3, 1.0, 0.3)));
  y_axis->set_direction({0, 100});

  handles.push_back(std::move(particle));
  handles.push_back(std::move(x_axis));
  handles.push_back(std::move(y_axis));
}

template<typename ToolT, typename HandlesT>
void make_rotate_handle(HandlesT& handles, ToolT& tool)
{
  using Status = omm::Handle::Status;
  auto rh = std::make_unique<omm::RotateHandle<ToolT>>(tool);
  rh->set_style(Status::Active, omm::ContourStyle(omm::Color(1.0, 1.0, 1.0)));
  rh->set_style(Status::Hovered, omm::ContourStyle(omm::Color(0.0, 0.0, 1.0)));
  rh->set_style(Status::Inactive, omm::ContourStyle(omm::Color(0.3, 0.3, 1.0)));
  rh->set_radius(100);

  handles.push_back(std::move(rh));
}

}  // namespace

namespace omm
{

// TODO 1) implement a kd-tree or similar to get the closest object fast
//      2) I guess it's more intuitive when items can be selected by clicking anywhere inside
//          its area.
//      3) improve mouse pointer icon
//      4) select during mouse move (like drawing)


template<typename PositionVariant> SelectTool<PositionVariant>::SelectTool(Scene& scene)
  : Tool(scene)
  , position_variant(scene)
{
  this->template add_property<FloatProperty>(RADIUS_PROPERTY_KEY, 20.0)
    .set_label(QObject::tr("radius").toStdString())
    .set_category(QObject::tr("tool").toStdString());
  this->template add_property<OptionsProperty>(ALIGNMENT_PROPERTY_KEY)
    .set_options({ "global", "local" })
    .set_label(QObject::tr("Alignment").toStdString())
    .set_category(QObject::tr("tool").toStdString());
}

template<typename PositionVariant>
ObjectTransformation SelectTool<PositionVariant>::transformation() const
{
  ObjectTransformation transformation;
  transformation.translate(position_variant.selection_center());
  if (this->property(ALIGNMENT_PROPERTY_KEY).template value<size_t>() == 1) {
    if (scene.object_selection().size() == 1) {
      const auto* lonely_object = *scene.object_selection().begin();
      transformation.rotate(lonely_object->global_transformation().rotation());
    }
  }
  return transformation;
}

template<typename PositionVariant> bool SelectTool<PositionVariant>
::mouse_press( const arma::vec2& pos, const QMouseEvent& event)
{
  if (!Tool::mouse_press(pos, event)) {
    position_variant.clear_selection();
  }
  return true;
}

template<typename PositionVariant>
void SelectTool<PositionVariant>::on_scene_changed()
{
  this->handles.clear();
  position_variant.make_handles(this->handles, *this);
  make_rotate_handle(this->handles, *this);
  make_move_handles(this->handles, *this);
}

template<typename PositionVariant> void SelectTool<PositionVariant>
::transform_objects(ObjectTransformation t, bool tool_space)
{
  if (tool_space) { t = t.transformed(this->transformation().inverted()); }
  position_variant.transform(t);
}

template<typename PositionVariant> bool SelectTool<PositionVariant>::has_transformation() const
{
  return !position_variant.is_empty();
}

std::string SelectObjectsTool::type() const
{
  return TYPE;
}

QIcon SelectObjectsTool::icon() const
{
  return QIcon();
}

SelectPointsTool::SelectPointsTool(Scene& scene)
  : SelectTool<PointPositions>(scene)
{
  add_property<OptionsProperty>(TANGENT_MODE_PROPERTY_KEY, 0)
    .set_options({ "Mirror", "Individual" })
    .set_label(QObject::tr("tangent").toStdString())
    .set_category(QObject::tr("tool").toStdString());
}

std::string SelectPointsTool::type() const
{
  return TYPE;
}

QIcon SelectPointsTool::icon() const
{
  return QIcon();
}

PointSelectHandle::TangentMode SelectPointsTool::tangent_mode() const
{
  const auto i = property(TANGENT_MODE_PROPERTY_KEY).value<size_t>();
  return static_cast<PointSelectHandle::TangentMode>(i);
}

std::unique_ptr<QMenu> SelectPointsTool::make_context_menu(QWidget* parent)
{
  return std::make_unique<PathMenu>(scene, parent);
}

void SelectPointsTool::on_selection_changed()
{
  on_scene_changed();
}

template class SelectTool<ObjectPositions>;
template class SelectTool<PointPositions>;

}  // namespace omm