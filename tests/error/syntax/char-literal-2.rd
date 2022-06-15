// test: File should not compile with syntax error
// stderr: = error[E0005]: 5:13-18: character literal contains invalid character\n

pub fn foo(): u8 {
    return '\uwesd';
}

