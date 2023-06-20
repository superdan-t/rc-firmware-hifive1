#pragma once

#include <util/logger.hpp>

/// Stream for the default UART serial interface through Freedom Metal BSP
///
/// This is meant to be a temporary class that will be replaced once the custom UART driver is available
class MetalUartStream /*: public BasicOutStream<uint8_t>*/ {
	public:
		void write(std::basic_string_view<uint8_t> data) {
			char terminatedBuf[2];
			terminatedBuf[1] = '\0';

			for (auto c : data) {
				terminatedBuf[0] = static_cast<char>(c);
				printf(terminatedBuf);
			}
		}

};

MetalUartStream& operator<<(MetalUartStream& stream, std::basic_string_view<uint8_t> data) {
	stream.write(data);
	return stream;
}

MetalUartStream& operator<<(MetalUartStream& stream, std::string_view str_view) {
	stream.write(std::basic_string_view(reinterpret_cast<const uint8_t*>(str_view.data()), str_view.size()));
	return stream;
}
