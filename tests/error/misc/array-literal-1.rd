// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:18-23: type error, conflicting types `int` and `[2]i64`\n

pub fn foo() {
    let a: int = [0, 5];
}
