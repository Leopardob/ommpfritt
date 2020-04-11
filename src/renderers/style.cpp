#include "renderers/style.h"
#include "nodesystem/nodes/fragmentnode.h"
#include "nodesystem/node.h"
#include "nodesystem/propertyport.h"
#include "nodesystem/nodemodel.h"
#include <QOpenGLFunctions>
#include "scene/messagebox.h"
#include "mainwindow/application.h"
#include "managers/nodemanager/nodemanager.h"
#include "nodesystem/nodemodel.h"
#include "renderers/offscreenrenderer.h"
#include <QApplication>
#include "properties/boolproperty.h"
#include "properties/colorproperty.h"
#include "properties/floatproperty.h"
#include "properties/triggerproperty.h"
#include "renderers/styleiconengine.h"
#include "properties/optionproperty.h"
#include "objects/tip.h"
#include "scene/scene.h"
#include "properties/stringproperty.h"
#include <QOpenGLShaderProgram>

namespace {

static constexpr auto start_marker_prefix = "start";
static constexpr auto end_marker_prefix = "end";
static constexpr double default_marker_size = 2.0;
static constexpr auto default_marker_shape = omm::MarkerProperties::Shape::None;

}  // namespace

namespace omm
{

Style::Style(Scene *scene)
  : PropertyOwner(scene), NodesOwner(AbstractNodeCompiler::Language::GLSL, *scene)
  , start_marker(start_marker_prefix, *this, default_marker_shape, default_marker_size)
  , end_marker(end_marker_prefix, *this, default_marker_shape, default_marker_size)
  , m_offscreen_renderer(std::make_unique<OffscreenRenderer>())
{
  const auto pen_category = QObject::tr("pen");
  const auto brush_category = QObject::tr("brush");
  const auto decoration_category = QObject::tr("decoration");
  create_property<StringProperty>(NAME_PROPERTY_KEY, QObject::tr("<unnamed object>"))
    .set_label(QObject::tr("Name"))
    .set_category(QObject::tr("basic"));

  create_property<BoolProperty>(PEN_IS_ACTIVE_KEY, true)
    .set_label(QObject::tr("active"))
    .set_category(pen_category);
  create_property<ColorProperty>(PEN_COLOR_KEY, Colors::BLACK)
    .set_label(QObject::tr("color"))
    .set_category(pen_category);
  create_property<FloatProperty>(PEN_WIDTH_KEY, 1.0)
    .set_step(0.1)
    .set_range(0, std::numeric_limits<double>::infinity())
    .set_label(QObject::tr("width"))
    .set_category(pen_category);
  create_property<OptionProperty>(STROKE_STYLE_KEY, 0)
    .set_options({ QObject::tr("Solid"),
                   QObject::tr("Dashed"),
                   QObject::tr("Dotted"),
                   QObject::tr("DashDotted"),
                   QObject::tr("DashDotDotted") })
    .set_label(QObject::tr("Stroke Style")).set_category(pen_category);
  create_property<OptionProperty>(JOIN_STYLE_KEY, 2)
    .set_options({ QObject::tr("Bevel"),
                   QObject::tr("Miter"),
                   QObject::tr("Round") })
    .set_label(QObject::tr("Join")).set_category(pen_category);
  create_property<OptionProperty>(CAP_STYLE_KEY, 1)
    .set_options({ QObject::tr("Square"),
                   QObject::tr("Flat"),
                   QObject::tr("Round") })
    .set_label(QObject::tr("Cap")).set_category(pen_category);
  create_property<BoolProperty>(COSMETIC_KEY, true).set_label(QObject::tr("Cosmetic"))
      .set_category(pen_category);

  create_property<BoolProperty>(BRUSH_IS_ACTIVE_KEY, false)
    .set_label(QObject::tr("active"))
    .set_category(brush_category);
  create_property<ColorProperty>(BRUSH_COLOR_KEY, Colors::RED)
    .set_label(QObject::tr("color"))
    .set_category(brush_category);
  create_property<BoolProperty>("gl-brush", false)
    .set_label(tr("Use Nodes")).set_category(brush_category);

  add_property(EDIT_NODES_PROPERTY_KEY, make_edit_nodes_property())
    .set_label(QObject::tr("Edit ...")).set_category(brush_category);

  start_marker.make_properties(decoration_category);
  end_marker.make_properties(decoration_category);
  init();
}

Style::~Style()
{
}

Style::Style(const Style &other)
  : PropertyOwner(other), NodesOwner(other)
  , start_marker(start_marker_prefix, *this, default_marker_shape, default_marker_size)
  , end_marker(end_marker_prefix, *this, default_marker_shape, default_marker_size)
  , m_offscreen_renderer(std::make_unique<OffscreenRenderer>())
{
  other.copy_properties(*this, CopiedProperties::Compatible);
  init();
}

void Style::init()
{
  NodeModel& model = node_model();
  AbstractNodeCompiler& compiler = model.compiler();
  connect(&compiler, SIGNAL(compilation_succeeded(QString)), this, SLOT(set_code(QString)));
  connect(&compiler, SIGNAL(compilation_failed(QString)), this, SLOT(set_error(QString)));
}

QString Style::type() const { return TYPE; }

std::unique_ptr<Style> Style::clone() const
{
  return std::make_unique<Style>(*this);
}

Flag Style::flags() const
{
  return Flag::None | Flag::HasGLSLNodes;
}

Texture Style::render_texture(const Object& object, const QSize& size, const QRectF& roi,
                              const Painter::Options& options) const
{
  update_uniform_values();
  return m_offscreen_renderer->render(object, size, roi, options);
}

void Style::serialize(AbstractSerializer& serializer, const Serializable::Pointer& root) const
{
  PropertyOwner::serialize(serializer, root);
  node_model().serialize(serializer, make_pointer(root, NODES_POINTER));
}

void Style::deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& root)
{
  PropertyOwner::deserialize(deserializer, root);
  node_model().deserialize(deserializer, make_pointer(root, NODES_POINTER));
}

void Style::on_property_value_changed(Property *property)
{
  if (    property == this->property(PEN_IS_ACTIVE_KEY)
       || property == this->property(PEN_COLOR_KEY)
       || property == this->property(PEN_WIDTH_KEY)
       || property == this->property(STROKE_STYLE_KEY)
       || property == this->property(JOIN_STYLE_KEY)
       || property == this->property(CAP_STYLE_KEY)
       || property == this->property(COSMETIC_KEY)
       || property == this->property(BRUSH_IS_ACTIVE_KEY)
       || property == this->property(BRUSH_COLOR_KEY) )
  {
    if (Scene* scene = this->scene(); scene != nullptr) {
      Q_EMIT scene->message_box().appearance_changed(*this);
    }
  }
}

void Style::update_uniform_values() const
{
  auto& compiler = static_cast<NodeCompilerGLSL&>(node_model().compiler());
  for (AbstractPort* port : compiler.uniform_ports()) {
    assert(port->flavor == omm::PortFlavor::Property);
    const Property* property = port->port_type == PortType::Input
                                ? static_cast<PropertyInputPort*>(port)->property()
                                : static_cast<PropertyOutputPort*>(port)->property();
    if (property != nullptr) {
      m_offscreen_renderer->set_uniform(port->uuid(), property->variant_value());
    }
  }
}

void Style::set_code(const QString& code) const
{
  NodeModel& model = node_model();
  if (m_offscreen_renderer->set_fragment_shader(code)) {
    update_uniform_values();
    model.set_error("");
  } else {
    model.set_error(tr("Compilation failed"));
  }
}

void Style::set_error(const QString& error) const
{
  node_model().set_error(error);
  m_offscreen_renderer->set_fragment_shader("");
}

}  // namespace omm
