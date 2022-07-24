// test: Test constant expressions with tuple

const a: (int, int) = (1, 2);
const b: (i32, f64, i32) = (1, 4.2, 10);
const c: int = a.0;
const d: int = a.1;
const e: i32 = b.0;
const f: f64 = b.1;
const g: i32 = b.2;

pub fn main(): int {
    if (c != 1 || d != 2) {
        return 1;
    }
    if (e != 1 || f != 4.2 || g != 10) {
        return 2;
    }
    return 0;
}

