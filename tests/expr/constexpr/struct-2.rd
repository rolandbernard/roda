// test: Test constant expressions with structs

const a: (a: int, b: int) = (a = 1, b = 2);
const b: (a: f32, b: int) = (b = 1, a = 2.5);
const c: (a: i32, b: f64, c: i32) = (b = 4.2, c = 10, a = 1);
const d: int = a.a;
const e: int = a.b;
const f: f32 = b.a;
const g: int = b.b;
const h: i32 = c.a;
const i: f64 = c.b;
const j: i32 = c.c;

pub fn main(): int {
    if (d != 1 || e != 2) {
        return 1;
    }
    if (f != 2.5 || g != 1) {
        return 2;
    }
    if (h != 1 || i != 4.2 || j != 10) {
        return 3;
    }
    return 0;
}

