#pragma once

#include "JL_ActionTree.h"
using namespace JL::action_tree;

#define CATCH_CONFIG_MAIN
#include "../catch2/catch.hpp"

#pragma region test_utilities

struct Value
{
	int value = int(0xEEEEEEEE); // If uninitialized
};

struct Addable : Value
{
	Addable operator + (Addable const& o) { return { value + o.value }; };
};

struct NonDefault : Value
{
	NonDefault() = delete;
	NonDefault(int i) : Value{ i } {}
};

Action makeNothing   { [](auto){} };
Action makeValue     { [](int i) -> Value      { return {1 * i}; } };
Action makeAddable   { [](int i) -> Addable    { return {2 * i}; } };
Action makeValueAlt  { [](int i) -> Value      { return {3 * i}; } };
Action makeAddableAlt{ [](int i) -> Addable    { return {5 * i}; } };
Action makeNonDefault{ [](int i) -> NonDefault { return {7 * i}; } };

Decision alwaysTrue       { [](auto)  { return true; } };
Decision isNotZero        { [](int i) { return i != 0; } };
Decision isEven           { [](int i) { return (i & 1) == 0; } };
Decision isGreaterEqualTwo{ [](int i) { return i >= 2; } };

#pragma endregion

#define REQUIRE_TYPE(type, expr) static_assert(std::is_same_v<type, decltype(expr)>)

TEST_CASE("Test Action")
{

	REQUIRE_TYPE(void   , makeNothing(0));
	REQUIRE_TYPE(Value  , makeValue  (0));
	REQUIRE_TYPE(Addable, makeAddable(0));

	{

		// Test Action

		REQUIRE(makeValue  (7).value == 7 );
		REQUIRE(makeAddable(7).value == 14);

		REQUIRE_TYPE(void   , (makeNothing | makeNothing)(0));
	
	}

	{

		// Test combined action (only one value)

		auto onlyA = makeValue   | makeNothing;
		auto onlyB = makeNothing | makeValue  ;
		
		REQUIRE_TYPE(Value  , onlyA(0));
		REQUIRE_TYPE(Value  , onlyB(0));
	
		REQUIRE(onlyA(7).value == 7);
		REQUIRE(onlyB(7).value == 7);

	}

	{

		// Test combined action (two values)

		auto values = makeValue   | makeValueAlt;
		auto added  = makeAddable | makeAddableAlt;

		using Paired = std::pair<Value, Value>;
		REQUIRE_TYPE(Paired , values(0));
		REQUIRE_TYPE(Addable, added (0));

		auto [v1, v2] = values(7);
		auto va       = added (7);
		REQUIRE(v1.value == 7);
		REQUIRE(v2.value == 7 * 3);
		REQUIRE(va.value == (7 * 2) + (7 * 5) );

	}

}

TEST_CASE("Test Decision")
{

	REQUIRE_TYPE(bool, alwaysTrue(0));
	REQUIRE(alwaysTrue(0) == true);

	{

		// Test combined and not

		auto combinedAND    = isGreaterEqualTwo && isEven;
		auto combinedOR     = isGreaterEqualTwo || isEven;
		auto combinedNot    = !isGreaterEqualTwo;

		REQUIRE_TYPE(bool, combinedAND(0));

		constexpr bool results_A[] { false, false, true, true , true, true  };
		constexpr bool results_B[] { true , false, true, false, true, false };
		bool test{ true };
		for (int i{}; i < std::size(results_A); ++i)
		{
			bool test{ true };
			test &= (combinedAND(i) == (results_A[i] && results_B[i]));
			test &= (combinedOR (i) == (results_A[i] || results_B[i]));
			test &= (combinedNot(i) == !results_A[i]);
			REQUIRE(test);
		}

	}
}

TEST_CASE("Test conditional Action")
{

	{
		// Conditions

		auto conditionalNothing = isEven & makeNothing;
		auto conditionalValue   = isEven & makeValue;

		REQUIRE_TYPE(void, conditionalNothing(0));
		REQUIRE_TYPE(Value, conditionalValue(0));

		REQUIRE(conditionalValue(2).value == 2            );
		REQUIRE(conditionalValue(1).value == Value{}.value);

	}

	{

		// Flip conditions

		int state{ 0 };

		Action   on      { [&state](auto)   { state =  1; } };
		Action   off     { [&state](auto)   { state = -1; } };
		Decision decision{ [&state](bool b) { state =  0; return b; } };

		auto decide = decision +on -off;

		decide(false); REQUIRE(state == -1);
		decide(false); REQUIRE(state ==  0);
		decide(true ); REQUIRE(state ==  1);
		decide(true ); REQUIRE(state ==  0);
		decide(false); REQUIRE(state == -1);
		decide(true ); REQUIRE(state ==  1);

	}

}

#include <optional>

TEST_CASE("Test Branch")
{

	auto eitherValue = isNotZero && makeValue   || makeValueAlt;
	auto eitherType  = isNotZero && makeValue   || makeAddable;
	auto eitherTrue  = isNotZero && makeValue   || makeNothing;
	auto eitherFalse = isNotZero && makeNothing || makeAddable;

	using Either = impl::Either<Value, Addable>;
	using True   = std::optional<Value>;
	using False  = std::optional<Addable>;
	REQUIRE_TYPE(Value , eitherValue(0));
	REQUIRE_TYPE(Either, eitherType (0));
	REQUIRE_TYPE(True  , eitherTrue (0));
	REQUIRE_TYPE(False , eitherFalse(0));

	REQUIRE(eitherValue(5).value == 5);
	REQUIRE(eitherValue(0).value == 0);

	auto five = eitherType(5);
	REQUIRE(five.first .has_value() == true );
	REQUIRE(five.second.has_value() == false);
	auto zero = eitherType(0);
	REQUIRE(zero.first .has_value() == false);
	REQUIRE(zero.second.has_value() == true );

	REQUIRE(eitherTrue (5).has_value() == true );
	REQUIRE(eitherTrue (0).has_value() == false);

	REQUIRE(eitherFalse(5).has_value() == false);
	REQUIRE(eitherFalse(0).has_value() == true );

}

TEST_CASE("Test branch stack")
{

	using namespace std::string_view_literals;
	using sv = std::string_view;

	auto select = [](int i) { return Decision{ [i](int j) { return i == j; } }; };
	auto name   = [](sv sv) { return Action  { [sv](int)  { return sv;     } }; };

	{

		// Same return type

		auto what = select(0) && name("a") || select(1) && name("b") || name("c");

		REQUIRE_TYPE(sv, what(0));
		REQUIRE(what(0) == "a");
		REQUIRE(what(1) == "b");
		REQUIRE(what(2) == "c");

	}

	{
		// Different return types

		auto stack = select(0) && makeValue || select(1) && makeAddable || makeNonDefault;

		using Pair = impl::Either<Value, impl::Either<Addable, NonDefault>>;
		REQUIRE_TYPE(Pair, stack(0));
		REQUIRE(stack(0).first .has_value() == true );
		REQUIRE(stack(0).second.has_value() == false);
		REQUIRE(stack(1).second.has_value() == true );
		REQUIRE(stack(1).second.value().first .has_value() == true );
		REQUIRE(stack(1).second.value().second.has_value() == false);
		REQUIRE(stack(2).second.value().first .has_value() == false);
		REQUIRE(stack(2).second.value().second.has_value() == true );

	}

}