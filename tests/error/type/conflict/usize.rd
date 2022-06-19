// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:12: type error, conflicting types `uint` and `usize`\n

pub fn foo(a: uint): usize {
    return a;
}
