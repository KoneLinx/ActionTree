// ActionThree_Base.h - Tree based decision/action stucture helper base.
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