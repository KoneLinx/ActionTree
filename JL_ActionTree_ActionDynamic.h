// ActionThree_ActionDynamic.h - Dynamic action structure.
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

namespace JL::action_tree
{

	template <typename>
	struct ActionDynamic {};

	template <typename R, typename ... P>
	struct ActionDynamic<R(P...)> : std::function<R(P...)>
	{
		explicit             ActionDynamic () = default;
		template<typename T>
		explicit             ActionDynamic (Action<T>);
		TEMPLATE /* This& */ operator =    (Action<T>);
	};


};

namespace JL::action_tree
{

	template<typename R, typename ...P>
	template<typename T>
	ActionDynamic<R(P...)>::ActionDynamic(Action<T> action)
	{
		operator=(std::move(action));
	}

	template <typename R, typename ... P>
	template <typename T>
	auto ActionDynamic<R(P...)>::operator = (Action<T> a)
	{
		if constexpr (std::is_same_v<R, decltype(a(std::declval<P>()...))>)
		{
			return std::function<R(P...)>::operator=(
				[a = std::move(a)]
				(P ... p) mutable->R
				{
					return a(p...);
				}
			);
		}
		else static_assert(false, "ActionDynamic::operator=(Action)  Action does not return the correct type.");
	}

}