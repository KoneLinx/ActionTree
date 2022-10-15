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
#include "JL_ActionTree_Branch.h"

namespace JL::action_tree
{

	template <typename F>
	struct Decision : impl::Functor<F>
	{
		auto      /*Decision*/ operator ! ();

		TEMPLATE  /*Decision*/ operator |  (Decision<T>);
		TEMPLATE  /*Decision*/ operator || (Decision<T>);
		TEMPLATE  /*Decision*/ operator &  (Decision<T>);
		TEMPLATE  /*Decision*/ operator && (Decision<T>);

		TEMPLATE  /* Action */ operator +  (Action<T>);
		TEMPLATE  /* Action */ operator -  (Action<T>);
		TEMPLATE  /* Action */ operator &  (Action<T>);

		TEMPLATE  /* Branch */ operator && (Action<T>);
	};

	template <typename T>
	Decision(T)->Decision<T>;

}



// Implementation

namespace JL::action_tree
{

	template <typename _T>
	auto Decision<_T>::operator!()
	{
		auto f{
			[d = *this]
			(auto&& ... p) mutable
			{
				return !d(p...);
			}
		};
		return Decision<decltype(f)>{ std::move(f) };
	}

	template <typename _T>
	template <typename T>
	auto Decision<_T>::operator|(Decision<T> other)
	{
		auto f{
			[a = *this, b = std::move(other)]
			(auto&& ... p) mutable
			{
				return a(p...) || b(p...);
			}
		};
		return Decision<decltype(f)>{ std::move(f) };
	}

	template <typename _T>
	template <typename T>
	auto Decision<_T>::operator||(Decision<T> other)
	{
		return *this | std::move(other);
	}

	template <typename _T>
	template <typename T>
	auto Decision<_T>::operator&(Decision<T> other)
	{
		auto f{
			[a = *this, b = std::move(other)]
			(auto&& ... p) mutable
			{
				return a(p...) && b(p...);
			}
		};
		return Decision<decltype(f)>{ std::move(f) };
	}

	template <typename _T>
	template <typename T>
	auto Decision<_T>::
		operator&&(Decision<T> other)
	{
		return *this & std::move(other);
	}

	template <typename _T>
	template <typename T>
	auto Decision<_T>::operator+(Action<T> other)
	{
		auto f{
			[d = *this, a = std::move(other), on = false]
			(auto&& ... p) mutable
			{
				bool const test = d(p...);
				if (!on && test)
					(void)a(p...);
				return on = test;
			}
		};
		return Decision<decltype(f)>{ std::move(f) };
	}

	template <typename _T>
	template <typename T>
	auto Decision<_T>::operator-(Action<T> other)
	{
		auto f{
			[d = *this, a = std::move(other), on = true]
			(auto&& ... p) mutable
			{
				bool const test = d(p...);
				if (on && !test)
					(void)a(p...);
				return on = test;
			}
		};
		return Decision<decltype(f)>{ std::move(f) };
	}

	template <typename _T>
	template <typename T>
	auto Decision<_T>::operator&(Action<T> action)
	{
		auto f{
			[d = *this, a = std::move(action)]
			(auto&& ... p) mutable
			{
				using R = decltype(a(p...));
				if constexpr (std::is_void_v<R>)
				{
					if (d(p...))
						a(p...);
					return;
				}
				else
				{
					using Maybe = impl::Maybe<R>;
					return d(p...) ? Maybe{ a(p...) } : Maybe{};
				}
			}
		};
		return Action<decltype(f)>{ std::move(f) };
	}

	template <typename _T>
	template <typename T>
	auto Decision<_T>::operator&&(Action<T> action)
	{
		return impl::Branch<Decision<_T>, Action<T>>{ *this, std::move(action) };
	}

}