// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:43: type error, expecting expression of type `f64` but found integer literal\n

pub fn foo() {
    let a: (a: int, b: f64) = (a = 0, b = 5);
}
