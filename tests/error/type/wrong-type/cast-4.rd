// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:13-23: type error, unsupported cast form `f64` to `[2]int`\n

pub fn foo(a: f64) {
    let b = a as [2]int;
}
