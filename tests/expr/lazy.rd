// test: Test lazy comparison

extern fn exit(status: int);

fn donteval(a: bool): bool {
    exit(10);
    return a;
}

pub fn main(): int {
    if !(true && (false || true)) {
        return 1;
    }
    if !(true && (true || donteval(false))) {
        return 2;
    }
    if false && (donteval(true) || donteval(false)) {
        return 3;
    }
    if !(true || (donteval(true) && donteval(true))) {
        return 4;
    }
    if !(false || (true && true)) {
        return 5;
    }
    return 0;
}
