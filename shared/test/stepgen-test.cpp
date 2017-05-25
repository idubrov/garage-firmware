#include <cstdint>
#include <iostream>
#include <cmath>
#include <string>
#include "algo/stepgen.hpp"

using namespace ::std;

int main(int argc, char** argv)
{
	int microsteps = 1;
	int steps = 10;
	if (argc > 1)
	{
		steps = stoi(argv[1]);
	}
	if (argc > 2)
	{
		microsteps = stoi(argv[2]);
	}
	int stopat = -1;
	if (argc > 3)
	{
		stopat = stoi(argv[3]);
	}
	steps *= microsteps;

	int rpm = 120;
	int leadscrew = 16; // 16 TPI
	int spr = 200 * microsteps; // steps per revolution
	int thread = 8;
	int K = leadscrew * spr / thread; // steps per revolution
	int F = 1000000;
	int Fusec = F / 1000000;
	int a = 1000 * microsteps;
	int v = K * rpm / 60; // steps per sec

	::algo::stepgen::stepgen gen(F);
	if (!gen.set_acceleration(a << 8))
	{
		cout << "First step is too long!" << endl;
	}
	gen.set_target_speed(v << 8); // convert speed to 24.8
	gen.set_target_step(steps);
	uint32_t delay;
	int i = 0;

	while (true)
	{
		if (i == stopat)
		{
			cout << "Stopping" << endl;
			gen.set_target_step(0);
		}
		delay = (gen.next() + 128) >> 8;
		if (delay == 0)
		{
			break;
		}
		cout << delay << endl;

		i++;
	}

	return 0;
}
