# A very basic functional programming language.

## Building.

To build this thing you will need `gcc`, `libc` and plan9's `mk`.

```
$ git clone REPO calcl
$ cd calcl
$ mk
```

## Example repl session.

```
> 1
1.000000
> 1 + 2
3.000000
> 1 * 3
3.000000
> 2 / 4
0.500000
> 2 ^ 3
8.000000
> fn x: x + 1
<fn x>
> (fn x: x + 1) 3
4.000000
> let x = 10
> x
10.000000
> let f = fn x: x + 1
> f 3
4.000000
> let fact = fn x: if x < 1 then 1 else x * fact (x - 1)
> fact 5
120.000000
> let pair = fn x y f: f x y
> let cons = pair
> let car = fn p: p (fn x y: x)
> let cdr = fn p: p (fn x y: y)
> car (cons 1 2)
1.000000
> cdr (cons 2 3)
3.000000
```
