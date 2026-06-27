# JSON Library (Arc)

The Arc-based `json` module provides tools to parse and interact with JSON strings.

## Functions

### `to_json(str)`
- **Description:** Parses a JSON string and returns the corresponding Arc object (e.g., a list of key-value pairs).
- **Parameters:** `str` - A JSON-formatted string.

### `json_get_value(map, key)`
- **Description:** Given a parsed JSON map, retrieves the value associated with a specific key.
- **Parameters:** `map` - A parsed JSON map, `key` - The key to look up.
