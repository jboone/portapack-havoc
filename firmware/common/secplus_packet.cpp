/*
 * Copyright (C) 2014 Jared Boone, ShareBrained Technology, Inc.
 *
 * This file is part of PortaPack.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "secplus_packet.hpp"

#include "crc.hpp"

#include <cstdlib>

namespace secplus {

Packet Packet::decode(const Timestamp timestamp, const std::array<uint8_t, 40> code) {
	int64_t rolling = 0;
	int64_t fixed = 0;
	int64_t acc = 0;

	for (size_t i=0; i<40; i+=2) {
		if (i <= 20) {
			acc = 0;
		}

		const int64_t code_0 = code[i];
		const auto digit_0 = code_0;
		rolling = (rolling * 3) + digit_0;
		acc += digit_0;

		const int64_t code_1 = code[i+1];
		const auto digit_1 = (code_1 - acc) % 3;
		fixed = (fixed * 3) + digit_1;
		acc += digit_1;
	}

	return Packet {
		timestamp,
		rolling,
		fixed
	};
}

} /* namespace secplus */
