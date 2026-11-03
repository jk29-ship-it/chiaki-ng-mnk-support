// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

#ifndef CHIAKI_MOUSESTICK_H
#define CHIAKI_MOUSESTICK_H

#include <cstdint>

/**
 * Converts relative mouse movement into a right analog stick deflection.
 *
 * A mouse reports position changes, a stick sets a turn speed. Each tick the
 * accumulated mouse counts are turned into a velocity (counts per second),
 * smoothed, scaled by the sensitivity, shaped by a response curve and pushed
 * past the game's inner deadzone so that small movements are not swallowed.
 */
struct MouseStickConfig
{
	// Mouse speed (counts per second) that gives full stick deflection is 1000 / sensitivity.
	double sensitivity = 0.5;
	// Vertical speed multiplier relative to horizontal.
	double y_ratio = 1.0;
	// Game's inner deadzone to jump over, 0..0.9 of full deflection.
	double deadzone = 0.15;
	// Response curve: 1.0 linear, < 1.0 more output for small movements, > 1.0 less.
	double exponent = 1.0;
	// Exponential smoothing of the velocity, 0 = none, up to 0.95.
	double smoothing = 0.3;
	bool invert_y = false;
};

class MouseStick
{
	public:
		explicit MouseStick(const MouseStickConfig &config = MouseStickConfig());

		void SetConfig(const MouseStickConfig &config);
		const MouseStickConfig &GetConfig() const { return config; }

		/** Add raw mouse counts since the last call. */
		void AddDelta(double dx, double dy);

		/**
		 * Advance by dt seconds and compute the stick position.
		 * @return true if the output changed since the previous tick.
		 */
		bool Tick(double dt, int16_t *out_x, int16_t *out_y);

		void Reset();

	private:
		MouseStickConfig config;
		double acc_x = 0.0, acc_y = 0.0;
		double vel_x = 0.0, vel_y = 0.0;
		int16_t last_x = 0, last_y = 0;
};

#endif // CHIAKI_MOUSESTICK_H
