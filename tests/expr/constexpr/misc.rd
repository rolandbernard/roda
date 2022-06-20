// test: Test constant expressions

pub fn main(): int {
    let a: [3]int;
    let b: [1 + 2]int = a;
    let c: [1 | 2]int = a;
    let d: [3 & 7]int = a;
    let e: [7 - 4]int = a;
    let f: [7 * 4 / 8]int = a;
    let g: [(7.8 * 4.2 / 8.9) as usize]int = a;
    let h: [(7.8 * 4.2 / 8.9) as u8]int = a;
    let i: [(7.8 * 4.2 / 8.9) as isize]int = a;
    let j: [(7.8 * 4.2 / 8.9) as i8]int = a;
    let k: [7 + -4]int = a;
    return 0;
}
