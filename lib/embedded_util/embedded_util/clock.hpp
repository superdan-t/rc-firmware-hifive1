#pragma once

#include <functional>
#include <vector>

#include <embedded_util/frequency.hpp>

class Clock {
	public:
		virtual Frequency get_frequency() = 0;

		/// Add a function that will run when the clock changes
		void add_frequency_change_listener(const std::function<void(Frequency)>& callback) {
			callbacks.emplace_back(callback);
		}
	
	protected:

		void emit_frequency_change(Frequency new_frequency) {
			for (auto& cbk : callbacks) {
				cbk(new_frequency);
			}
		}

	private:
		std::vector<std::function<void(Frequency)>> callbacks;

};
