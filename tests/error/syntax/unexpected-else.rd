// test: File should not compile with syntax error
// stderr: = error[E0001]: 5:5-8: syntax error, unexpected `else`\n

pub fn foo(): int {
    else {
        return 0;
    }
}

