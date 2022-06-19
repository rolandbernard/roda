// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:18-33: type error, conflicting types `int` and `(a = i64, c = f64)`\n

pub fn foo() {
    let a: int = (a = 0, c = 5.5);
}
