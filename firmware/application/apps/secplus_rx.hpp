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

#ifndef __SECPLUS_APP_H__
#define __SECPLUS_APP_H__

#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "ui_receiver.hpp"
#include "ui_rssi.hpp"
#include "ui_channel.hpp"

#include "ui_geomap.hpp"

#include "event_m0.hpp"

#include "log_file.hpp"

#include "secplus_packet.hpp"

#include "lpc43xx_cpp.hpp"
using namespace lpc43xx;

#include <cstdint>
#include <cstddef>
#include <string>
#include <list>
#include <utility>

#include <iterator>

class SecplusLogger {
public:
	Optional<File::Error> append(const std::filesystem::path& filename) {
		return log_file.append(filename);
	}
	
	void on_packet(const secplus::Packet& packet);

private:
	LogFile log_file { };
};

namespace ui {

class SecplusRXAppView : public View {
public:
	SecplusRXAppView(NavigationView& nav);
	~SecplusRXAppView();

	void set_parent_rect(const Rect new_parent_rect) override;

	// Prevent painting of region covered entirely by a child.
	// TODO: Add flag to View that specifies view does not need to be cleared before painting.
	void paint(Painter&) override { };

	void focus() override;

	std::string title() const override { return "Sec+"; };

private:
	static constexpr uint32_t initial_target_frequency = 315000000;
	static constexpr uint32_t sampling_rate = 2560000;
	static constexpr uint32_t baseband_bandwidth = 1750000;
	NavigationView& nav_;

	MessageHandlerRegistration message_handler_packet {
		Message::ID::SecplusPacket,
		[this](Message* const p) {
			const auto message = static_cast<const SecplusPacketMessage*>(p);
			const auto packet = secplus::Packet::decode(message->received_at, message->pair);
			this->on_packet(packet);
		}
	};

	static constexpr auto header_height = 1 * 16;

	RFAmpField field_rf_amp {
		{ 13 * 8, 0 * 16 }
	};

	LNAGainField field_lna {
		{ 15 * 8, 0 * 16 }
	};

	VGAGainField field_vga {
		{ 18 * 8, 0 * 16 }
	};

	RSSI rssi {
		{ 21 * 8, 0, 6 * 8, 4 },
	};

	Channel channel {
		{ 21 * 8, 5, 6 * 8, 4 },
	};

	FrequencyField field_frequency {
		{ 0 * 8, 0 * 16},
	};

	Console console {
		{ 0, 4 * 16, 240, 240 }
	};

	std::unique_ptr<SecplusLogger> logger { };

	uint32_t target_frequency_ = initial_target_frequency;

	void on_packet(const secplus::Packet& packet);

	void on_frequency_changed(const uint32_t new_target_frequency);

	uint32_t target_frequency() const;
	void set_target_frequency(const uint32_t new_value);

	uint32_t tuning_frequency() const;
};

} /* namespace ui */

#endif/*__SECPLUS_APP_H__*/
