#pragma once

#include "aspects/serializable.h"
#include <QObject>
#include <QTimer>
#include <set>
#include <memory>
#include "animation/fcurve.h"

namespace omm
{

class AbstractFCurve;

class Animator : public QObject, public Serializable
{
  Q_OBJECT
public:
  enum class PlayMode { Repeat, Stop };
  explicit Animator();
  void serialize(AbstractSerializer&, const Pointer&) const override;
  void deserialize(AbstractDeserializer&, const Pointer&) override;

  int start() const { return m_start_frame; }
  int end() const { return m_end_frame; }
  int current() const { return m_current_frame; }

  static constexpr auto START_FRAME_POINTER = "start-frame";
  static constexpr auto END_FRAME_POINTER = "end-frame";
  static constexpr auto CURRENT_FRAME_POINTER = "current-frame";
  static constexpr auto FCURVES_POINTER = "fcurves";
  static std::string map_property_to_fcurve_type(const std::string& property_type);
  AbstractFCurve& get_fcurve(AbstractPropertyOwner& owner, const std::string& property_key);

public Q_SLOTS:
  void set_start(int start);
  void set_end(int end);
  void set_current(int current);
  void toggle_play_pause(bool play);
  void advance();

Q_SIGNALS:
  void start_changed(int);
  void end_changed(int);
  void current_changed(int);
  void play_pause_toggled(bool);

private:
  int m_start_frame = 1;
  int m_end_frame = 100;
  int m_current_frame = 1;
  bool m_is_playing = false;
  QTimer m_timer;
  PlayMode m_play_mode = PlayMode::Repeat;

  std::set<std::unique_ptr<AbstractFCurve>> m_fcurves;

};

}  // namespace omm