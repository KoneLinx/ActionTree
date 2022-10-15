// Copyright (C) 2021 Kobe Vrijsen <kobevrijsen@posteo.be>
// 
// ActionTree - Tree based decision/action stucture helper. An alternative to branches.
// 
// This file is free software and distributed under the terms of the European Union
// Public Lincense as published by the European Commision; either version 1.2 of the
// License, or, at your option, any later version.

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