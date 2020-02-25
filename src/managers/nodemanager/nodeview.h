#pragma once

#include "aspects/abstractpropertyowner.h"
#include "managers/panzoomcontroller.h"
#include "cachedgetter.h"
#include <QGraphicsView>
#include "managers/range.h"

class QPainter;
class QMenu;
class QMimeData;

namespace omm
{

class NodeModel;
class InputPort;
class AbstractPort;
class Node;
class NodeModel;
class PortItem;

class NodeView : public QGraphicsView
{
  Q_OBJECT

public:
  explicit NodeView(QWidget* parent = nullptr);
  ~NodeView();
  const QFont font;
  void abort();
  void remove_selection();
  void set_model(NodeModel* model);
  NodeModel* model() const;
  QPointF node_insert_pos() const { return m_node_insert_pos; }
  void populate_context_menu(QMenu& menu) const;
  void pan_to_center();
  bool accepts_paste(const QMimeData& mime_data) const;
  void reset_scene_rect();

public Q_SLOTS:
  void copy_to_clipboard();
  void paste_from_clipboard();

protected:
  void mouseMoveEvent(QMouseEvent* event) override;
//  void dragEnterEvent(QDragEnterEvent* event) override;
//  void dropEvent(QDropEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event);
  void drawForeground(QPainter* painter, const QRectF&) override;
  void mousePressEvent(QMouseEvent* event) override;
  void resizeEvent(QResizeEvent*) override;
  void mouseDoubleClickEvent(QMouseEvent*) override;

private:
  NodeModel* m_model = nullptr;
  PanZoomController m_pan_zoom_controller;
  void draw_connection(QPainter& painter, const QPointF& in, const QPointF& out,
                       bool is_floating, bool reverse) const;
  bool can_drop(const QDropEvent& event) const;
  constexpr static auto m_droppable_kinds = Kind::Item;
  PortItem* port_item_at(const QPoint& pos) const;

  PortItem* m_tmp_connection_origin = nullptr;
  PortItem* m_tmp_connection_target = nullptr;
  InputPort* m_former_connection_target = nullptr;
  InputPort* m_about_to_disconnect = nullptr;
  bool m_aborted = false;
  QPoint m_last_mouse_position;
  QPointF m_node_insert_pos;
  QPointF m_viewport_center;
};

}  // namespace omm
