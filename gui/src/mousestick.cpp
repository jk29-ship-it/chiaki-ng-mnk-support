// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

#include "mousestick.h"

#include <algorithm>
#include <cmath>

// Below this stick magnitude the output snaps back to center.
static const double MOUSE_STICK_EPSILON = 1e-4;
// Time step the smoothing factor refers to, in seconds.
static const double MOUSE_STICK_SMOOTHING_STEP = 0.004;

MouseStick::MouseStick(const MouseStickConfig &config)
{
	SetConfig(config);
}

void MouseStick::SetConfig(const MouseStickConfig &cfg)
{
	config = cfg;
	config.sensitivity = std::clamp(config.sensitivity, 0.001, 100.0);
	config.y_ratio = std::clamp(config.y_ratio, 0.05, 20.0);
	config.deadzone = std::clamp(config.deadzone, 0.0, 0.9);
	config.exponent = std::clamp(config.exponent, 0.2, 5.0);
	config.smoothing = std::clamp(config.smoothing, 0.0, 0.95);
}

void MouseStick::AddDelta(double dx, double dy)
{
	acc_x += dx;
	acc_y += dy;
}

void MouseStick::Reset()
{
	acc_x = acc_y = 0.0;
	vel_x = vel_y = 0.0;
	last_x = last_y = 0;
}

bool MouseStick::Tick(double dt, int16_t *out_x, int16_t *out_y)
{
	if(dt <= 0.0)
		dt = 0.001;
	// Ignore huge gaps (window was inactive, timer stalled) to avoid a burst.
	dt = std::min(dt, 0.05);

	double vx = acc_x / dt;
	double vy = acc_y / dt;
	acc_x = acc_y = 0.0;

	// Smoothing is defined per 4 ms step and scaled to the real step length, so
	// it behaves the same whether updates come from mouse input or the timer,
	// and a stopped mouse recenters the stick quickly.
	const double a = config.smoothing > 0.0 ? std::pow(config.smoothing, dt / MOUSE_STICK_SMOOTHING_STEP) : 0.0;
	vel_x = a * vel_x + (1.0 - a) * vx;
	vel_y = a * vel_y + (1.0 - a) * vy;

	// Counts per second -> fraction of full deflection.
	double mx = vel_x * config.sensitivity / 1000.0;
	double my = vel_y * config.sensitivity * config.y_ratio / 1000.0;
	if(config.invert_y)
		my = -my;

	double mag = std::hypot(mx, my);
	double ox = 0.0, oy = 0.0;
	if(mag > MOUSE_STICK_EPSILON)
	{
		double shaped = std::pow(std::min(mag, 1.0), config.exponent);
		double out = config.deadzone + (1.0 - config.deadzone) * shaped;
		out = std::min(out, 1.0);
		ox = mx / mag * out;
		oy = my / mag * out;
	}
	else
	{
		// fully stopped, drop leftover smoothing so the stick recenters cleanly
		vel_x = vel_y = 0.0;
	}

	int16_t x = (int16_t)std::lround(std::clamp(ox, -1.0, 1.0) * 32767.0);
	int16_t y = (int16_t)std::lround(std::clamp(oy, -1.0, 1.0) * 32767.0);
	bool changed = x != last_x || y != last_y;
	last_x = x;
	last_y = y;
	*out_x = x;
	*out_y = y;
	return changed;
}
