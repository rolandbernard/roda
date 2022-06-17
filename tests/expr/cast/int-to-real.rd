// test: Conversions should be possible

pub fn main(): int {
    let a: u64 = 12;
    let b: f64 = 12.0;
    if a as f64 != b {
        return 1;
    }
    let c: i64 = -12;
    let d: f32 = -12.0;
    if c as f32 != d {
        return 2;
    }
    return 0;
}
