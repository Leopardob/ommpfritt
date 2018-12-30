#include "mainwindow/viewport/viewport.h"

#include <glog/logging.h>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>

#include "renderers/viewportrenderer.h"
#include "scene/scene.h"

namespace
{

arma::vec2 point2vec(const QPoint& p)
{
  return arma::vec2 {
    static_cast<double>(p.x()),
    static_cast<double>(p.y()),
  };
}

void set_cursor_position(QWidget& widget, const arma::vec2& pos)
{
  auto cursor = widget.cursor();
  cursor.setPos(widget.mapToGlobal(QPoint(pos(0), pos(1))));
  widget.setCursor(cursor);
}

// coordinate system of QWidget's canvas goes top-down and left-to-right
// I think bottom-up and left-to-right is more intuitive.
const auto TOP_RIGHT = omm::ObjectTransformation().scaled({1, -1});

}  // namespace

namespace omm
{

Viewport::Viewport(Scene& scene)
  : m_scene(scene)
  , m_timer(std::make_unique<QTimer>())
  , m_pan_controller([this](const arma::vec2& pos) { set_cursor_position(*this, pos); })
  , m_viewport_transformation(TOP_RIGHT)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  connect(&*m_timer, &QTimer::timeout, [this]() {
    update();
  });
  m_timer->setInterval(30);
  m_timer->start();

  m_scene.Observed<AbstractSelectionObserver>::register_observer(*this);
  setMouseTracking(true);
}

Viewport::~Viewport()
{
  m_scene.Observed<AbstractSelectionObserver>::unregister_observer(*this);
}

#if USE_OPENGL
void Viewport::paintGL()
#else
void Viewport::paintEvent(QPaintEvent* event)
#endif
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.fillRect(rect(), Qt::gray);
  ViewportRenderer renderer(painter, m_scene);
  renderer.set_base_transformation(viewport_transformation());
  renderer.render();

  m_scene.tool_box.active_tool().draw(renderer);
}

void Viewport::mousePressEvent(QMouseEvent* event)
{
  const arma::vec2 cursor_position = point2vec(event->pos());
  m_pan_controller.init(cursor_position);

  if (event->modifiers() & Qt::AltModifier) {
    event->accept();
  }  else if (event->modifiers() == Qt::NoModifier) {
    const auto pos = viewport_transformation().inverted().apply_to_position(cursor_position);
    if (m_scene.tool_box.active_tool().mouse_press(pos))
    {
      event->accept();
    }
  } else {
    QWidget::mousePressEvent(event);
  }
}

void Viewport::mouseMoveEvent(QMouseEvent* event)
{
  const auto widget_size = arma::vec2{ static_cast<double>(width()),
                                       static_cast<double>(height()) };
  const auto cursor_position = point2vec(event->pos());
  arma::vec2 delta { 0.0, 0.0 };
  if (event->buttons() == Qt::LeftButton)
  {
    delta = m_pan_controller.delta(cursor_position, widget_size);
    if (event->modifiers() & Qt::AltModifier) {
      m_viewport_transformation.translate(delta);
      event->accept();
      return;
    }
  }

  if (event->modifiers() == Qt::NoModifier)
  {
    auto& tool = m_scene.tool_box.active_tool();
    const auto delta_ = viewport_transformation().inverted().apply_to_direction(delta);
    const auto cpos_ = viewport_transformation().inverted().apply_to_position(cursor_position);
    if (tool.mouse_move(delta_, cpos_))
    {
      event->accept();
      return;
    }
  }

  QWidget::mouseMoveEvent(event);
}

void Viewport::mouseReleaseEvent(QMouseEvent* event)
{
  m_scene.tool_box.active_tool().mouse_release();
  QWidget::mouseReleaseEvent(event);
}

void Viewport::set_selection(const std::set<AbstractPropertyOwner*>& selection)
{
  m_scene.tool_box.active_tool().set_selection(AbstractPropertyOwner::cast<Object>(selection));
}

ObjectTransformation Viewport::viewport_transformation() const
{
  const auto center = arma::vec2{ static_cast<double>(width()) / 2.0,
                                  static_cast<double>(height()) / 2.0 };
  return m_viewport_transformation.translated(center);
}

Scene& Viewport::scene() const
{
  return m_scene;
}

}  // namespace omm
