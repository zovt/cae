#pragma once


#include <variant>
#include <optional>
#include <functional>


namespace util {

template<typename T, typename ...Variants>
std::optional<std::reference_wrapper<T>> opt_get(std::variant<Variants...>& var) {
	if (std::holds_alternative<T>(var)) {
		return std::ref(std::get<T>(var));
	}
	
	return std::nullopt;
}

}