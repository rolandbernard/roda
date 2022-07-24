// test: Test constant variables

pub fn main(): int {
    const x: usize = 5 - 2;
    const y: isize = x as isize;
    const z: usize = x * 2 - 1;

    let a: [3]int;
    let b: [x]int = a;
    let c: [y as int]int = a;
    let d: [z - 2]int = a;
    return 0;
}

