/*
 * Copyright (C) 2015 Jared Boone, ShareBrained Technology, Inc.
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

#include "proc_secplus.hpp"

#include "dsp_fir_taps.hpp"

#include "event_m4.hpp"

SecplusProcessor::SecplusProcessor() {
	decim_0.configure(taps_200k_decim_0.taps, 33554432);
	decim_1.configure(taps_200k_decim_1.taps, 131072);
}

void SecplusProcessor::execute(const buffer_c8_t& buffer) {
	/* 2.56MHz, 2048 samples */

	const auto decim_0_out = decim_0.execute(buffer, dst_buffer);
	const auto decimator_out = decim_1.execute(decim_0_out, dst_buffer);

	/* 320kHz, 256 samples */
	feed_channel_stats(decimator_out);

	/* Decimate by 32 by averaging. */
	for(size_t i=0; i<decimator_out.count; i+=channel_decimation, current_sample++) {
		auto sum = 0.0f;
		for(size_t j=i; j<(i+channel_decimation); j++) {
			const auto real = decimator_out.p[j].real();
			const auto imag = decimator_out.p[j].imag();
			const auto mag_sq = real * real + imag * imag;
			const auto mag = __builtin_sqrtf(mag_sq);
			sum += mag;
		}

		const auto sample = sum * (1.0f / 32768.0f / channel_decimation);
		if ((last_sample < threshold) && (threshold <= sample)) {
			last_rise = current_sample;
		} else if ((last_sample >= threshold) && (threshold > sample)) {
			const auto on_samples = current_sample - last_rise;
			process_symbol(on_samples);
		}
		if ((current_sample - last_rise) > (3.25e-3 * channel_sample_rate)) {
			buffer_items = 0;
		}
		last_sample = sample;
	}
}

void SecplusProcessor::process_symbol(const size_t on_samples) {
	if (on_samples < (0.35e-3 * channel_sample_rate)) {
		buffer_items = 0;
	} else if (on_samples < (0.75e-3 * channel_sample_rate)) {
		buffer[buffer_items] = 0;
		buffer_items++;
	} else if (on_samples < (1.25e-3 * channel_sample_rate)) {
		buffer[buffer_items] = 1;
		buffer_items++;
	} else if (on_samples < (1.75e-3 * channel_sample_rate)) {
		buffer[buffer_items] = 2;
		buffer_items++;
	} else {
		buffer_items = 0;
	}

	if (buffer_items == 21) {
		process_buffer();
		buffer_items = 0;
	}
}

void SecplusProcessor::process_buffer() {
	if (buffer[0] == 0) {
		std::copy(buffer.begin() + 1, buffer.end(), pair.begin());
		pair_items = 20;
	} else if ((pair_items == 20) && (buffer[0] == 2)) {
		std::copy(buffer.begin() + 1, buffer.end(), pair.begin() + pair_items);
		pair_items = 40;
	}

	if ((pair_items == 40) && (pair != last_pair)) {
		const SecplusPacketMessage message { Timestamp::now(), pair };
		shared_memory.application_queue.push(message);
		last_pair = pair;
	}
}

int main() {
	EventDispatcher event_dispatcher { std::make_unique<SecplusProcessor>() };
	event_dispatcher.run();
	return 0;
}
