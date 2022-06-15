// test: Comments are ignored by the lexer

// Line comments cover the whole line
pub fn /* Block comments have to be ended */ main /* Can be anywhere */ (): int {
    return /*
        Can also be multiline. /*
            And nested. /* Arbitrarily many times. */
        */
    */ 0;
}

