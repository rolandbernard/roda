// test: Variables are scoped to blocks

pub fn main(): int {
    let a = 42;
    {
        let a = 12;
        if true {
            let a = 4;
            if a != 4 {
                return 1;
            }
        }
        if a != 12 {
            return 2;
        }
    }
    if a != 42 {
        return 3;
    }
    return 0;
}

