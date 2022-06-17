// test: Test moving around structs

type MyStruct = (a: [2]int, b: f32, c: [4]int);

fn foo(a: MyStruct, b: int, c: int): (a: int, b: f64) {
    a.a[c] += b;
    return (a = a.a[c], b = a.b as f64);
}

pub fn main(): int {
    let a = (a = [1, 2], b = 2.25, c = [1, 2, 3, 4]);
    let b = foo(a, 5, 1);
    if b.a != 7 || a.b != 2.25 {
        return 1;
    }
    return 0;
}
