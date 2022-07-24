// test: Test constant expressions with arrays

const a: [4]int = [1, 2, 3, 4];
const b: [0]int = [];
const c: [10]int = [1, 2, 3, 4, 5, 6, 7, 8, 9, 0];

pub fn main(): int {
    if (a[0] != 1 || a[1] != 2 || a[2] != 3 || a[3] != 4) {
        return 1;
    }
    let d = a;
    if (d[0] != 1 || d[1] != 2 || d[2] != 3 || d[3] != 4) {
        return 2;
    }
    let e = c;
    if (e[0] != 1 || e[4] != 5 || e[9] != 0) {
        return 3;
    }
    let f: [0]int = b;
    return 0;
}

