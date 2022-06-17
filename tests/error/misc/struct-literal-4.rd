// test: Compiler should find a type conflict
// stderr: = error[E0020]: 5:28: duplicate definition of struct field named `a`\n

pub fn foo() {
    let a = (a = 0, b = 4, a = 5);
}
