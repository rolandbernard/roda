// test: File should not compile with syntax error
// stderr: = error[E0001]: 5:12: syntax error, unexpected invalid token, expecting `;`\n

pub fn foo(): u8 {
    return 'hello';
}

