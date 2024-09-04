# A very basic functional programming language.

## About.

This is an interpreter for an ML-like functional programming language with Hindley-Milner type inference
(typing is disabled by default, you can enable it via `-t` flag).
It supports both strict (the default) and lazy (`-l`) evaluation strategies.

There is also a very limited compiler for `amd64`.
For now it only supports strict evaluation with typing.

This project is purely educational and just-for-fun.

## Building.

To build this thing you will need `gcc`, `libc` and plan9's `mk`.

```
$ git clone REPO calcl
$ cd calcl
$ mk
```

## Example repl session.

```
$ ./interp
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
> let fact x = if x < 1 then 1 else x * fact (x - 1)
> fact 5
120.000000
> let cons x y = fn f: f x y
> let car p = p (fn x y: x)
> let cdr p = p (fn x y: y)
> car (cons 1 2)
1.000000
> cdr (cons 2 3)
3.000000
> let z f = (fn h: fn x: f x (h h)) (fn h: fn x: f x (h h))
> let fac x f = if x < 1 then 1 else x * f (x - 1)
> let f = z fac
> f 3
6.000000
> f 5
120.000000
```

## Compiler usage example.

```
$ ./comp <examples/test.calcl >test.s
$ gcc -o test -lm test.s runtime.o
$ ./test
1.000000
0.000000
0.000000
0.000000
0.000000
1.000000
0.000000
1.000000
1.000000
-1.000000
-125.000000
125.000000
6.000000
3628800.000000
61.000000
```
