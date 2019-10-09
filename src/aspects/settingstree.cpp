#include "aspects/settingstree.h"
#include <QFile>
#include <QSettings>
#include "logging.h"

namespace omm
{

SettingsTree::SettingsTree(const std::string filename)
{
  if (!load_from_file(filename)) {
    LERROR << "Failed to read file '" << filename << "'.";
    LFATAL("Failed to read default settings file.");
  }
}

SettingsTree::~SettingsTree()
{

}

void SettingsTree::save_in_qsettings(const std::string& q_settings_group) const
{
  QSettings settings;
  settings.beginGroup(QString::fromStdString(q_settings_group));
  for (const std::unique_ptr<SettingsTreeGroupItem>& group : m_groups) {
    settings.beginGroup(QString::fromStdString(group->name));
    for (const std::unique_ptr<SettingsTreeValueItem>& value : group->values) {
      settings.setValue(QString::fromStdString(value->name),
                        QString::fromStdString(value->value()));
    }
    settings.endGroup();
  }
  settings.endGroup();
}

void SettingsTree::load_from_qsettings(const std::string& q_settings_group)
{
  const auto settings_group = QString::fromStdString(q_settings_group);
  beginResetModel();
  QSettings settings;
  if (settings.childGroups().contains(settings_group)) {
    settings.beginGroup(settings_group);
    for (std::unique_ptr<SettingsTreeGroupItem>& group : m_groups) {
      settings.beginGroup(QString::fromStdString(group->name));
      for (std::unique_ptr<SettingsTreeValueItem>& value_item : group->values) {
        const QString key = QString::fromStdString(value_item->name);
        if (settings.contains(key)) {
          const QString value = settings.value(key).toString();
          value_item->set_value(value.toStdString());
        } else {
          // keep default sequence
        }
      }
      settings.endGroup();
    }
    settings.endGroup();
  }
  endResetModel();
}

bool SettingsTree::save_to_file(const std::string& filename) const
{
  QFile file(QString::fromStdString(filename));
  if (file.open(QIODevice::WriteOnly)) {
    LERROR << "failed to open file '" << filename << "'.";
    return false;
  }

  const auto write = [&file](const std::string& line) {
    file.write(QString::fromStdString(line).toUtf8() + "\n");
  };

  for (auto&& group : m_groups) {
    write("[" + group->name + "]");
    for (auto&& value : group->values) {
      write(value->name + ":" + value->value());
    }
  }
  return true;
}

bool SettingsTree::load_from_file(const std::string& filename)
{
  const bool insert_mode = m_groups.empty();
  QFile file(QString::fromStdString(filename));
  if (!file.open(QIODevice::ReadOnly)) {
    LERROR << "Failed to open file '" << filename << "'.";
    return false;
  }

  static const QRegExp context_regexp("\\[\\w+\\]");
  std::string group_name = "";

  QTextStream stream(&file);
  while (!stream.atEnd()) {
  line_loop:
    QString line = stream.readLine();
    line = line.trimmed();
    if (line.startsWith("#") || line.isEmpty()) {
      continue;  // comment
    }

    if (context_regexp.exactMatch(line)) {
      group_name = line.mid(1, line.size() - 2).toStdString();
    } else if (!group_name.empty()) {
      const auto tokens = line.split(":");
      if (tokens.size() != 2) {
        LWARNING << "ignoring line '" << line.toStdString()
                     << "'. Expected format: <name>: <key sequence>.";
        continue;
      }
      const auto name = tokens[0].trimmed().toStdString();
      const auto value = tokens[1].trimmed().toStdString();
      auto git = std::find_if(m_groups.begin(), m_groups.end(),
                              [group_name](const std::unique_ptr<SettingsTreeGroupItem>& group)
      {
        return group->name == group_name;
      });

      SettingsTreeGroupItem* group = nullptr;
      if (git == m_groups.end()) {
        if (insert_mode) {
          m_groups.push_back(std::make_unique<SettingsTreeGroupItem>(group_name));
        } else {
          LWARNING << "Ignore unexpected group '" << group_name << "'.";
          goto line_loop;
        }
        group = m_groups.back().get();
      } else {
        group = git->get();
      }

      const auto vit = std::find_if(group->values.begin(), group->values.end(),
                                    [name](const std::unique_ptr<SettingsTreeValueItem>& value_item)
      {
        return value_item->name == name;
      });

      if (vit != group->values.end()) {
        LWARNING << "Duplicate key sequence for '" << group_name << "'::'" << name << "'."
                 << "Drop '" << value << "', "
                 << "keep '" << (*vit)->value() << "'.";
        LFATAL("Duplicate key sequence.");
      } else {
        group->values.push_back(std::make_unique<SettingsTreeValueItem>(group->name, name, value));
      }
    } else {
      LWARNING << "line '" << line << "' ignored since no group is active.";
    }
  }
  return true;
}

SettingsTreeGroupItem* SettingsTree::group(const std::string& name) const
{
  const auto git = std::find_if(m_groups.begin(), m_groups.end(), [name](const auto& group) {
    return group->name == name;
  });
  if (git == m_groups.end()) {
    return nullptr;
  } else {
    return git->get();
  }
}

std::vector<SettingsTreeGroupItem*> SettingsTree::groups() const
{
  return ::transform<SettingsTreeGroupItem*>(m_groups, [](const auto& g) { return g.get(); });
}

SettingsTreeValueItem* SettingsTree::value(const std::string group_name, const std::string& key) const
{
   const auto* group = this->group(group_name);
   const auto vit = std::find_if(group->values.begin(), group->values.end(),
                                 [key](const auto& value)
   {
     return value->name == key;
   });
   return vit->get();
}

void SettingsTree::store()
{
  for (const std::unique_ptr<SettingsTreeGroupItem>& group : m_groups) {
    for (const std::unique_ptr<SettingsTreeValueItem>& value : group->values) {
      m_stored_values[group->name][value->name] = value->value();
    }
  }
}

void SettingsTree::restore()
{
  for (const std::unique_ptr<SettingsTreeGroupItem>& group : m_groups) {
    for (const std::unique_ptr<SettingsTreeValueItem>& value : group->values) {
      value->set_value(m_stored_values[group->name][value->name]);
    }
  }
}

QModelIndex SettingsTree::group_index(const std::string& group_name) const
{
  const auto git = std::find_if(m_groups.begin(), m_groups.end(), [group_name](const auto& group) {
    return group->name == group_name;
  });
  const int row = std::distance(m_groups.begin(), git);
  return index(row, 0, QModelIndex());
}

QModelIndex SettingsTree::value_index(const std::string& group_name, const std::string& key) const
{
  const QModelIndex parent_index = group_index(group_name);
  SettingsTreeGroupItem& group = *m_groups.at(parent_index.row());
  const auto vit = std::find_if(group.values.begin(), group.values.end(), [key](const auto& value) {
    return value->name == key;
  });
  const int row = std::distance(group.values.begin(), vit);
  return index(row, 0, parent_index);
}

QModelIndex SettingsTree::index(int row, int column, const QModelIndex& parent) const
{
  SettingsTreeItem* internal_pointer = nullptr;
  if (!parent.isValid()) {
    internal_pointer = m_groups.at(row).get();
  } else {
    auto* ptr = static_cast<const SettingsTreeItem*>(parent.internalPointer());
    assert (ptr->is_group());
    const auto* group = static_cast<const SettingsTreeGroupItem*>(ptr);
    internal_pointer = group->values.at(row).get();
  }
  return createIndex(row, column, internal_pointer);
}

QModelIndex SettingsTree::parent(const QModelIndex& child) const
{
  assert(child.isValid());
  if (static_cast<SettingsTreeItem*>(child.internalPointer())->is_group()) {
    return QModelIndex();
  } else {
    auto* const ptr = static_cast<SettingsTreeItem*>(child.internalPointer());
    auto* value_item  = static_cast<SettingsTreeValueItem*>(ptr);
    const auto it = std::find_if(m_groups.begin(), m_groups.end(), [&](const auto& group) {
      return group->name == value_item->group;
    });

    const int row = std::distance(m_groups.begin(), it);
    return createIndex(row, 0, m_groups.at(row).get());
  }
}

int SettingsTree::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid()) {
    SettingsTreeItem* ptr = static_cast<SettingsTreeItem*>(parent.internalPointer());
    if (ptr->is_group()) {
      return group(ptr->name)->values.size();
    } else {
      return 0;
    }
  } else {
    return m_groups.size();
  }
}

}  // namespace omm
