
#ifndef ACE_CGHS
#define ACE_CGHS

#include "main.h"

using std::string;

// Partner Controller Declaration
extern pros::Controller partner;

/* ========================================================================== */
/*                                Device Ports                                */
/* ========================================================================== */

/* --------------------------------- Chassis -------------------------------- */
#define PORT_CHASSIS_L_F -4
#define PORT_CHASSIS_L_B -2
#define PORT_CHASSIS_R_F 3
#define PORT_CHASSIS_R_B 1

/* --------------------------------- Motors --------------------------------- */

/* ---------------------------------- Other --------------------------------- */
#define PORT_IMU 20



/* ========================================================================== */
/*                             Main Ace Namespace                             */
/* ========================================================================== */
namespace ace {

	/* ----------------------- Util Namespace Declarations ---------------------- */
	namespace util {

		// Celsius to Farenheit
		static float cel_to_faren(float celsius);
		// Farenheit to Celsius
		static float faren_to_cel(float farenheit);
		// Bool to String
		static std::string bool_to_str(bool input);

		/* ------------------------------- Timer Class ------------------------------ */
		class timer {
			public:
			// maximum time, when curr >= max timer is done
			float maxTime = 0;
			// current time
			float currTime = 0;
			// Constructor
			timer(float max_time) {
				maxTime = max_time;
				currTime = 0;
			}
			// Update timer with elapsed time since past call
			void update(float updateTime) {
				currTime += updateTime;
			}
			// returns bool whether is done or not
			bool done() {
				return currTime >= maxTime;
			}
			// resets timer
			void reset() {
				currTime = 0;
			}
		};
	}


	/* ========================================================================== */
	/*                              Global Variables                              */
	/* ========================================================================== */

	/* ------------------------------ Miscellaneous ----------------------------- */

	// Const rad2, for 45 degree travel in auton
	const double rad2 = 1.4142;

	// Controller type enum
	enum cntr_t {
		cntr_master = 1,
		cntr_partner = 2,
		cntr_both = 3
	};
	// bool whether
	extern bool partner_connected;
	static std::vector<std::string> cntr_master_text_arr = { "", "", "", "" };
	static std::vector<std::string> cntr_partner_text_arr = { "", "", "", "" };

	extern std::string cntr_haptic_text;
	extern bool new_haptic_request;
	extern bool new_haptic_request_is_master;

	extern bool is_red_alliance;

	/* --------------------------- Custom Motor Class --------------------------- */
	class A_Motor : public pros::Motor {
		public:
		using Motor::Motor;
		bool has_init = false;
		// DO NOT CALL
		void init();
		// returns temperature in american units
		float get_temp();
		// spins motor at % rpms
		void spin_percent(float percent);
		float get_percent_velocity();
		float get_percent_torque();
	};

	/* --------------------------- Custom Button Class -------------------------- */
	class Btn_Digi {
		public:

		pros::controller_digital_e_t btn_master;
		pros::controller_digital_e_t btn_partner;
		cntr_t mode;

		// Constructor with one btn
		Btn_Digi(pros::controller_digital_e_t btn_assign, cntr_t is_master);
		// get whether button pressed
		bool get_press();
		// get whether new button press
		bool get_press_new();
	};

	/* ========================================================================== */
	/*                      Device Declaration / Definitions                      */
	/* ========================================================================== */


	/* ------------------------- Other Motors / Devices ------------------------- */


	/* ========================================================================== */
	/*                                   Buttons                                  */
	/* ========================================================================== */

	/* --------------------------------- Master --------------------------------- */


	/* ---------------------------------- Both ---------------------------------- */

	// Custom Button to Cycle Auton	
	static Btn_Digi btn_auton(pros::E_CONTROLLER_DIGITAL_X, cntr_both);

	// Custom Button to switch alliance 
	static Btn_Digi btn_alliance(pros::E_CONTROLLER_DIGITAL_A, cntr_both);

	/* ========================================================================== */
	/*                                SPEEEEEEEEED                                */
	/* ========================================================================== */

	// Chassis Speeds ( * 1.27 to fit in range of [-127, 127])
	const float DRIVE_SPEED = 87.0 * 1.27; // 87
	const float DRIVE_SPEED_INTAKE = 25.0 * 1.27;
	const float TURN_SPEED = 71.0 * 1.27; // 71
	const float TURN_SPEED_SLOW = 45.0 * 1.27;

	/* ========================================================================== */
	/*                            Function Declarations                           */
	/* ========================================================================== */

	/* --------------------------------- Standby -------------------------------- */

	/**
	 * @brief 	resets motors when called
	 *
	 */
	extern void reset_motors();


	/* ========================================================================== */
	/*                           Controller Screen Task                           */
	/* ========================================================================== */

	/**
	 * @brief	updates controller text buffers at given row
	 *
	 * @param cntr	controller to display on, from cntr_t enum
	 * @param row	row (0-2) at which to draw text
	 * @param text	std::string text that wants to be drawn
	 */
	extern void update_cntr_text(cntr_t cntr, u_int8_t row, std::string text);

	/**
	 * @brief takes in new haptic text to be rumbled on next frame
	 *
	 * @param new_haptic the haptic text ("-", ".") to rumble. see controller.rumble()
	 */
	extern void update_cntr_haptic(std::string new_haptic, bool is_master = true);

	/**
	 * @brief	compiles controller string arrays into a single string separated by newline; mainly for internal use
	 *
	 * @param arr	array to compile
	 * @return		string in stated format
	 */
	extern std::string cntr_compile_string(std::vector<std::string> arr);

	/**
	 * @brief	function that runs every 50ms and updates controller screen
	 *
	 */
	extern void update_cntr_task();

	// init bool
	extern bool cntr_task_init;


	// Actual pros::Task for controller update screen function
	static pros::Task __task_update_cntr_task(update_cntr_task, "cntr_update");

}


/* ========================================================================== */
/*                             Ace Auton Namepace                             */
/* ========================================================================== */
namespace ace::auton {

	/* ------------------------------- Autonomous ------------------------------- */

	static std::vector<std::string> auton_selection = {
		"skills", "score", "contact"
	};
	extern int auton_selection_index;

	extern void skills();
	extern void score();
	extern void contact();

	/**
		 * @brief 	pages up auton control int by one
		 *
		 */
	extern void auton_page_up();

	/**
	 * @brief 	pages down auton control int by one
	 *
	 */
	extern void auton_page_down();

	/**
	 * @brief 	spins roller by x relative degrees; autonomously
	 *
	 * @param rollerDegrees degrees to spin by
	 */
	extern void drive_chassis(float distance, float speed, bool wait = true);

	extern void turn_chassis(float distance, float speed, bool wait = true);

}


/* ========================================================================== */
/*                             Ace Util Namespace                             */
/* ========================================================================== */
namespace ace::util {

	/* ========================================================================== */
	/*                            Function Declarations                           */
	/* ========================================================================== */

	/* -------------------------- Celsius To Farenheit -------------------------- */
	static float cel_to_faren(float celsius) {
		return (float)((celsius * 9.0 / 5.0) + 32.0);
	}

	/* -------------------------- Farenheit To Celsius -------------------------- */
	static float faren_to_cel(float farenheit) {
		return (float)((farenheit - 32.0) * 5.0 / 9.0);
	}

	/* ----------------------------- Bool To String ----------------------------- */
	static std::string bool_to_str(bool input) {
		if (input)
			return "y";
		else
			return "n";
	}
};



#endif