#pragma once

#include <chrono>

#include <franka/robot.h>
#include <research_interface/rbk_types.h>
#include <research_interface/service_types.h>

#include "complete_robot_state.h"
#include "network.h"
#include "robot_control.h"

namespace franka {

class Robot::Impl : public RobotControl {
 public:
  static constexpr std::chrono::seconds kDefaultTimeout{5};

  explicit Impl(const std::string& franka_address,
                uint16_t franka_port = research_interface::kCommandPort,
                std::chrono::milliseconds timeout = kDefaultTimeout,
                RealtimeConfig realtime_config = RealtimeConfig::kEnforce);

  bool update() override;

  void controllerCommand(const research_interface::ControllerCommand&
                             controller_command) noexcept override;
  void motionGeneratorCommand(const research_interface::MotionGeneratorCommand&
                                  motion_generator_command) noexcept override;
  const RobotState& robotState() const noexcept override;
  ServerVersion serverVersion() const noexcept;
  bool motionGeneratorRunning() const noexcept;
  bool controllerRunning() const noexcept;
  RealtimeConfig realtimeConfig() const noexcept override;

  void startController() override;
  void stopController() override;

  void startMotionGenerator(
      research_interface::StartMotionGenerator::MotionGeneratorMode mode)
      override;
  void stopMotionGenerator() override;

  template <research_interface::StartMotionGenerator::MotionGeneratorMode,
            typename T>
  void control(
      std::function<T(const RobotState&)> control_callback,
      std::function<void(const T&, research_interface::MotionGeneratorCommand*)>
          conversion_callback);

  template <typename T>
  void control(
      std::function<T(const RobotState&)> control_callback,
      std::function<void(const T&, research_interface::ControllerCommand*)>
          conversion_callback);

 private:
  void handleStartMotionGeneratorResponse(
      const research_interface::StartMotionGenerator::Response& response);
  void handleStopMotionGeneratorResponse(
      const research_interface::StopMotionGenerator::Response& response);
  void handleStartControllerResponse(
      const research_interface::StartController::Response& response);
  void handleStopControllerResponse(
      const research_interface::StopController::Response& response);

  Network network_;

  const RealtimeConfig realtime_config_;
  uint16_t ri_version_;
  bool motion_generator_running_;
  bool controller_running_;

  research_interface::RobotCommand robot_command_;
  CompleteRobotState robot_state_;

  // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60970
  // taken from http://stackoverflow.com/a/24847480/249642
  struct EnumClassHash {
    template <typename T>
    size_t operator()(T t) const {
      return static_cast<size_t>(t);
    }
  };
  std::unordered_multiset<research_interface::Function, EnumClassHash>
      expected_responses_;
};

}  // namespace franka
