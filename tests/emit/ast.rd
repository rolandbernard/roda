// test: Test emiting ast
// build: LLVM_PROFILE_FILE="profile/tests/%%p.profraw" ./build/$BUILD/bin/rodac $(realpath --relative-to=. %) -o=%.ast $TEST_ARGS
// run: cmp %.ast %.expected
// cleanup: rm %.ast

type Char = i32;

extern fn getchar(): Char;
extern fn printf(fmt: *u8, ..): i32;
extern fn malloc(size: usize): *();
extern fn free(ptr: *());

type Scanner = (peek: Char, pos: int);

fn peek_token(scan: *Scanner): Char {
    if (&scan).peek == 0 {
        let c = getchar();
        (&scan).pos += 1;
        while c == ' ' || c == '\t' {
            c = getchar();
            (&scan).pos += 1;
        }
        (&scan).peek = c;
    }
    return (&scan).peek;
}

fn next_token(scan: *Scanner): Char {
    let c = peek_token(scan);
    (&scan).peek = 0;
    return c;
}

type Node = (op: Char, val: int, childs: [2]*Node);

fn create_value_node(val: int): *Node {
    let n = malloc(sizeof Node) as *Node;
    (&n).op = 'C';
    (&n).val = val;
    (&n).childs = [0 as *Node, 0 as *Node];
    return n;
}

fn create_binary_node(op: Char, left: *Node, right: *Node): *Node {
    let n = malloc(sizeof Node) as *Node;
    (&n).op = op;
    (&n).childs = [left, right];
    return n;
}

fn free_node(node: *Node) {
    if node != 0 as *Node {
        free_node((&node).childs[0]);
        free_node((&node).childs[1]);
        free(node as *());
    }
}

fn parse_expr(scan: *Scanner): *Node {
    return parse_add_sub(scan);
}

fn parse_binary_left_right(scan: *Scanner, ops: [2]Char, next: *fn (*Scanner): *Node): *Node {
    let l = (&next)(scan);
    while l != 0 as *Node && (peek_token(scan) == ops[0] || peek_token(scan) == ops[1]) {
        let op = next_token(scan);
        let r = (&next)(scan);
        if r == 0 as *Node {
            free_node(l);
            l = r;
        } else {
            l = create_binary_node(op, l, r);
        }
    }
    return l;
}

fn parse_add_sub(scan: *Scanner): *Node {
    return parse_binary_left_right(scan, ['+', '-'], *parse_mul_div);
}

fn parse_mul_div(scan: *Scanner): *Node {
    return parse_binary_left_right(scan, ['*', '/'], *parse_pow);
}

fn parse_pow(scan: *Scanner): *Node {
    let l = parse_base(scan);
    if l != 0 as *Node && peek_token(scan) == '^' {
        next_token(scan);
        let r = parse_pow(scan);
        if r == 0 as *Node {
            free_node(l);
            l = r;
        } else {
            l = create_binary_node('^', l, r);
        }
    }
    return l;
}

fn parse_base(scan: *Scanner): *Node {
    let c = peek_token(scan);
    if c >= '0' && c <= '9' {
        let v = (c - '0') as int;
        next_token(scan);
        c = peek_token(scan);
        while c >= '0' && c <= '9' {
            v *= 10;
            v += (c - '0') as int;
            next_token(scan);
            c = peek_token(scan);
        }
        return create_value_node(v);
    } else if c == '(' {
        next_token(scan);
        let n = parse_expr(scan);
        if peek_token(scan) != ')' {
            free_node(n);
            return 0 as *Node;
        } else {
            next_token(scan);
            return n;
        }
    } else {
        return 0 as *Node;
    }
}

fn pow(x: int, e: int): int {
    if e == 0 {
        return 1;
    } else if e % 2 == 1 {
        return x * pow(x*x, e / 2);
    } else {
        return pow(x*x, e / 2);
    }
}

fn eval_node(node: *Node): int {
    if (&node).op == '+' {
        return eval_node((&node).childs[0]) + eval_node((&node).childs[1]);
    } else if (&node).op == '-' {
        return eval_node((&node).childs[0]) - eval_node((&node).childs[1]);
    } else if (&node).op == '*' {
        return eval_node((&node).childs[0]) * eval_node((&node).childs[1]);
    } else if (&node).op == '/' {
        return eval_node((&node).childs[0]) / eval_node((&node).childs[1]);
    } else if (&node).op == '^' {
        return pow(eval_node((&node).childs[0]), eval_node((&node).childs[1]));
    } else {
        return (&node).val;
    }
}

pub fn main(): int {
    let exit = false;
    while !exit {
        let scan = (peek = 0, pos = 0);
        if peek_token(*scan) == 'q' {
            exit = true;
        } else {
            let node = parse_expr(*scan);
            if peek_token(*scan) == '\n' && node != 0 as *Node {
                next_token(*scan);
                printf(" = %li\n", eval_node(node));
                free_node(node);
            } else {
                free_node(node);
                if peek_token(*scan) <= 4 {
                    exit = true;
                } else {
                    while scan.pos > 1 {
                        printf(" ");
                        scan.pos -= 1;
                    }
                    printf("^ syntax error\n");
                    while next_token(*scan) != '\n' {
                        // Skip to start of next line
                    }
                }
            }
        }
    }
    return 0;
}

