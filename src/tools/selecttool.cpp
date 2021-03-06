#include "tools/selecttool.h"
#include <memory>
#include <algorithm>
#include "scene/scene.h"
#include "objects/path.h"
#include "menuhelper.h"
#include "properties/floatproperty.h"
#include "properties/optionproperty.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include <QCoreApplication>
#include "commands/objectstransformationcommand.h"
#include "commands/pointstransformationcommand.h"
#include "logging.h"
#include "properties/boolproperty.h"
#include "scene/history/historymodel.h"

namespace omm
{

// TODO 1) implement a kd-tree or similar to get the closest object fast
//      2) I guess it's more intuitive when items can be selected by clicking anywhere inside
//          their area.
//      3) improve mouse pointer icon

AbstractSelectTool::AbstractSelectTool(Scene& scene)
  : Tool(scene)

{
  const QString category = QObject::tr("tool");
  create_property<OptionProperty>(ALIGNMENT_PROPERTY_KEY, 1)
    .set_options({ QObject::tr("global"), QObject::tr("local") })
    .set_label(QObject::tr("Alignment"))
    .set_category(category)
    .set_animatable(false);

  create_property<BoolProperty>(SYMMETRIC_PROPERTY_KEY, false)
    .set_label(QObject::tr("Symmetric"))
    .set_category(category)
    .set_animatable(false);
}

void AbstractSelectTool::reset_absolute_object_transformation()
{
  m_init_position = transformation().null();
  m_last_object_transformation = ObjectTransformation();
}

bool AbstractSelectTool::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e)
{
  m_current_position = transformation().null();
  return Tool::mouse_move(delta, pos, e);
}

bool AbstractSelectTool::mouse_press(const Vec2f& pos, const QMouseEvent& e)
{
  const bool r = Tool::mouse_press(pos, e);
  reset_absolute_object_transformation();
  return r;
}

void AbstractSelectTool::mouse_release(const Vec2f &pos, const QMouseEvent &event)
{
  tool_info.clear();
  Tool::mouse_release(pos, event);
}

ObjectTransformation AbstractSelectTool::transformation() const
{
  ObjectTransformation transformation;
  transformation.translate(selection_center());
  if (this->property(ALIGNMENT_PROPERTY_KEY)->template value<size_t>() == 1) {
    if (scene()->item_selection<Object>().size() == 1) {
      const auto* lonely_object = *scene()->item_selection<Object>().begin();
      transformation.rotate(lonely_object->global_transformation(Space::Viewport).rotation());
    }
  }
  return transformation;
}

void AbstractSelectTool::draw(Painter &renderer) const
{
  Tool::draw(renderer);
  if (!tool_info.isEmpty()) {
    renderer.toast(m_current_position + Vec2f(30.0, 30.0), tool_info);
    renderer.painter->setPen(ui_color(HandleStatus::Active, "Handle", "line"));
    renderer.painter->drawLine(m_init_position.x, m_init_position.y,
                               m_current_position.x, m_current_position.y);
  }
}

void AbstractSelectTool::cancel()
{
  transform_objects(ObjectTransformation());

  Command* cmd = scene()->history().last_command();
  Tool::cancel();
  if (cmd != nullptr && cmd->is_noop()) {
    cmd->setObsolete(true);
    QSignalBlocker(&scene()->history());
    scene()->history().undo();
  }
}

}  // namespace omm
