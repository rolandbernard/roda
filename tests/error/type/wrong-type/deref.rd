// test: Compiler should find a type conflict
// stderr: = error[E0011]: 5:9: type error, unable to infer the type of variable `b`\n

pub fn foo(a: int) {
    let b = &a;
}
