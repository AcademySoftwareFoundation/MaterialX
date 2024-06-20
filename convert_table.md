
| from\to | float   | color3      | color4      | vector2     | vector3     | vector 4    | integer          | boolean      |
|---------|---------|-------------|-------------|-------------|-------------|-------------|------------------|--------------|
| float   | (no-op) | convert     | convert     | convert     | convert     | convert     | floor/ceil/round | ifgreater/eq |
| color3  | extract | (no-op)     | convert     | MISSING (1) | convert     | MISSING (1) | ????             | ????         |
| color4  | extract | convert     | (no-op)     | MISSING (1) | MISSING (1) | convert     | ????             | ????         |
| vector2 | extract | MISSING (1) | MISSING (1) | (no-op)     | convert     | MISSING (1) | ????             | ????         |
| vector3 | extract | convert     | MISSING (1) | convert     | (no-op)     | convert     | ????             | ????         |
| vector4 | extract | MISSING (1) | convert     | MISSING (1) | convert     | (no-op)     | ????             | ????         |
| integer | convert | MISSING (2) | MISSING (2) | MISSING (2) | MISSING (2) | MISSING (2) | (no-op)          | MISSING (3)  |
| boolean | convert | MISSING (2) | MISSING (2) | MISSING (2) | MISSING (2) | MISSING (2) | MISSING (3)      | (no-op)      |

