// test: Compiler should give type error
// stderr: = error[E0012]: 4:14-21: type error, definition of invalid type `[2]TypeA`\n

type TypeA = [2]TypeA;

