// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0023]: 5:49: in constant expression, recursive constant reference to `x`\n

const x: (a: int, c: int, e: int) = (a = 1, c = y.b, e = 5);
const y: (b: int, d: int, f: int) = (b = 5, d = x.e, f = 1);

pub fn main(): int {
    return 0;
}
