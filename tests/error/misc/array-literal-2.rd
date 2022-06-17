// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:21-26: inconsistent length of array literal, expected literal of type `[3]int`\n

pub fn foo() {
    let a: [3]int = [0, 5];
}
