// test: Test constant expressions with arrays

const a: [4]int = [1, 2, 3, 4];
const b: int = a[0];
const c: int = a[b];
const d: int = a[c];
const e: int = a[d];
const f: int = a[a[0] + a[1]];
const g: int = a[a[0] + 2 * b];

pub fn main(): int {
    if (b != 1 || c != 2 || d != 3 || e != 4) {
        return 1;
    }
    if (f != 4) {
        return 2;
    }
    if (g != 4) {
        return 2;
    }
    return 0;
}

