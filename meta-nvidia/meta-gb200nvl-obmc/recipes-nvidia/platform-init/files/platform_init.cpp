// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (C) 2025 Nvidia
 */

#include <gpiod.hpp>

#include <cstdlib>
#include <iostream>

int main()
{
	gpiod::chip chip("gpiochip0");
	gpiod::line fpga_init = chip.find_line("FPGA_RST_L-O");
	if (!fpga_init){
		std::cerr << "unable to find FPGA_RST_L-O";
		return EXIT_FAILURE;
	}
	fpga_init.request({
		"platform_init",
		gpiod::line_request::DIRECTION_OUTPUT,
		0
	}, 1);

	::std::cin.get();

	return EXIT_SUCCESS;
}
