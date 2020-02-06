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


#include "secplus_rx.hpp"

#include "string_format.hpp"

#include "baseband_api.hpp"

#include "portapack.hpp"
using namespace portapack;

#include <algorithm>

namespace secplus {
namespace format {



} /* namespace format */
} /* namespace secplus */

void SecplusLogger::on_packet(const secplus::Packet& packet) {
	const auto message = to_string_dec_uint(packet.rolling()) + "/" + to_string_dec_uint(packet.fixed());
	log_file.write_entry(packet.timestamp(), message);
}	

namespace ui {

SecplusRXAppView::SecplusRXAppView(NavigationView& nav) : nav_ { nav } {
	baseband::run_image(portapack::spi_flash::image_tag_secplus);

	add_children({
		&field_rf_amp,
		&field_lna,
		&field_vga,
		&rssi,
		&field_frequency,
		&console,
	});

	target_frequency_ = initial_target_frequency;
	
	radio::enable({
		tuning_frequency(),
		sampling_rate,
		baseband_bandwidth,
		rf::Direction::Receive,
		receiver_model.rf_amp(),
		static_cast<int8_t>(receiver_model.lna()),
		static_cast<int8_t>(receiver_model.vga()),
	});
	
	field_frequency.set_value(target_frequency());
	field_frequency.set_step(receiver_model.frequency_step());
	field_frequency.on_change = [this](rf::Frequency f) {
		set_target_frequency(f);
	};
	field_frequency.on_edit = [this, &nav]() {
		// TODO: Provide separate modal method/scheme?
		auto new_view = nav.push<FrequencyKeypadView>(target_frequency());
		new_view->on_changed = [this](rf::Frequency f) {
			set_target_frequency(f);
			field_frequency.set_value(f);
		};
	};

	logger = std::make_unique<SecplusLogger>();
	if( logger ) {
		logger->append(u"secplus.txt");
	}
}

SecplusRXAppView::~SecplusRXAppView() {
	radio::disable();

	baseband::shutdown();
}

void SecplusRXAppView::focus() {
	field_frequency.focus();
}

void SecplusRXAppView::set_parent_rect(const Rect new_parent_rect) {
	View::set_parent_rect(new_parent_rect);
}

void SecplusRXAppView::on_packet(const secplus::Packet& packet) {
	if( logger ) {
		logger->on_packet(packet);
	}

	const auto message_rolling = "r" + to_string_bin(packet.rolling(), 32) + "\n";
	console.write(message_rolling);

	const auto message_fixed = "f" + to_string_bin(packet.fixed(), 32) + "\n";
	console.write(message_fixed);
}

void SecplusRXAppView::on_frequency_changed(const uint32_t new_target_frequency) {
	set_target_frequency(new_target_frequency);
}

void SecplusRXAppView::set_target_frequency(const uint32_t new_value) {
	target_frequency_ = new_value;
	radio::set_tuning_frequency(tuning_frequency());
}

uint32_t SecplusRXAppView::target_frequency() const {
	return target_frequency_;
}

uint32_t SecplusRXAppView::tuning_frequency() const {
	return target_frequency() - (sampling_rate / 4);
}


} /* namespace i */
