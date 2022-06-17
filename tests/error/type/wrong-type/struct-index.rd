// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:13: type error, incompatible type `int` for struct index expession, must be a struct\n

pub fn foo(a: int) {
    let b = a.b;
}
