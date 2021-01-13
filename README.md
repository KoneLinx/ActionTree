# ActionTree

Tree based decision making and action performing structure.

# Documentation

## Actions

Actions perform an action. They can be seen as a function that takes parameters return a result. That is pretty much all there is to explain about what they do.

Actions can also be combined together.

```c++
action_A | action_B
```
This result is a new action that, when called, calls these two actions, in sequence.
The returned type depends in the returned types of two actions used.

Old types | New type
--- | ---
`void`, `void` | `void`
`void`, `B` | `B`
`A`, `void` | `A`
Valid: `a + b` | `a + b`
`A`, `B` | `pair<A, B>`

There is no limit to the amount of actions that can be combined
```c++
action | action | action | ...
```
Though, be aware that when the return types do not match up, they may end up nested in multiple pairs.

## Decisions

Decisions are simmilar to actions. They also take paramaters but instead return a boolean value. Decisions are used to control the actions and branches that are executed.

Just like actions, decisions can be combined together:
```c++
decision_A | decision_B
decision_A & decision_B
```
Or inverted:
```c++
!decision
```

Much like ordinary boolean values:

operator | operation
--- | ---
`\|` | OR
`\|\|` | OR
`&` | AND
`&&` | AND
`!` | NOT

They too can be combined without limits
```c++
decision & desicion | !decision  &&  decision | !decision  ||  decision
```
Be wary about the operator precedence and place brackets where needed.

## Conditional actions

Actions can be made conditional by binding it to the result of a decision.

```c++
decision & action
```

This will give you a new action that will first call the decision and, if that returned true, call the action next.
Based on the returned type of the action, the new returned type might differ:

Old type | New type
--- | ---
`void` | `void`
Otherwise | `optional<T>`

Since the goal of what must be done with or without the result is unknown, optional is used.

This combination results in an action. to bind mutiple action together, you must do that first. Same for the decision.
```c++
(decision | decision) & (action | action)
```

## Branches

As you would expect, and if/else branch can also be created.

Since the ternary ` ? : ` operator cannot be overloaded, a different solution has to be used:
```c++
decision && action_yes || action_no
```
This notation stems from shell scripting languages where this technique is sometimes used to fit a condition on one line.

The result type agian depens on the used actions.

Old types | New type
--- | ---
both `void` | `void`
both `T` | `T`
`void`, `B` | `optional<B>`
`A`, `void` | `optional<A>`
`A`, `B` | `variant<A, B>`

A series of branches, which some may call a `switch` or a series of `else if`s, can also be constructed.
```c++
decision_1 && action_1 || decision_2 && action_2 || action_none
```
Or possibly even longer.
Also watch out for non matching types so the results don't get nested too deeply.

## Visitors

Visitors are functions that take the result of the action they are combined with and return an new result.

To add a visitor to the output of an action:
```c++
action | visitor
```
This will make a new action that returns the visitors return type. When called the action gets calles first, after which its returned value is passed to the visitor, whataver the visitor returs is returned at the end.

The visitor provied in this library is specially built to ease the usage of optional and variant.

The visitor can be called with the following typed as single parameter

### `void`

* `operator()(void)` is called.
  * *this is up to you to implement, if it is required*

### `T`

* `operator()(T)` is called.
  * *this is up to you to implement, if it is required*

### `optional<T>`

* Visitor has `operator()(void)`
  * Optional has value
    * `operator()(T)` is called.
  * Optional has no value
    * `operator()(void)` is called.
    
* Visitor has no `operator()(void)`
  * T is default constructable
    * Optional has value
      * `operator()(T)` is called.
    * Optional has no value
      * `T()` is returned. No `operator()` is called.
  * T is not default constrcutable
    * <span style="color:red">*Illformed*</span>. static_assert.
    
### `variant<A, B>`

* `A` is active
  * `operator()(A)` is called.
* `B` is active
  * `operator()(B)` is called.
  
The return type will be whatever these calls return.

This can be used to undo the possible `optional` and `variant` objects that are created in the previous actions:
      
