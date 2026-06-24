# 🛠️ Compilers Projects

> Two compiler implementations built from scratch in C — from a simple expression compiler to a full Sub-Pascal → C transpiler.

---

## 📖 About

This repository contains two compiler construction projects developed as part of a university Compilers course. Each project progressively deepens engagement with formal language theory and compiler design: the first builds a minimal expression compiler from first principles, while the second implements a complete multi-phase compiler pipeline — scanner, predictive parser, type checker, and C code generator — for a statically-typed, Pascal-derived language called Sub-Pascal.

Both projects are implemented in **C**, operating on real file I/O via command-line arguments, and produce correct output on well-formed programs while gracefully handling lexical and syntactic errors.

---

## 📁 Repository Structure

```
Compilers_projects/
├── SimpleExpressionCompiler/    # Project 1 — File-driven expression compiler with extended grammar
└── SubPascalToC/                # Project 2 — Full Sub-Pascal → standard C transpiler
```

---

## 🗂️ Projects

### 1. `SimpleExpressionCompiler` — Extended Expression Compiler

**Objective:** Design and implement a simple compiler for arithmetic expressions with an extended grammar, file-based I/O, and command-line invocation.

**Key Features:**
- Extended grammar based on Chapter 2 of the course textbook, accepting the program structure:
  ```
  program ID(input, output)
  {
      // list of expressions
  }
  ```
- Added keywords: `program`, `input`, `output`
- Added operator `%` (same precedence as `*` and `/`)
- Added `#` as a comment-line marker (entire line ignored by the scanner)
- Reads source from an input file and writes postfix output to an output file
- Invoked via command-line arguments: `sc infile.inf outfile.pos`

**Example:**

Input (`infile.inf`):
```
program test(input,output)
{
    123+sum \ 15;
    # This is a comment line
    12 div 3;
    # Another comment line
    num % 4 +2;
}
```

Output (`outfile.pos`):
```
program test(input,output)
{
    123 sum 15 \ + ;
    12 3 DIV;
    num 4 % 2 +;
}
```

**Usage:**
```bash
sc infile.inf outfile.pos
```

---

### 2. `SubPascalToC` — Sub-Pascal to Standard C Transpiler

**Objective:** Implement a complete multi-phase compiler for Sub-Pascal, a statically-typed, case-insensitive subset of Pascal, targeting standard C as the output language.

**Language: Sub-Pascal**

Sub-Pascal supports integer and real variables, assignment, arithmetic and relational expressions, `if-then-else`, `repeat-until`, and `writeln`. It excludes arrays, records, and procedures for simplicity.

**Compiler Phases Implemented:**
- **Scanner (Lexer):** Tokenizes identifiers, unsigned numbers (integer + real with optional fractional and exponent parts), reserved keywords, relational operators (`=`, `>`, `<`, `>=`, `<=`, `<>`), addops (`+`, `-`, `OR`), mulops (`*`, `/`, `DIV`, `MOD`, `AND`), and `assignop` (`:=`).
- **Parser:** Predictive (LL(1)) recursive-descent parser. The grammar was first rewritten to eliminate left recursion and left-factor all productions to make it amenable to top-down parsing.
- **Type Checker:** Semantic actions verify expression type compatibility (`integer` vs. `real`); implicit coercion and type mismatch errors are reported.
- **Code Generator:** Emits a syntactically valid standard C program, mapping Sub-Pascal constructs as follows:

| Sub-Pascal | Generated C |
|---|---|
| `VAR x, y: integer;` | `int x, y;` |
| `VAR a, b: real;` | `float a, b;` |
| `x := expr` | `x = expr;` |
| `if E then S else S` | `if(E) S else S` |
| `repeat S until E` | `do { S } while !(E);` |
| `writeln(E)` | `printf("%d\n", E);` or `printf("%f\n", E);` |

**Rewritten LL(1) Grammar:** The original left-recursive grammar was transformed into a non-left-recursive, left-factored form (included in the project submission) before implementing the recursive-descent parser.

**Example:**

Input (Sub-Pascal):
```pascal
PROGRAM Example(Input, Output);
VAR
    X, Y, Z: integer; A, B: real;
BEGIN
    X := 20;
    Y := 10;
    IF X > Y THEN
        Z := X + Y;
    ELSE
        Z := Y;
    REPEAT
        Z := X - Y;
        X := X - 2
    UNTIL Z < 0;
    WRITELN(Z)
END.
```

Generated Output (standard C):
```c
#include<stdio.h>
int X, Y, Z; float A, B;
int main(void)
{
    X = 20; Y = 10;
    if(X > Y)
        Z = X + Y;
    else
        Z = Y;
    do {
        Z = X - Y;
        X = X - 2;
    } while !(Z < 0);
    printf("%d\n", Z);
    return 0;
}
```

---

## 🏷️ Keywords

`compiler` `compiler-design` `compiler-construction` `lexer` `scanner` `parser` `recursive-descent` `predictive-parser` `LL1` `code-generation` `transpiler` `Sub-Pascal` `Pascal` `C` `postfix` `expression-compiler` `type-checking` `formal-languages` `grammar` `university-project`

---

## 🧰 Tech Stack

- **Language:** C
- **Paradigm:** Procedural, hand-written (no parser generator tools)
- **I/O:** File-based, command-line argument driven
- **Tools:** `gcc`, standard C library only

---

## 🚀 Getting Started

### Compile

```bash
gcc -o sc SimpleExpressionCompiler/sc.c
# or
gcc -o subpascal SubPascalToC/compiler.c
```

### Run

```bash
# Project 1
sc infile.inf outfile.pos

# Project 2
subpascal input.pas output.c
```

---

## 👩‍💻 Author

**Sarah Abu Irmeileh**
Palestine Polytechnic University — Computer Science, Class of 2025