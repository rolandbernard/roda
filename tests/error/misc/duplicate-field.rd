// test: Compiler should give type error
// stderr: = error[E0020]: 4:31: duplicate definition of struct field named `a`\n

type TypeA = (a: int, b: f32, a: int);
