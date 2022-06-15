// test: Void type acts as a zero length tuple/struct

fn foo(a: (), ..): () {
    return ();
}

pub fn main(): int {
    let a: () = ();
    let b = a;
    let c = foo(a, b);

    return 0;
}

