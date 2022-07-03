// test: Test constant variables

const x: int = 3;
const y: f64 = 3.5;
const z = 5;

pub fn main(): int {
    let a = x;
    if a != 3 {
        return 1;
    }
    let b = y;
    if b != 3.5 {
        return 2;
    }
    let c = z;
    if c != 5 {
        return 3;
    }
    if z != 5 || x + 2 != z || y != 3.5 {
        return 4;
    }
    return 0;
}
