
#include "ace.h"


Drive chassis(
	// Left Chassis Ports (negative port will reverse it!)
	{ PORT_CHASSIS_L_F,  PORT_CHASSIS_L_B }

	// Right Chassis Ports (negative port will reverse it!)
	,
	{ PORT_CHASSIS_R_F, PORT_CHASSIS_R_B }

	// IMU Port
	,
	PORT_IMU

	// Wheel Diameter (Remember, 4" wheels are actually 4.125!)
	,
	4.125

	// Cartridge RPM
	,
	200

	// External Gear Ratio (MUST BE DECIMAL)
	,
	1.25);
// partner controller
pros::Controller partner(pros::E_CONTROLLER_PARTNER);

namespace ace
{

	/* ========================================================================== */
	/*                         Global Variable Definitions                        */
	/* ========================================================================== */

	bool partner_connected = false;
	bool is_red_alliance = false;


	/* ========================================================================== */
	/*                              Class Definitions                             */
	/* ========================================================================== */

	/* --------------------------- Custom Motor Class --------------------------- */
	void A_Motor::init()
	{
		if (!has_init)
		{
			has_init = true;

			set_encoder_units(MOTOR_ENCODER_DEGREES);
		}
	}
	float A_Motor::get_temp()
	{
		init();
		return util::cel_to_faren(get_temperature());
	}
	void A_Motor::spin_percent(float percent)
	{
		init();

		if (std::abs(percent) <= 5)
		{
			move_voltage(0);
			return;
		}

		if (get_gearing() == MOTOR_GEARSET_06)
		{
			move_velocity(percent * 6.0f);
		}
		else if (get_gearing() == MOTOR_GEARSET_18)
		{
			move_velocity(percent * 2.0f);
		}
		else if (get_gearing() == MOTOR_GEARSET_36)
		{
			move_velocity(percent);
		}
		else
		{
			printf("ERROR CARTRIDGE NOT FOUND");
		}
	}
	float A_Motor::get_percent_velocity()
	{
		init();
		if (get_gearing() == MOTOR_GEARSET_06)
		{
			return get_actual_velocity() / 6.0f;
		}
		else if (get_gearing() == MOTOR_GEARSET_18)
		{
			return get_actual_velocity() / 2.0f;
		}
		else if (get_gearing() == MOTOR_GEARSET_36)
		{
			return get_actual_velocity() / 1.0f;
		}
		else
		{
			printf("ERROR CARTRIDGE NOT FOUND");
			return 0.0f;
		}
	}
	float A_Motor::get_percent_torque()
	{
		init();
		if (get_gearing() == MOTOR_GEARSET_06)
		{
			return get_torque() * 6.0f / 2.1f * 100.0f;
		}
		else if (get_gearing() == MOTOR_GEARSET_18)
		{
			return get_torque() * 2.0f / 2.1f * 100.0f;
		}
		else if (get_gearing() == MOTOR_GEARSET_36)
		{
			return get_torque() * 1.0f / 2.1f * 100.0f;
		}
		else
		{
			printf("ERROR CARTRIDGE NOT FOUND");
			return 0.0f;
		}
	}

	/* --------------------------- Custom Button Class -------------------------- */

	// Constructor with one btn
	Btn_Digi::Btn_Digi(pros::controller_digital_e_t btn_assign, cntr_t is_master)
	{

		mode = is_master;
		btn_master = btn_assign;
		btn_partner = btn_assign;
	};
	// get whether button pressed
	bool Btn_Digi::get_press()
	{

		if (mode == cntr_both)
		{

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
	bool Btn_Digi::get_press_new()
	{

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

	// reset motors to 0 voltage
	void reset_motors()
	{


	}


	/* -------------------------------------------------------------------------- */
	/*                              Controller Stuffs                             */
	/* -------------------------------------------------------------------------- */

	bool new_haptic_request = false;
	std::string cntr_haptic_text = "";

	bool cntr_task_init = false;

	void update_cntr_task()
	{

		bool draw_master = false;
		int curr_line = 0;

		while (1)
		{

			partner_connected = partner.is_connected();

			if (new_haptic_request)
			{
				if (new_haptic_request_is_master)
				{
					master.rumble(cntr_haptic_text.c_str());
				}
				else
				{
					partner.rumble(cntr_haptic_text.c_str());
				}
				new_haptic_request = false;
			}

			/* ------------------------------ Update Screen ----------------------------- */
			else if (!draw_master)
			{
				partner.set_text(curr_line, 0, cntr_partner_text_arr[curr_line]);
			}
			else
			{
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

	std::string cntr_compile_string(std::vector<std::string> arr)
	{

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

	void update_cntr_text(cntr_t cntr, u_int8_t row, std::string text)
	{

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

	bool new_haptic_request_is_master = false;

	void update_cntr_haptic(std::string new_haptic, bool is_master)
	{
		new_haptic_request = true;
		new_haptic_request_is_master = is_master;
		cntr_haptic_text = new_haptic;
	}
}
