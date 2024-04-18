// The code is basing on article: https://sii.pl/blog/en/implementing-a-state-machine-in-c17/

// MIT License

// Copyright (c) 2020 Michał Adamczyk

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef FSM_H
#define FSM_H

#include <tuple>
#include <variant>

namespace fsm
{

template <typename Action>
struct ByDefault
{
	template <typename Event>
	Action handle(const Event&) const
	{
		return Action{};
	}
};

template <typename Event, typename Action>
struct On
{
	Action handle(const Event&) const
	{
		return Action{};
	}
};

struct Nothing
{
	template <typename Machine, typename State, typename Event>
	void execute(Machine&, State&, const Event&)
	{
	}
};

template <typename TargetState>
class TransitionTo
{
public:
	template <typename Machine, typename State, typename Event>
	void execute(Machine& machine, State& prevState, const Event& event)
	{
		leave(prevState, event);
		TargetState& newState = machine.template transitionTo<TargetState>();
		enter(newState, event);
	}

private:
	void leave(...)
	{
	}

	template <typename State, typename Event>
	auto leave(State& state, const Event& event) -> decltype(state.onLeave(event))
	{
		return state.onLeave(event);
	}

	void enter(...)
	{
	}

	template <typename State, typename Event>
	auto enter(State& state, const Event& event) -> decltype(state.onEnter(event))
	{
		return state.onEnter(event);
	}
};

template <typename TargetState, typename T>
class ParameterizedTransitionTo
{
public:
	template <typename Machine, typename State, typename Event>
	void execute(Machine& machine, State& prevState, const Event& event)
	{
		leave(prevState, event);
		TargetState& newState = machine.template transitionTo<TargetState>();
		enter(newState, event);
	}
	
	T param;

private:
	void leave(...)
	{
	}

	template <typename State, typename Event>
	auto leave(State& state, const Event& event) -> decltype(state.onLeave(event))
	{
		return state.onLeave(event);
	}

	void enter(...)
	{
	}

	template <typename State, typename Event>
	auto enter(State& state, const Event& event) -> decltype(state.onEnter(event, param))
	{
		return state.onEnter(event, param);
	}
};

template <typename... Actions>
class OneOf
{
public:
	template <typename T>
	OneOf(T&& arg)
		: options(std::forward<T>(arg))
	{
	}

	template <typename Machine, typename State, typename Event>
	void execute(Machine& machine, State& state, const Event& event)
	{
		std::visit([&machine, &state, &event](auto& action){ action.execute(machine, state, event); }, options);
	}

private:
	std::variant<Actions...> options;
};

template <typename Action>
struct Maybe : public OneOf<Action, Nothing>
{
	using OneOf<Action, Nothing>::OneOf;
};

template <typename... Handlers>
struct Will : Handlers...
{
	using Handlers::handle...;
};

template <typename... States>
class StateMachine
{
public:
	StateMachine() = default;

	StateMachine(States... states)
		: states(std::move(states)...)
	{
	}

	template <typename State>
	State& transitionTo()
	{
		State& state = std::get<State>(states);
		currentState = &state;
		return state;
	}

	template <typename Event>
	void handle(const Event& event)
	{
		handleBy(event, *this);
	}

	template <typename Event, typename Machine>
	void handleBy(const Event& event, Machine& machine)
	{
		auto passEventToState = [&machine, &event] (auto statePtr) {
			auto action = statePtr->handle(event);
			action.execute(machine, *statePtr, event);
		};
		std::visit(passEventToState, currentState);
	}

private:
	std::tuple<States...> states;
	std::variant<States*...> currentState{ &std::get<0>(states) };
};

}

#endif
