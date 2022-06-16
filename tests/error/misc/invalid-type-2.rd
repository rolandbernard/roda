// test: Compiler should give type error
// stderr: = error[E0012]: 4:14-18: type error, definition of invalid type `TypeA`\n

type TypeA = TypeA;

