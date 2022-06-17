// test: Compiler should find a type conflict
// stderr: = error[E0021]: 5:31-46: inconsistent fields in struct literal, expected literal of type `(a = int, b = f64)`\n

pub fn foo() {
    let a: (a: int, b: f64) = (a = 0, c = 5.5);
}
