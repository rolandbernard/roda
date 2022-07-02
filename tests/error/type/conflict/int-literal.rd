// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:12-13: type error, expecting expression of type `&u8` but found integer literal\n

pub fn foo(a: int): &u8 {
    return 42;
}
