// test: Test constant expressions with structs

const a: (a: int, b: int) = (a = 1, b = 2);
const b: (a: f32, b: int) = (b = 1, a = 2.5);
const c: (a: i32, b: f64, c: i32) = (b = 4.2, c = 10, a = 1);

pub fn main(): int {
    if (a.a != 1 || a.b != 2) {
        return 1;
    }
    let d = a;
    if (a.a != 1 || a.b != 2) {
        return 2;
    }
    if (c.a != 1 || c.b != 4.2 || c.c != 10) {
        return 3;
    }
    let e = c;
    if (e.a != 1 || e.b != 4.2 || e.c != 10) {
        return 4;
    }
    let f: (a: f32, b: int) = b;
    if (f.a != 2.5 || f.b != 1) {
        return 5;
    }
    return 0;
}

