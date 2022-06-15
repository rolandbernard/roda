// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:5-10: type error, expected a return value of type `int`\n

pub fn foo(): int {
    return;
}
