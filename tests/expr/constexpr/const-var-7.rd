// test: Test constant variables

pub fn main(): int {
    const x: int = {
        const y: int = 5;
        const z: int = {
            const a: u32 = 1;
            const b: f64 = 42.7;
            const c: int = 7;
            b as int / c + (2 * a) as int 
        };
        z - y
    };
    const y: f64 = x as f64 + 0.5;
    const z: int = x * 2 - 1;

    let a: [3]int;
    let b: [x]int = a;
    let c: [y as int]int = a;
    let d: [z - 2]int = a;
    return 0;
}

