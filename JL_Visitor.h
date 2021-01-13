// Visitor.h - Visitor structure.
/*
   Copyright (C) 2021 Kobe Vrijsen

   this file is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3.0 of the License, or (at your option) any later version.

   This file is made available in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, see
   <https://www.gnu.org/licenses/>.

   Author :   KoneLinx
   Contact:   konelinx@posteo.be
*/

#pragma once

#include <variant>
#include <optional>

namespace JL
{

	template<typename ...Base>
	struct Visitor : Base...
	{
		using Base::operator()...;

		template <typename ...T>
		constexpr auto operator() (std::variant<T...> const& variant) const;
		template <typename ...T>
		constexpr auto operator() (std::variant<T...> const& variant);

		template <typename T>
		constexpr auto operator() (std::optional<T> const& optional, T const& alternative) const;
		template <typename T>
		constexpr auto operator() (std::optional<T> const& optional, T const& alternative);
		template <typename T>
		constexpr auto operator() (std::optional<T> const& optional) const;
		template <typename T>
		constexpr auto operator() (std::optional<T> const& optional);
	};

	template<typename ...T>
	Visitor(T...) -> Visitor<T...>;

};



// Implementation

namespace JL
{

	template <typename ...B>
	template <typename ...T>
	constexpr auto Visitor<B...>
	::operator() (std::variant<T...> const& variant) const
	{
		return std::visit(*this, variant);
	}

	template <typename ...B>
	template <typename ...T>
	constexpr auto Visitor<B...>
	::operator() (std::variant<T...> const& variant)
	{
		return std::visit(*this, variant);
	}

	template<typename ...B>
	template<typename T>
	constexpr auto Visitor<B...>
	::operator() (std::optional<T> const& optional, T const& alternative) const
	{
		return operator()(optional.value_or(alternative));
	}

	template<typename ...B>
	template<typename T>
	constexpr auto Visitor<B...>
	::operator() (std::optional<T> const& optional, T const& alternative)
	{
		return operator()(optional.value_or(alternative));
	}

	template<typename ...B>
	template<typename T>
	constexpr auto Visitor<B...>
	::operator() (std::optional<T> const& optional) const
	{
		using R = decltype(operator()(std::declval<T&>()));
		if (optional.has_value())
		{
			return operator()(optional.value());
		}
		else
		if constexpr (std::is_invocable_v<decltype(*this)>)
		{
			return operator()();
		}
		else
		{
			static_assert(std::is_default_constructible_v<R>, "Cannot construct default value, please provide alternative value");
			return R();
		}
	}

	template<typename ...B>
	template<typename T>
	constexpr auto Visitor<B...>
	::operator() (std::optional<T> const& optional)
	{
		using R = decltype(operator()(std::declval<T&>()));
		if (optional.has_value())
		{
			return operator()(optional.value());
		}
		else
		if constexpr (std::is_invocable_v<decltype(*this)>)
		{
			return operator()();
		}
		else
		{
			static_assert(std::is_default_constructible_v<R>, "Cannot construct default value, please provide alternative value");
			return R();
		}
	}

}