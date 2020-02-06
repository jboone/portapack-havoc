/*
 * Copyright (C) 1996 Thomas Sailer (sailer@ife.ee.ethz.ch, hb9jnx@hb9w.che.eu)
 * Copyright (C) 2012-2014 Elias Oenal (multimon-ng@eliasoenal.com)
 * Copyright (C) 2015 Jared Boone, ShareBrained Technology, Inc.
 * Copyright (C) 2016 Furrtek
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

#ifndef __PROC_SECPLUS_H__
#define __PROC_SECPLUS_H__

#include "baseband_processor.hpp"
#include "baseband_thread.hpp"
#include "rssi_thread.hpp"

#include "channel_decimator.hpp"

#include "clock_recovery.hpp"
#include "symbol_coding.hpp"
#include "packet_builder.hpp"
#include "baseband_packet.hpp"

#include "ook.hpp"

#include "secplus_packet.hpp"
#include "message.hpp"
#include "portapack_shared_memory.hpp"

#include <cstdint>

class SecplusProcessor : public BasebandProcessor {
public:
	SecplusProcessor();

	void execute(const buffer_c8_t& buffer) override;
	
private:
	static constexpr size_t baseband_fs = 2560000;

	BasebandThread baseband_thread { baseband_fs, this, NORMALPRIO + 20, baseband::Direction::Receive };
	RSSIThread rssi_thread { NORMALPRIO + 10 };
	
	std::array<complex16_t, 512> dst { };
	const buffer_c16_t dst_buffer {
		dst.data(),
		dst.size()
	};

	dsp::decimate::FIRC8xR16x24FS4Decim4 decim_0 { };
	dsp::decimate::FIRC16xR16x16Decim2 decim_1 { };

	static constexpr float channel_rate_in = baseband_fs / 4.0 / 2.0;
	static constexpr size_t channel_decimation = 32;
	static constexpr float channel_sample_rate = channel_rate_in / channel_decimation;

	static constexpr float threshold = 0.02f;
	float last_sample = 0.0f;
	size_t current_sample = 0;
	size_t last_rise = 0;
	std::array<uint8_t, 21> buffer;
	size_t buffer_items = 0;

	void process_symbol(const size_t on_samples);
	void process_buffer();
};

#endif/*__PROC_SECPLUS_H__*/
