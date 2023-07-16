#pragma once

#include <cstdint>
#include <type_traits>

/// Represents a contiguous sequence of bits within the underlying type T
template<typename T>
class BitField {
	static_assert(std::is_integral_v<T>, "BitField: underlying type must be integral");
	public:

		/// Return a BitField from the range [UPPER_BOUND, LOWER_BOUND]
		template<uint_least8_t UPPER_BOUND, uint_least8_t LOWER_BOUND>
		static constexpr BitField from_range() {
			static_assert(UPPER_BOUND > LOWER_BOUND, "BitField: the upper bound (end bit) must be greater than the lower bound (start bit)");
			return BitField(UPPER_BOUND, LOWER_BOUND);
		}

		template<uint_least8_t N>
		static constexpr BitField single_bit() {
			return BitField(N, N);
		}

		uint_least8_t get_offset() const {
			return offset;
		}

		T get_mask() const {
			return mask;
		}

	private:
		constexpr BitField(uint_least8_t upper_bound, uint_least8_t lower_bound) :
			offset(lower_bound),
			mask(((static_cast<T>(0x1) << (upper_bound - lower_bound + 1)) - 1) << offset)
		{}

		/// The amount to shift when reading/writing the field
		uint_least8_t offset;
		/// The bit mask for extracting the field
		T mask;
};

template<typename T>
class ControlRegisterTransaction;

template<typename T>
class CopiedControlRegister;

/// Abstraction for manipulating memory-mapped control registers
template<typename T>
class ControlRegister {
	static_assert(std::is_integral_v<T>, "ControlRegister: underlying type must be integral");
	public:

		/// Construct for a control register at the specified address
		constexpr ControlRegister(uintptr_t register_address) :
			reg(register_address)
		{}

		/// Set a field of the register to a new value
		/// @param field BitField specifying location in the register
		/// @param new_value New value to apply to that field which has not been shifted or masked.
		/// 	If new_value has any bits set beyond the width of the field, this will overwrite
		/// 	neighboring fields
		void set_field(BitField<T> field, T new_value) const {
			T reg_temp = read();

			// Clear the bits in the field
			reg_temp &= ~field.get_mask();

			// Write the new value at the offset
			reg_temp |= new_value << field.get_offset();

			write(reg_temp);

		}

		/// Read a field of the register and return as the desired type
		/// @tparam R Desired return type to cast into (defaults to the register's underlying type)
		template<typename R = T>
		R get_field(BitField<T> field) const {
			return static_cast<R>((read() & field.get_mask()) >> field.get_offset());
		}

		/// Read and return the value of the register
		T read() const {
			return *reinterpret_cast<volatile T*>(reg);
		}

		/// Write a new value to the register
		void write(T value) const {
			*reinterpret_cast<volatile T*>(reg) = value;
		}

		/// Return a control structure to allow changing multiple fields in one register write
		///
		/// @note This is not "atomic" as in thread-safe atomicity. Here, atomic refers to changing
		/// multiple bitfields in a single write instruction
		ControlRegisterTransaction<T> start_atomic_transaction() const {
			return ControlRegisterTransaction(*this, read());
		};

		/// Return a control structure for reading multiple register fields in one register read
		///
		/// @return A CopiedControlRegister instance which allows reading fields but is detached
		/// from the actual register 
		CopiedControlRegister<T> copy_value() const {
			return CopiedControlRegister(read());
		}

	private:
		uintptr_t reg;

};

/// A temporary copy of a register used to modify the fields without applying to the register until
/// calling finalize()
template<typename T>
class ControlRegisterTransaction : public ControlRegister<T> {
	public:
		ControlRegisterTransaction(const ControlRegister<T>& reg, T init_value) :
			ControlRegister<T>(reinterpret_cast<uintptr_t>(&temp_value)),
			reg(reg),
			temp_value(init_value)
		{}

		/// Write the pending changes to the register
		void finalize() const {
			reg.write(temp_value);
		}

	private:
		const ControlRegister<T>& reg;
		T temp_value;

};

/// A read-only copy of a control register that allows reading multiple fields with a single read
/// instruction
template<typename T>
class CopiedControlRegister {
	public:

		CopiedControlRegister(T value) :
			copied_value(value)
		{}

		/// Read a field of the saved register and return as the desired type
		/// @tparam R Desired return type to cast into (defaults to the register's underlying type)
		template<typename R = T>
		R get_field(BitField<T> field) const {
			return static_cast<R>((read() & field.get_mask()) >> field.get_offset());
		}

		/// Read and return the saved copy of the register
		T read() const {
			return copied_value;
		}

	private:
		T copied_value;
};