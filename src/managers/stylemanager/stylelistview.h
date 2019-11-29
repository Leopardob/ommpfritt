#pragma once

#include <QListView>
#include "common.h"
#include "managers/manageritemview.h"
#include "scene/stylelist.h"
#include "widgets/itemproxyview.h"

namespace omm
{

class Object;
class StyleList;
class StyleListView : public ManagerItemView<ItemProxyView<QListView>, StyleList>
{
  Q_OBJECT
public:
  explicit StyleListView(StyleList& model);
  using model_type = StyleList;
  using ManagerItemView<ItemProxyView<QListView>, model_type>::ManagerItemView;
  void mouseReleaseEvent(QMouseEvent* e) override;

protected:
  void resizeEvent(QResizeEvent* e) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

public Q_SLOTS:
  void set_selection(const std::set<Style*>& selection);
};

}  // namespace
