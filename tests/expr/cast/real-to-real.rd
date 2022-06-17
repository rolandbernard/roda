// test: Conversions should be possible

pub fn main(): int {
    let a: f32 = 12.75;
    let b: f64 = 12.75;
    if a != b as f32 {
        return 1;
    }
    let c: f32 = -12.25;
    let d: f64 = -12.25;
    if c as f64 != d {
        return 2;
    }
    return 0;
}
