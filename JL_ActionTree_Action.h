// ActionThree_Action.h - Action structure.
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

#include "JL_ActionTree_Base.h"

namespace JL::action_tree
{

	template <typename F>
	struct Action : impl::Functor<F>
	{
		TEMPLATE  /* Action */ operator |  (Action<T>);
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

}