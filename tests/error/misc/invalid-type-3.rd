// test: Compiler should give type error
// stderr: = error[E0012]: 4:14-31: type error, definition of invalid type `(b = int, a = TypeA)`\n

type TypeA = (b: int, a: TypeA);

