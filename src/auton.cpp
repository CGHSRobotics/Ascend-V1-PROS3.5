
#include "ace.h"

namespace ace::auton {

	/* ========================================================================== */
	/*                        Global Variables Definitions                        */
	/* ========================================================================== */

	int auton_selection_index = 0;

	/* ========================================================================== */
	/*                               Score Auton                                   */
	/* ========================================================================== */
	void score() {
		drive_chassis(5, DRIVE_SPEED);
	}


	/* ========================================================================== */
	/*                               Contact Auton                                  */
	/* ========================================================================== */
	void contact() {
		drive_chassis(-5, DRIVE_SPEED);
	}


	/* ========================================================================== */
	/*                                   Skills                                   */
	/* ========================================================================== */
	void skills() {


	}

	/* ========================================================================== */
	/*                         Auton Function Definitions                         */
	/* ========================================================================== */

	/* ------------------------------ Auton Page Up ----------------------------- */
	void auton_page_up() {
		auton_selection_index += 1;
		if (auton_selection_index > auton_selection.size() - 1) {
			auton_selection_index = 0;
		}
	}

	/* ----------------------------- Auton Page Down ---------------------------- */
	void auton_page_down() {
		auton_selection_index -= 1;
		if (auton_selection_index < 0) {}
		auton_selection_index = auton_selection.size() - 1;
	}

	/* --------------------------- Drive Chassis Auton -------------------------- */
	void drive_chassis(float distance, float speed, bool wait) {

		bool slew_enabled = distance >= 14;
		chassis.set_drive_pid(distance, speed, slew_enabled);
		if (wait)
		{
			chassis.wait_drive();
		}
	}

	/* --------------------------- Turn Chassis Auton --------------------------- */
	void turn_chassis(float angle, float speed, bool wait) {
		chassis.set_turn_pid(angle, speed);
		if (wait)
		{
			chassis.wait_drive();
		}
	}
}