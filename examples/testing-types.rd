
type TypeA = [5]*TypeA;
type TypeB = *[5]TypeB;

fn main(test: i32): int {
    let a: *TypeA;
    let b: TypeB;
    a = b;
}

