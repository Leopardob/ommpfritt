#include <iostream>
#include "tags/tag.h"
#include "mainwindow/application.h"
#include "objects/view.h"
#include "mainwindow/exportdialog.h"
#include "animation/animator.h"
#include <stdio.h>
#include <memory>
#include <QApplication>
#include <QFileInfo>
#include "logging.h"
#include "scene/scene.h"
#include "subcommandlineparser.h"

template<typename T> const T& find(omm::Scene& scene, const QString& name)
{
  const auto type_matches = ::filter_if(scene.object_tree().items(), [](const auto* c) {
    return c->type() == T::TYPE || std::is_same_v<T, omm::Object>;
  });
  const auto name_type_matches = ::filter_if(type_matches, [name](const auto* c) {
    return c->name() == name;
  });
  if (name_type_matches.size() == 0) {
    LERROR << QString("%1 '%2' not found.").arg(T::TYPE).arg(name);
    const QStringList view_names = ::transform<QString, QList>(type_matches, [](const auto* v) {
      return v->name();
    });
    LINFO << QString("There are %1 objects of type [%2] in this scene:\n%3")
             .arg(view_names.size())
             .arg(T::TYPE)
             .arg(view_names.join("\n"));
    exit(EXIT_FAILURE);
  } else if (name_type_matches.size() > 1) {
    LWARNING << QString("%1 '%2' is ambiguous (%3) occurences.")
                .arg(T::TYPE).arg(name).arg(name_type_matches.size());
  }
  return static_cast<const omm::View&>(**name_type_matches.begin());
}

QString interpolate_filename(QString fn_template, int i)
{
  const int first_match = fn_template.indexOf(omm::SubcommandLineParser::FRAMENUMBER_PLACEHOLDER);
  if (first_match == -1) {
    return fn_template;
  }

  const int last_match = fn_template.lastIndexOf(omm::SubcommandLineParser::FRAMENUMBER_PLACEHOLDER);
  const QString placeholder = fn_template.mid(first_match, last_match - first_match + 1);
  if (placeholder.count(omm::SubcommandLineParser::FRAMENUMBER_PLACEHOLDER) != placeholder.size()) {
    LERROR << QObject::tr("Framenumber placeholder must be contiguous.");
    exit(EXIT_FAILURE);
  }

  const auto formatted_number = QString("%1").arg(i, placeholder.size(), 10, QChar('0'));
  fn_template.replace(first_match, placeholder.size(), formatted_number);
  return fn_template;
}

void print_tree(const omm::Object& root, const QString& prefix = "")
{
  const auto tags = ::transform<QString, QList>(root.tags.items(), [](const omm::Tag* tag) {
    return tag->type();
  }).join(", ");
  const auto label = QString("%1[%2] (%3)").arg(root.type()).arg(root.name()).arg(tags);
  std::cout << prefix.toStdString() << label.toStdString() << "\n";

  for (const omm::Object* c : root.tree_children()) {
    print_tree(*c, prefix + " ");
  }
}

void prepare_scene(omm::Scene& scene, const omm::SubcommandLineParser& args)
{
  const QString& object_name = args.get<QString>("object", "");
  if (!object_name.isEmpty()) {
    const auto& object = find<omm::Object>(scene, object_name);
    for (auto other : scene.object_tree().items()) {
      if (!object.is_ancestor_of(*other)) {
        other->property(omm::Object::VISIBILITY_PROPERTY_KEY)->set(omm::Object::Visibility::Hidden);
      }
    }
    object.property(omm::Object::VISIBILITY_PROPERTY_KEY)->set(omm::Object::Visibility::Visible);
  }
}

void render(omm::Application& app, const omm::SubcommandLineParser& args)
{
  const QString scene_filename = args.get<QString>("input");
  const QString fn_template = args.get<QString>("output");
  app.scene.load_from(scene_filename);
  prepare_scene(app.scene, args);
  const int start_frame = args.get<int>("start-frame", 1);
  const int n_frames = args.get<int>("sequence-length", 1);
  const QSize resolution = args.get<QSize>("resolution");
  const omm::View& view = find<omm::View>(app.scene, args.get<QString>("view"));
  const bool force = args.isSet("overwrite");

  const auto render = [&view, resolution, fn_template, force](omm::Animator& animator) {
    const QString filename = interpolate_filename(fn_template, animator.current());
    if (QFileInfo::exists(filename) && !force) {
      LERROR << QObject::tr("Refuse to overwrite existing file '%1'.").arg(filename);
      exit(EXIT_FAILURE);
    }
    QImage image(resolution, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::red);
    image.fill(Qt::transparent);
    omm::ExportDialog::render(animator.scene, &view, image);
    image.save(filename);
  };

  auto& animator = app.scene.animator();
  animator.set_current(start_frame);
  render(animator);
  for (int i = 0; i < n_frames; ++i) {
    animator.advance();
    render(animator);
  }
}

void tree(omm::Application& app, const omm::SubcommandLineParser& args)
{
  const QString scene_filename = args.get<QString>("input");
  app.scene.load_from(scene_filename);
  print_tree(app.scene.object_tree().root());
}

int main(int argc, char* argv[])
{
  using namespace omm;
  QApplication app(argc, argv);
  Application omm(app);

  SubcommandLineParser args(argc, argv);

  using subcommand_t = std::function<void(Application&, const SubcommandLineParser&)>;
  static const std::map<QString, subcommand_t> f_map {
    { "render", &render },
    { "tree", &tree },
  };

  if (const auto it = f_map.find(args.command()); it == f_map.end()) {
    return EXIT_FAILURE;
  } else {
    it->second(omm, args);
    return EXIT_SUCCESS;
  }
}
