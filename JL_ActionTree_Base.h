// Copyright (C) 2021 Kobe Vrijsen <kobevrijsen@posteo.be>
// 
// ActionTree - Tree based decision/action stucture helper. An alternative to branches.
// 
// This file is free software and distributed under the terms of the European Union
// Public Lincense as published by the European Commision; either version 1.2 of the
// License, or, at your option, any later version.

#pragma once

#include <functional>
#include <tuple>
#include <optional>
#include <variant>

namespace JL::action_tree
{

	#define TEMPLATE  template <typename T> auto
	#define TEMPLATE2 template <typename T1, typename T2> auto
	#define TEMPLATEV template <typename ...T> auto

	namespace impl
	{

		template <typename F>
		struct Functor : F
		{
			using F::operator();
		};

		template <typename T>
		Functor(T)->Functor<T>;
		
		template <typename A, typename B>
		using Either = std::variant<A, B>;

		template <typename A>
		using Maybe  = std::optional<A>;

	}

}