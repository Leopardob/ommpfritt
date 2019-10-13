#pragma once

#include <QItemDelegate>
#include <QKeySequenceEdit>
#include <memory>
#include "widgets/styleditemview.h"
#include <QTreeView>

namespace omm
{

class AbstractPreferencesTreeViewDelegate;
class KeyBindings;

class PreferencesTreeView: public StyledItemView<QTreeView>
{
  Q_OBJECT
public:
  explicit PreferencesTreeView(QWidget* parent = nullptr);
  ~PreferencesTreeView();
  void set_model(QAbstractItemModel& key_bindings,
                 std::vector<std::unique_ptr<AbstractPreferencesTreeViewDelegate>> delegates);
  void transfer_editor_data_to_model();

protected:
  void resizeEvent(QResizeEvent* event) override;

private Q_SLOTS:
  void update_column_width();

private:
  std::vector<std::unique_ptr<AbstractPreferencesTreeViewDelegate>> m_column_delegates;
};

}  // namespace omm