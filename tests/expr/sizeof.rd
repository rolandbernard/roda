// test: Sizeof returns the allocation size of the type

pub fn main(): int {
    if sizeof(int) != 8 {
        return 1;
    }
    if sizeof([16]u8) != 16 {
        return 2;
    }
    if sizeof([0]u8) != 0 {
        return 3;
    }
    if sizeof(()) != 0 {
        return 3;
    }
    return 0;
}
