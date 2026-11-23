# Coding rules

- **Treat the codebase as yours.** Read the surrounding code, understand who relies on it, and verify the behavior you change.
- **Deduplicate.** When you see the same or nearly the same logic written out multiple times, give it one implementation and update the callers to use it. For near-duplicates, identify any intentional differences and preserve them. For example, both rendering backends use the same tone-mapping function, while each backend handles its own render targets.
- **Attach pointer and reference symbols to the type.** Write `int* pointer`, `int& reference`, and `const Camera& camera`. Use the same spacing in function parameters and return types.
- **Use braces and put the opening brace on a new line.** An `if` may omit braces when the entire statement fits on one line, as in `if (!camera) return;`. An empty loop may keep its braces on the same line, as in `while (true) {}`. Preserve the surrounding indentation.
