
#include "lvgl.cpp"

/*
	Commands for stack trace:

	arm-none-eabi-addr2line -faps -e ./bin/hot.package.elf
	arm-none-eabi-addr2line -faps -e ./bin/cold.package.elf
*/


/* ========================================================================== */
/*                                 Initialize                                 */
/* ========================================================================== */
void initialize()
{

	// load lvgl loading screen
	ace::lvgl::init_lvgl();

	pros::delay(500); // Stop the user from doing anything while legacy ports configure.

	// Configure your chassis controls
	chassis.toggle_modify_curve_with_controller(false); // Enables modifying the controller curve with buttons on the joysticks
	chassis.set_active_brake(0);                        // Sets the active brake kP. We recommend 0.1.
	chassis.set_curve_default(10, 0);                   // Defaults for curve. If using tank, only the first parameter is used. (Comment this line out if you have an SD card!)
	default_constants();                                // Set the drive to your own constants from autons.cpp!
	exit_condition_defaults();                          // Set the exit conditions to your own constants from autons.cpp!

	// init flap
	lv_label_set_text(ace::lvgl::label_load_flap, "Init Flap       -  OK");

	ace::__task_update_cntr_task.set_priority(TASK_PRIORITY_DEFAULT - 1);

	lv_label_set_text(ace::lvgl::label_load_shenan, "Init Shenan     -  OK");

	// init chassis
	chassis.initialize();
	pros::lcd::shutdown();

	// Go to main screen
	lv_label_set_text(ace::lvgl::label_load_imu, "IMU Calibrate -  OK");
	ace::lvgl::start_preloader_anim();

	ace::update_cntr_haptic(".");
}

/* -------------------------------- Disabled -------------------------------- */
void disabled() {}

/* ----------------------------       ---------------------------- */
void competition_initialize() {}

/* ========================================================================== */
/*                                 Autonomous                                 */
/* ========================================================================== */
void autonomous()
{

	chassis.reset_pid_targets();               // Resets PID targets to 0
	chassis.reset_gyro();                      // Reset gyro position to 0
	chassis.reset_drive_sensor();              // Reset drive sensors to 0
	chassis.set_drive_brake(MOTOR_BRAKE_HOLD); // Set motors to hold.  This helps autonomous consistency.

	std::string curr_auton = ace::auton::auton_selection[ace::auton::auton_selection_index];

	ace::reset_motors();

	if (curr_auton == "score")
	{
		ace::auton::score();
	}
	else if (curr_auton == "contact")
	{
		ace::auton::contact();
	}
	else if (curr_auton == "skills")
	{
		ace::auton::skills();
	}
}

/* ========================================================================== */
/*                                User Control                                */
/* ========================================================================== */
void opcontrol()
{

	int i = 0;

	chassis.set_drive_brake(MOTOR_BRAKE_COAST);

bool leftarm = false;

	while (true)
	{

		/* -------------------------------- Get Input ------------------------------- */

		// Auton Page Up
		if (ace::btn_auton.get_press_new())
		{
			ace::auton::auton_page_up();
		}

		// Alliance Toggle
		if (ace::btn_alliance.get_press_new())
		{
			ace::is_red_alliance = !ace::is_red_alliance;
		}

		if (master.get_digital(DIGITAL_Y))
		{
			leftarm=true;
		}else{
			leftarm=false;
		}
		

		/* --------------------------- Chassis Tank Drive --------------------------- */
		chassis.tank();

		/* ------------------------------ User Control ------------------------------ */
		// for loop exists just so you can break from it if you want to skip all else
		for (int i = 0; i < 1; i++)
		{
			
			// Spin intake motor
			if (leftarm)
			{
				ace::Left_Arm.spin_percent(100);
				ace::Right_Arm.spin_percent(-100);

			}else{
				ace::Left_Arm.spin_percent(0);
				ace::Right_Arm.spin_percent(0);
			}

			
			

		}

		/* ------------------------- Controller Screen Draw ------------------------- */

		// Line 2
		std::string ally_str = "";
		if (ace::is_red_alliance)
		{
			ally_str = "red";
		}
		else
		{
			ally_str = "blue";
		}

		std::string auton_string = "";
		if (ace::auton::auton_selection[ace::auton::auton_selection_index] == "contact")
		{
			auton_string = "c";
		}
		else if (ace::auton::auton_selection[ace::auton::auton_selection_index] == "score")
		{
			auton_string = "sc";
		}
		else
		{
			auton_string = "sk";
		}

		ace::update_cntr_text(ace::cntr_both, 1,
			" " + auton_string +
			" " + ally_str);

		// Line 3


		/* ---------------------------------- Delay --------------------------------- */
		pros::delay(ez::util::DELAY_TIME);
	}
}

