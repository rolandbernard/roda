// test: Compiler should find a type conflict
// stderr: = error[E0001]: 5:27: syntax error, unexpected `;`, expecting `else`\n

pub fn foo(a: int): int {
    return if a == 0 { 5 };
}
