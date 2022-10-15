// Copyright (C) 2021 Kobe Vrijsen <kobevrijsen@posteo.be>
// 
// ActionTree - Tree based decision/action stucture helper. An alternative to branches.
// 
// This file is free software and distributed under the terms of the European Union
// Public Lincense as published by the European Commision; either version 1.2 of the
// License, or, at your option, any later version.

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