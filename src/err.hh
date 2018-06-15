#pragma once

#include <variant>
#include <string_view>

#include "macros.hh"

namespace err {

typedef std::string_view Err;

template <typename T>
using Result = std::variant<Err, T>;

}

#define try(ident_decl, maybe_err)\
	auto CAT(__err__, __LINE__) = maybe_err;\
	if (std::holds_alternative<Err>(CAT(__err__, __LINE__))) {\
		return std::get<Err>(CAT(__err__, __LINE__));\
	}\
	ident_decl = std::get<std::variant_alternative<1, decltype(CAT(__err__, __LINE__))>::type>(CAT(__err__, __LINE__))
