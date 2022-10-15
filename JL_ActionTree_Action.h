// Copyright (C) 2021 Kobe Vrijsen <kobevrijsen@posteo.be>
// 
// ActionTree - Tree based decision/action stucture helper. An alternative to branches.
// 
// This file is free software and distributed under the terms of the European Union
// Public Lincense as published by the European Commision; either version 1.2 of the
// License, or, at your option, any later version.

#pragma once

#include "JL_ActionTree_Base.h"
#include "JL_Visitor.h"

namespace JL::action_tree
{

	template <typename F>
	struct Action : impl::Functor<F>
	{
		TEMPLATE  /* Action */ operator |  (Action <T   >);
		TEMPLATEV /* Action */ operator |  (Visitor<T...>);
	};

	template <typename T>
	Action(T)->Action<T>;

}



// Implementation

namespace JL::action_tree
{

	namespace impl
	{

		template <typename A, typename B, typename = void>
		struct is_addable : std::false_type {};

		template <typename A, typename B>
		struct is_addable<A, B, std::void_t<decltype(std::declval<A>() + std::declval<B>())>> : std::true_type {};

		template <typename A, typename B>
		constexpr bool is_addable_v = is_addable<A, B>::value;

	}

	template <typename _T>
	template <typename T>
	auto Action<_T>::operator|(Action<T> other)
	{
		auto f {
			[a = *this, b = std::move(other)]
			(auto&& ... p) mutable
			{
				using A = decltype(a(p...));
				using B = decltype(b(p...));

				if constexpr (std::is_void_v<A>)			// A = void
					return a(p...), b(p...);
				else
				if constexpr (std::is_void_v<B>)			// B = void
				{
					auto out = a(p...);
					return b(p...), out;
				}
				else
				if constexpr (impl::is_addable_v<A, B>)		// A + B
					return a(p...) + b(p...);
				else										// {A, B}
					return std::pair{ a(p...), b(p...) };
			}
		};
		return Action<decltype(f)>{ std::move(f) };
	}

	template <typename _T>
	template <typename ...T>
	auto Action<_T>::operator|(Visitor<T...> visitor)
	{
		auto f{
			[a = *this, v = std::move(visitor)]
			(auto&& ... p) mutable
			{
				return v(a(p...));
			}
		};
		return Action<decltype(f)>{ std::move(f) };
	}

}