// test: File should not compile with syntax error
// stderr: = error[E0001]: 5:13-13: syntax error, unexpected invalid token, expecting `;`\n

pub fn foo() {
    let hellöworld;
}

