# ActionTree

Tree based decision making and action performing structure.

# Documentation

## Actions

Actions perform an action. They can be seen as a function that takes parameters return a result. That is pretty much all there is to explain about what they do.

```c++
Action action{
  /*labda expression*/
  [ /*captures*/ ]
  ( /*parameters*/ ) -> T
  { /*code*/ }
}
```
The lambda may have any capture, parameter, mutability, and return type. There are no restrictions

Actions inherit from the lambda or functor object that has been given to them and can always be called with the `operator()`. They will return the result of their inherited `operator()`.
```
result = action(param...);
```
When calling an action, be sure to pass in the correct argument types. If compile errors occur, verify whether the arguments really are correct.

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

Examples:
```c++
sayHello | sayWorld    // says "Hello", next, says "World"
getMean | getMedian    // gets a mean, next a median. Returns a pair of them (Let's say the types are different).
```

## Decisions

Decisions are simmilar to actions. They also take paramaters but instead return a boolean value. Decisions are used to control the actions and branches that are executed.

```c++
Decision decision{
  /*labda expression*/
  [ /*captures*/ ]
  ( /*parameters*/ ) -> bool
  { /*code*/ }
}
```
The lambda may have any capture, parameter and mutability. However, the return type must be convertible to bool.

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

Actions can also be bound to state changes of a decision.
```c++
decision +action_on
decision -action_off
```
This will activate action when the result of the decision changed compared to it's previous state.

It forms a new decision that first call its decision, and if the state changed compared to the previous state, action is called. The returned result of the action is discarded and the decision's state is returned.

op | on
---|---
`+` | false -> true
`-` | true -> false

We can bind any number of actions
```c++
decision +action_on -action_off
decision +(action_a | action_b)  // This line is practically ...
decision +action_a +action_b     // ... equal to this line 
```

Examples:
```c++
!bottle_filled & fill_bottle         // For as long the bottle isn't filled, keep filling it.
!is_eof -close_file & read_line      // Is not EOF? Yes: read line. From the moment file is EOF: close file.
is_light_on -turn_light_on           // When the light goes out, turn it on again.
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

Examples:
```c++
is_airbourne && fall || run            // Will fall when in the air, run when on the ground
needs_something && search || explore   // When something is needed, search, otherwise explore.
// Is the button pressed? Yes: +1 *BEEEEP*   No:  Is it pressed two times already? No: nothing  Yes: turn red and flash
pressed +press_count && beep || pressed_twice +turn_red & flash
```

## Visitors

Visitors are functions that take the result of the action they are combined with and return an new result.

```c++
Visitor visitor{

  // labda expression 1
  [ /*captures*/ ]
  ( /*parameters*/ )
  { /*code*/ },
  
  // labda expression 2
  /* ... */
 
  // labda expression N
  /* ... */
 
}
```
Visitors can be constructed with any amount of lambdas, which can all have different states return types. Provided that none have the same input paramaters.

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

```c++
stream_open & read_stream | parse_input                              // Is the stream open? Yes:  read input from the stream,   next parse the input
(in_memory && get_from_memory || get_from_file) | transform_data     // Get data from memory or file, then transform it
```
      
## Future changes

Combined action with equal types return the sum of the returned values. This may not fit in all use cases, so a better way is begin sought after.
It could be changed to `|` instead. Or a custom operator might be asked for.

Actions may also return pairs. This may be changed in the future to tuples that are also extended when going through mutiple layers.
Instead of `pair<A, pair<B, C>>`, it would be better off being `tuple<A, B, C>`.
This is also the case for branches accumulating variants (`variant<A, variant<B, C>>`). They too might be reduced to one variant instead (`variant<A, B, C>`).

## Known bugs

~~Currently, when moving/copying a visitor with two or more lambda objects, which is what happens internally, may corrupt the stack. The cause is being investigated.~~
I have no idea how I fixed this. But I fixed it.

