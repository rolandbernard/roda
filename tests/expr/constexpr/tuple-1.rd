// test: Test constant expressions with tuple

const a: (int, int) = (1, 2);
const b: () = ();
const c: (i32, f64, i32) = (1, 4.2, 10);

pub fn main(): int {
    if (a.0 != 1 || a.1 != 2) {
        return 1;
    }
    let d = a;
    if (a.0 != 1 || a.1 != 2) {
        return 2;
    }
    if (c.0 != 1 || c.1 != 4.2 || c.2 != 10) {
        return 3;
    }
    let e = c;
    if (e.0 != 1 || e.1 != 4.2 || e.2 != 10) {
        return 4;
    }
    let f: () = b;
    return 0;
}

