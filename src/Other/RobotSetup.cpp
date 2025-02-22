#include "robokauz/PRELUDE.hpp"
#include "robokauz/COMMON.hpp"
#include "robokauz/ROBOT.hpp"
#include "robokauz/PURE_PURSUIT.hpp"
#include "robokauz/Autonomous/IMUOdometry.hpp"
#include "robokauz/Other/TaskGate.hpp"

//Acquire the controller for global use
pros::Controller master (CONTROLLER_MASTER);

//Initialize drivetrain motors
//TODO specify gearsets?
pros::Motor right_back (12, true);
pros::Motor right_middle (15);
pros::Motor right_front (16, true);
pros::Motor left_back (19);
pros::Motor left_middle (6, true);
pros::Motor left_front (9);

//Initialize manipulator motors/pneumatics
pros::Motor arm_motor (5);
pros::Motor conveyor_motor (8);
pros::ADIDigitalOut back_piston ({{11 , 'E'}});
pros::ADIDigitalOut clamp_piston ('G');
pros::ADIDigitalOut lock_piston ({{11, 'F'}});

//Initialize sensors
pros::IMU inertial_sensor (14);
pros::ADIEncoder left_encoder ('A', 'B', true);
pros::ADIEncoder right_encoder ('C', 'D');
pros::ADIEncoder middle_encoder ('E', 'F');
pros::ADIAnalogIn potentiometer ('H'); 

//Drivetrain gear ratio constant
const double GEAR_RATIO = 60.0/84.0;

LiftController arm_controller(arm_motor, potentiometer);
ConveyorController conveyor_controller(conveyor_motor);
std::shared_ptr<DriveController> drive_controller = std::make_shared<DriveController>(DriveController({19, 6, 9}, {12, 15,16}));
std::shared_ptr<okapi::ChassisController> okapi_controller = okapi::ChassisControllerBuilder()
    .withMotors({19, 6, 9}, {12, 15, 16})
    .withDimensions({AbstractMotor::gearset::green, GEAR_RATIO}, {{3.98_in, 12_in}, imev5GreenTPR})
    .withClosedLoopControllerTimeUtil(1.0, 1.0, 250_ms)
    .build();

EncoderGroup encoders = {left_encoder, middle_encoder, right_encoder};
IMUOdometer imu_odometer(inertial_sensor, encoders, 2.875_in);

BotManager overwatch;
TaskGate driver_control_gate;

//0.25 mps minimum, ~1.4-5 mps maximum, ~2.5 (?) mps2 maximum, 11.5-12 in drive train, 4.125 wheel diameter
const RobotProperties robot_properties = {0.25_mps, 1.25_mps, 2.5_mps2, 12.0_in, 4.125_in, drive_controller};
PathManager wayfarer(robot_properties);
