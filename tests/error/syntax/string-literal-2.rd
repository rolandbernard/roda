// test: File should not compile with syntax error
// stderr: = error[E0005]: 5:19-24: string literal contains invalid escape character\n

pub fn foo(): *u8 {
    return "Hello \u12ag";
}

