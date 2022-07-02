// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:13: type error, incompatible type `int` for tuple index expession, must be a tuple\n

pub fn foo(a: int) {
    let b = a.0;
}
