#pragma once

#include <string_view>

/// An ostream-like data stream that is much simpler and avoids the STL's iostream
template<typename DataType>
class BasicOutStream {
	public:
		virtual void write(std::basic_string_view<DataType> data) = 0;
};

template<typename DataType>
BasicOutStream<DataType>& operator<<(BasicOutStream<DataType>& stream, std::basic_string_view<DataType> data) {
	stream.write(data);
	return stream;
}

template<typename DataType>
BasicOutStream<DataType>& operator<<(BasicOutStream<DataType>& stream, std::string_view str_view) {
	stream.write(std::basic_string_view(reinterpret_cast<const uint8_t*>(str_view.data()), str_view.size()));
	return stream;
}
