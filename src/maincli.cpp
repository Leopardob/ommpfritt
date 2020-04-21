#include <iostream>
#include "mainwindow/application.h"
#include "objects/view.h"
#include "mainwindow/exportdialog.h"
#include "animation/animator.h"
#include <stdio.h>
#include <memory>
#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include "logging.h"
#include "scene/scene.h"
#include "python/pythonengine.h"
#include "registers.h"

static constexpr auto FRAMENUMBER_PLACEHOLDER = '#';
static constexpr auto RESOLUTION_SEPARATOR = 'x';
static constexpr QSize DEFAULT_RESOLUTION(1000, 1000);

auto make_cmd_line_parser()
{
  auto p = std::make_unique<QCommandLineParser>();
  p->setApplicationDescription(QObject::tr("app-description"));
  p->addOptions({
    {
      { "f", "input" },
      QObject::tr("The input scene file."),
      QObject::tr("filename")
    },
    {
      { "o", "output" },
      QObject::tr("Where to save the batch renderings. Use `%1` as framenumber placeholder.")
                    .arg(QString(FRAMENUMBER_PLACEHOLDER).repeated(4)),
      QObject::tr("filename")
    },
    {
      { "y", "overwrite" },
      QObject::tr("Overwrite existing files without warning.")
    },
    {
      { "r", "resolution" },
      QObject::tr("Resolution of the rendering (optional)."),
      QObject::tr("W%1H").arg(RESOLUTION_SEPARATOR),
      QString("%1%2%3")
                    .arg(DEFAULT_RESOLUTION.width())
                    .arg(RESOLUTION_SEPARATOR)
                    .arg(DEFAULT_RESOLUTION.height())
    },
    {
      { "s", "start-frame" },
      QObject::tr("start-frame of the rendering (optional)."),
      QObject::tr("S"),
      "1"
    },
    {
      { "v", "view" },
      QObject::tr("name of the view."),
      QObject::tr("N"),
    },
    {
      { "n", "sequence-length" },
      QObject::tr("number of frames (optional)."),
      QObject::tr("N"),
      "1"
    },
  });
  p->addHelpOption();

  return p;
}

int get_int(const QCommandLineParser& args, const QString& name)
{
  bool ok = true;
  const auto value = args.value(name);
  const auto ivalue = value.toInt(&ok);
  if (!ok) {
    LERROR << QObject::tr("Expected integer argument for '%1', but got '%2'.").arg(name).arg(value);
    exit(EXIT_FAILURE);
  }
  return ivalue;
}

QSize get_size(const QCommandLineParser& args, const QString& name)
{
  const auto value = args.value(name);
  const auto tokens = value.split(RESOLUTION_SEPARATOR);
  static const auto get_int = [](const QString& s, int& i) {
    bool ok;
    i = s.toInt(&ok);
    return ok;
  };
  int w, h;
  if (tokens.size() != 2 || !get_int(tokens[0], w) || !get_int(tokens[1], h)) {
    LERROR << QObject::tr("Expected two integers, separated by '%1' for '%2', but got '%3'.").arg(value);
    exit(EXIT_FAILURE);
  }
  return QSize(w, h);
}

const omm::View& find_view(omm::Scene& scene, const QString& name)
{
  const auto all_views = ::filter_if(scene.object_tree().items(), [](const auto* c) {
    return c->type() == omm::View::TYPE;
  });
  const auto views = ::filter_if(all_views, [name](const auto* c) {
    return c->name() == name;
  });
  if (views.size() == 0) {
    LERROR << QString("View '%1' not found.").arg(name);
    const QStringList view_names = ::transform<QString, QList>(all_views, [](const auto* v) {
      return v->name();
    });
    LINFO << QString("There are %1 views in this scene:\n%2")
             .arg(view_names.size())
             .arg(view_names.join("\n"));
    exit(EXIT_FAILURE);
  } else if (views.size() > 1) {
    LWARNING << QString("View '%1' is ambiguous (%2) occurences.").arg(name).arg(views.size());
  }
  return static_cast<const omm::View&>(**views.begin());
}

QString interpolate_filename(QString fn_template, int i)
{
  const int first_match = fn_template.indexOf(FRAMENUMBER_PLACEHOLDER);
  if (first_match == -1) {
    return fn_template;
  }

  const int last_match = fn_template.lastIndexOf(FRAMENUMBER_PLACEHOLDER);
  const QString placeholder = fn_template.mid(first_match, last_match - first_match + 1);
  if (placeholder.count(FRAMENUMBER_PLACEHOLDER) != placeholder.size()) {
    LERROR << QObject::tr("Framenumber placeholder must be contiguous.");
    exit(EXIT_FAILURE);
  }

  const auto formatted_number = QString("%1").arg(i, placeholder.size(), 10, QChar('0'));
  fn_template.replace(first_match, placeholder.size(), formatted_number);
  return fn_template;
}

QString require(const QCommandLineParser& args, const QString& name)
{
  const QString value = args.value(name);
  if (value.isEmpty()) {
    LERROR << QString("<%1> must be specified.").arg(name);
    std::cout << args.helpText().toStdString();
    exit(EXIT_FAILURE);
  }
  return value;
}

int main(int argc, char* argv[])
{
  using namespace omm;
  register_everything();
  QApplication app(argc, argv);
  Application omm(app);

  auto args = make_cmd_line_parser();
  args->process(app);

  const QString scene_filename = require(*args, "input");
  const QString fn_template = require(*args, "output");
  omm.scene.load_from(scene_filename);
  const int start_frame = get_int(*args, "start-frame");
  const int n_frames = get_int(*args, "sequence-length");
  const QSize resolution = get_size(*args, "resolution");
  const View& view = find_view(omm.scene, require(*args, "view"));
  const bool force = args->isSet("overwrite");

  const auto render = [&view, resolution, fn_template, force](Animator& animator) {
    const QString filename = interpolate_filename(fn_template, animator.current());
    if (QFileInfo::exists(filename) && !force) {
      LERROR << QObject::tr("Refuse to overwrite existing file '%1'.").arg(filename);
      exit(EXIT_FAILURE);
    }
    QImage image(resolution, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::red);
    image.fill(Qt::transparent);
    ExportDialog::render(animator.scene, &view, image);
    image.save(filename);
  };

  auto& animator = omm.scene.animator();
  animator.set_current(start_frame);
  render(animator);
  for (int i = 0; i < n_frames; ++i) {
    animator.advance();
    render(animator);
  }
}

