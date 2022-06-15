// test: Compiler should find a type conflict
// stderr: = error[E0011]: 5:12-14: type error, expecting expression of type `*u8` but found character literal\n

pub fn foo(a: int): *u8 {
    return 'a';
}
