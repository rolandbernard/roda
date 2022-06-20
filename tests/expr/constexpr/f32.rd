// test: Test constant expressions with booleans

pub fn main(): int {
    let a: [3]int;
    let b: [(1.5 as f32 + 1.5) as int]int = a;
    let c: [(7.7 as f32 - 4.3) as int]int = a;
    let d: [(7.0 as f32 * 0.5) as int]int = a;
    let e: [(0.01 as f32 / 0.003) as int]int = a;
    return 0;
}
