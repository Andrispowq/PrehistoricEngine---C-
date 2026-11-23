# Coding rules

- **Treat the codebase as yours.** Read the surrounding code, understand who relies on it, and verify the behavior you change.
- **Deduplicate.** When you see the same or nearly the same logic written out multiple times, give it one implementation and update the callers to use it. For near-duplicates, identify any intentional differences and preserve them. For example, both rendering backends use the same tone-mapping function, while each backend handles its own render targets.
- **Attach pointer and reference symbols to the type.** Write `int* pointer`, `int& reference`, and `const Camera& camera`. Use the same spacing in function parameters and return types.
- **Use braces and put the opening brace on a new line.** An `if` may omit braces when the entire statement fits on one line, as in `if (!camera) return;`. An empty loop may keep its braces on the same line, as in `while (true) {}`. Preserve the surrounding indentation.

- Keep backend-independent shader math in `PrehistoricEngine/res/shaders/shared/`. Include it from the OpenGL and Vulkan entry points. Keep stage interfaces, descriptor layouts and API-specific code in the corresponding backend tree.
- Stage built-in shaders, fonts, primitive models and default textures from `PrehistoricEngine/res/`. Reference built-in models and textures with `engine://`. Place shared demonstration models, sounds and textures in `samples/res/` and use `sample://`. Keep settings, worlds, scripts and unique assets in the owning application's `res/` tree. Resolve portable IDs in loaders and stage shared resources through `scripts/resources/CMakeLists.txt`.
