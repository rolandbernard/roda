// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:21: type error, expecting expression of type `(int)` but found integer literal\n

pub fn foo(): int {
    let a: (int) = (5);
    return 0;
}
