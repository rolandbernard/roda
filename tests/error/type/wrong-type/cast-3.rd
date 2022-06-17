// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:13-24: type error, unsupported cast form `fn (f64): ()` to `usize`\n

pub fn foo(a: f64) {
    let b = foo as usize;
}
