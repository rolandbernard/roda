// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:13-21: type error, unsupported cast form `f64` to `*int`\n

pub fn foo(a: f64) {
    let b = a as *int;
}
