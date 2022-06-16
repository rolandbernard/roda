// test: Test moving around array type data

fn foo(a: [5]int, b: int, c: int): [5]int {
    a[c] += b;
    return a;
}

pub fn main(): int {
    let x: [0]int = ();
    let y = [1, 2, 3,];
    let z = [1];
    let a: [5]int = [5, 4, 3, 2, 1];
    if a[0] != 5 || a[1] != 4 || a[2] != 3 || a[3] != 2 || a[4] != 1 {
        return 2;
    }
    a[0] = 1;
    a[1] = 2;
    a[2] = 3;
    a[3] = 4;
    a[4] = 5;
    let b = a[3];
    let c = a[1];
    let d = foo(a, b, c);
    if d[0] != 1 || d[1] != 2 || d[2] != 7 || d[3] != 4 || d[4] != 5 || foo(a, b, c)[2] != 7 {
        return 1;
    }
    let e = [a[0] + a[1], a[2] + a[3]];
    if e[0] != 3 || e[1] != 7 {
        return 3;
    }
    return 0;
}
