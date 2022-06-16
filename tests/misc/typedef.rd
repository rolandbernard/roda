// test: Typedefs should world

type TypeA = [2]int;
type TypeB = *TypeB;

pub fn main(): int {
    let a: TypeA;
    a[0] = 1;
    a[1] = 2;
    if a[0] != 1 && a[1] != 2 {
        return 1;
    }
    let b: TypeB;
    return 0;
}
