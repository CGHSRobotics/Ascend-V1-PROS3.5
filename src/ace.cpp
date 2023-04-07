
#include "ace.h"

Drive chassis(
	// Left Chassis Ports (negative port will reverse it!)
	{ PORT_CHASSIS_L_F, PORT_CHASSIS_L_C, PORT_CHASSIS_L_B }

	// Right Chassis Ports (negative port will reverse it!)
	, { PORT_CHASSIS_R_F, PORT_CHASSIS_R_C, PORT_CHASSIS_R_B }

	// IMU Port
	, PORT_IMU

	// Wheel Diameter (Remember, 4" wheels are actually 4.125!)
	, 4.125

	// Cartridge RPM
	, 200

	// External Gear Ratio (MUST BE DECIMAL)
	, 1
);

// master controller
pros::Controller master(pros::E_CONTROLLER_MASTER);

// partner controller
pros::Controller partner(pros::E_CONTROLLER_PARTNER);

namespace ace {

	/* ========================================================================== */
	/*                              Class Definitions                             */
	/* ========================================================================== */

	/* --------------------------- Custom Motor Class --------------------------- */
	void A_Motor::init() {
		if (!has_init) {
			has_init = true;

			set_encoder_units(MOTOR_ENCODER_DEGREES);
		}
	}
	float A_Motor::get_temp() {

		init();

		return util::cel_to_faren(get_temperature());
	}
	void A_Motor::spin_percent(float percent) {

		init();

		if (std::abs(percent) <= 5)
		{
			move_voltage(0);
			return;
		}

		if (get_gearing() == MOTOR_GEARSET_06) {
			move_velocity(percent / 100.0f * 600.0f);
		}
		else if (get_gearing() == MOTOR_GEARSET_18) {
			move_velocity(percent / 100.0f * 200.0f);
		}
		else if (get_gearing() == MOTOR_GEARSET_36) {
			move_velocity(percent / 100.0f * 100.0f);
		}
		else {
			printf("ERROR CARTRIDGE NOT FOUND");
		}
		move_velocity(percent / 100.0f * 600.0f);
	}
	float A_Motor::get_percent_velocity() {

		init();

		if (get_gearing() == MOTOR_GEARSET_06) {
			return get_actual_velocity() / 6.0f;
		}
		else if (get_gearing() == MOTOR_GEARSET_18) {
			return get_actual_velocity() / 2.0f;
		}
		else if (get_gearing() == MOTOR_GEARSET_36) {
			return get_actual_velocity() / 1.0f;
		}
		else {
			printf("ERROR CARTRIDGE NOT FOUND");
			return 0.0f;
		}
	}
	float A_Motor::get_percent_torque() {

		init();

		if (get_gearing() == MOTOR_GEARSET_06) {
			return get_torque() * 6.0f / 2.1f * 100.0f;
		}
		else if (get_gearing() == MOTOR_GEARSET_18) {
			return get_torque() * 2.0f / 2.1f * 100.0f;
		}
		else if (get_gearing() == MOTOR_GEARSET_36) {
			return get_torque() * 1.0f / 2.1f * 100.0f;
		}
		else {
			printf("ERROR CARTRIDGE NOT FOUND");
			return 0.0f;
		}
	}

	/* --------------------------- Custom Button Class -------------------------- */

	// Constructor with one btn
	Btn_Digi::Btn_Digi(pros::controller_digital_e_t btn_assign, cntr_t is_master) {

		mode = is_master;
		btn_master = btn_assign;
		btn_partner = btn_assign;

	};
	// get whether button pressed
	bool Btn_Digi::get_press() {

		if (mode == cntr_both) {

			if (partner_connected)
			{
				return partner.get_digital(btn_partner);
			}
			else
			{
				return master.get_digital(btn_master);
			}
		}
		else if (mode == cntr_partner)
		{
			if (partner_connected)
			{
				return partner.get_digital(btn_partner);
			}
			else
			{
				return false;
			}
		}
		else if (mode == cntr_master)
		{
			return master.get_digital(btn_master);
		}
		return false;
	};
	// get whether new button press
	bool Btn_Digi::get_press_new() {

		if (mode == cntr_both)
		{
			return master.get_digital_new_press(btn_master) || partner.get_digital_new_press(btn_partner);
		}
		else if (mode == cntr_partner)
		{
			if (partner_connected)
			{
				return partner.get_digital_new_press(btn_partner);
			}
			else
			{
				return false;
			}
		}
		else if (mode == cntr_master)
		{
			return master.get_digital_new_press(btn_master);
		}
		return false;
	};


	/* -------------------------------------------------------------------------- */
	/*                             User Control Stuffs                            */
	/* -------------------------------------------------------------------------- */

	// Launch disks
	void launch(float speed, bool isLong) {

		if (launcherMotor.get_actual_velocity() < (speed - LAUNCHER_SPEED_CUTOFF) * 6.0)
		{
			launcherMotor.move_velocity(600);
			return;
		}
		else {
			launcherMotor.move_velocity(speed * 600.0 / 100.0);
			intakeMotor.spin_percent(-100);
		}

	}

	// launch standby
	void launch_standby(bool enabled, float speed) {

		if (enabled)
			launcherMotor.move_voltage(12000 * (speed / 100.0));
		else
			launcherMotor.move_voltage(0);
	}

	// reset motors to 0 voltage
	void reset_motors() {
		launcherMotor.move_voltage(0);
		intakeMotor.move_voltage(0);

		launcher_standby_enabled = false;

		endgamePneumatics.set_value(false);
		flapPneumatics.set_value(false);
	}

	// toggles endgame
	void endgame_toggle(bool enabled) {
		if (enabled) {
			endgame_timer.reset();
			endgamePneumatics.set_value(1);
			return;
		}
		else {
			if (endgame_timer.done())
			{
				endgamePneumatics.set_value(0);
				return;
			}

			endgame_timer.update(20);
			endgamePneumatics.set_value(1);
		}
	}

	// toggles flap
	void flap_toggle(bool enabled) {
		if (enabled)
		{
			flapPneumatics.set_value(1);
		}
		else {
			flapPneumatics.set_value(0);
		}
	}

	void roller_forward() {
		intakeMotor.spin_percent(ROLLER_SPEED);
	}

	void roller_reverse() {
		intakeMotor.spin_percent(-ROLLER_SPEED);
	}

	void intake_toggle() {
		intakeMotor.spin_percent(INTAKE_SPEED);
	}

	void intake_reverse() {
		intakeMotor.spin_percent(-INTAKE_SPEED);
	}

	/* ------------------------------ Vision Sensor ----------------------------- */
	double theta = 0;
	void auto_target(bool enabled) {
		pros::vision_object_s_t goal = visionSensor.get_by_sig(0, 1);

		theta = (((double)(goal.x_middle_coord) / ((double)VISION_FOV_WIDTH / 2.0)) * 30.0) + auto_target_angle_adjustment;

		if (enabled && std::abs(theta) > 1 && std::abs(theta) <= 30)
		{
			chassis.reset_gyro();
			chassis.set_turn_pid(theta, 0.5 * 127.0);
		}
	}

	/* ------------------------------ Light Sensor ------------------------------ */
	std::vector<float> light_detection_arr = {};
	void launch_sensor_detection() {

		for (size_t i = 0; i < light_detection_arr.size(); i++)
		{
			//light_detection_arr.push_back 
		}

	}



	/* -------------------------------------------------------------------------- */
	/*                              Controller Stuffs                             */
	/* -------------------------------------------------------------------------- */

	bool new_haptic_request = false;
	std::string cntr_haptic_text = "";

	void update_cntr_task() {

		bool draw_master = false;
		int curr_line = 0;

		while (1)
		{
			if (new_haptic_request) {

				master.rumble(cntr_haptic_text.c_str());
			}

			/* ------------------------------ Update Screen ----------------------------- */
			else if (!draw_master)
			{
				partner.set_text(curr_line, 0, cntr_partner_text_arr[curr_line]);
			}
			else {
				master.set_text(curr_line, 0, cntr_master_text_arr[curr_line]);
			}

			curr_line++;
			if (curr_line > 2)
			{
				curr_line = 0;
				draw_master = !draw_master;
			}

			pros::delay(50);
		}
	}


	std::string cntr_compile_string(std::vector<std::string> arr) {

		std::string result = "";
		for (int i = 0; i <= 3; i++)
		{
			result += arr[i];
			if (i < 3)
			{
				result += "\n";
			}
		}
		return result;
	}


	void update_cntr_text(cntr_t cntr, u_int8_t row, std::string text) {

		text = text + "      ";

		// both controllers
		if (cntr == cntr_both)
		{
			cntr_partner_text_arr[row] = text;
			cntr_master_text_arr[row] = text;
		}

		//  master controller
		else if (cntr == cntr_master)
		{
			cntr_master_text_arr[row] = text;
		}

		//  partner controller
		else if (cntr == cntr_partner)
		{
			cntr_partner_text_arr[row] = text;
		}
	}


	void update_cntr_haptic(std::string new_haptic) {
		new_haptic_request = true;
		cntr_haptic_text = new_haptic;
	}
}






























