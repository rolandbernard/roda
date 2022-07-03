// test: Test constant variables

pub fn main(): int {
    let a: [3]int;
    let b: [x]int = a;
    let c: [y as int]int = a;
    let d: [z - 2]int = a;
    return 0;
}

const z: int = x * 2 - 1;
const x: int = 3;
const y: f64 = x as f64 + 0.5;

