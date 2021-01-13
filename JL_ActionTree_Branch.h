// ActionThree_Branch.h - Branch structure.
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
#include "JL_ActionTree_Action.h"

namespace JL::action_tree::impl
{

	template <typename D, typename A>
	struct Branch
	{
		D decision;
		A action;

		TEMPLATE  /* Action */ operator || (Action<T>);
		TEMPLATE2 /* Stack  */ operator || (Branch<T1, T2>&&);
	};

	template <typename ... B>
	struct Stack : std::tuple<B...>
	{
		using std::tuple<B...>::tuple;

		TEMPLATE  /* Action */ operator || (Action<T>);
		TEMPLATE2 /* Stack  */ operator || (Branch<T1, T2>&&);

		template<typename T, size_t ... I>
		auto ReduceStack(Action<T>&, std::index_sequence<I...>);
	};

}


namespace JL::action_tree::impl
{

	//-------------
	//   Branch

	template <typename _D, typename _A>
	template <typename T>
	auto Branch<_D, _A>::operator||(Action<T> action)
	{
		auto f{
			[a = std::move(*this), b = std::move(action)]
			(auto&& ... p) mutable
			{
				using Ra = decltype(a.action(p...));
				using Rb = decltype(b(p...));
				bool const test = a.decision(p...);
				if constexpr (std::is_same_v<Ra, Rb>)
				{
					return test ? a.action(p...) : b(p...);
				}
				else
				if constexpr (std::is_void_v<Ra>)
				{
					using Maybe = impl::Maybe<Rb>;
					return test ? (a.action(p...), Maybe{}) : Maybe{ b(p...) };
				}
				else
				if constexpr (std::is_void_v<Rb>)
				{
					using Maybe = impl::Maybe<Ra>;
					return test ? Maybe{ a.action(p...) } : (b(p...), Maybe{});
				}
				else
				{
					using Either = Either<Ra, Rb>;
					if (a.decision(p...))
						return Either{ a.action(p...) };
					else
						return Either{ b(p...) };
				}
			}
		};
		return Action<decltype(f)>{ std::move(f) };
	}

	template <typename _D, typename _A>
	template <typename T1, typename T2>
	auto Branch<_D, _A>::operator||(Branch<T1, T2>&& other)
	{
		return Stack<Branch<_D, _A>, Branch<T1, T2>>{ std::move(*this), std::move(other) };
	}

	//------------
	//   Stack

	template <typename ... _T>
	template <typename A, size_t ... I>
	auto Stack<_T...>::ReduceStack(Action<A>& action, std::index_sequence<I...>)
	{
		//    <-----------------o
		//  (1 || (... ( N || action)))
		return (std::move(std::get<I>(*this)) || ... || std::move(action));
	}

	template <typename ... _T>
	template <typename T>
	auto Stack<_T...>::operator||(Action<T> action)
	{
		return ReduceStack(
			action,
			std::index_sequence_for<_T...>{}
		);
	}

	template <typename ... _T>
	template <typename T1, typename T2>
	auto Stack<_T...>::operator||(Branch<T1, T2>&& next)
	{
		return Stack<_T..., Branch<T1, T2>>{
			std::tuple_cat(
				std::tuple<_T...>{ std::move(*this) },
				std::tuple{ std::move(next) }
			)
		};
	}

}