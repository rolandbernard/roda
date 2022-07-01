// test: Compiler should find a type conflict
// stderr: = error[E0021]: 6:15: no field with name `b` in struct type `(a: int, c: f64)`\n

pub fn foo() {
    let a: (a: int, c: f64);
    let b = a.b;
}
