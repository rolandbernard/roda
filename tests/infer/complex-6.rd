// test: Test moving around structs and tuples

fn foo(): u8 {
    let o;
    let n;
    let l;
    let h;
    let g;
    let d;
    let f = [d];
    let e = [f[0]];
    let b = (g, e[0], e[1]);
    let a = (b.2, [e[0] + g]);
    let c = (b, a.1[1]);
    let j;
    let i = ([[[[c.1]]], h], &j);
    let k = (a = &i, b = &j);
    j = &k;
    let m = [l, j];
    n = **(*(*m[1]).a).1;
    o = *(*(*n.b)).a;
    let p = **i.1;
    return (*(*(*p.b)).a).0[0][0][0][0];
}

pub fn main(): int {
    foo(); // Return value of this is undefined
    return 0;
}

