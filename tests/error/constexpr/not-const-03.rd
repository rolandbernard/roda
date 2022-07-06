// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0009]: 9:13-17: call expression not allowed in constant expressions\n

fn foo(): int {
    return 0;
}

pub fn main(): int {
    let b: [foo()]int;
    return 0;
}
