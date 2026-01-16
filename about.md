# Infinite Inputs

Tired of being limited to just 6 input triggers in your Geometry Dash levels? This mod unlocks full keyboard and mouse support!

## Features

- **Complete Keyboard Support**: Letters, numbers, function keys, modifiers, and more
- **Full Mouse Support**: All mouse buttons including side buttons
- **Mouse Wheel**: Scroll up/down actions
- **Cursor Tracking**: Make objects follow your mouse cursor

## Usage

This mod works with **text labels** using the following format:

`@{keyName} {keyDown} = {groupID}`



### Parameters

#### `keyName` (string)
The identifier for the key or input. Must be written in exact **camelCase** as listed below. Case-sensitive.

*Examples:* `f`, `enter`, `4`, `f5`, `leftShift`

#### `keyDown` (number)
`1` for key press (down), `0` for key release (up)

#### `groupID` (number)
The group ID to activate when the condition is met

## Examples

**Trigger group 93 when F is pressed:**

`@f 1 = 93`


**Trigger group 94 when F is released:**

`@f 0 = 94`


## Special Keys

### Mouse Wheel

Wheel actions (`wheelUp`, `wheelDown`) don't use the `keyDown` parameter.

**Format:** `@{wheelUp|wheelDown} = {groupID}`

**Trigger group 3 on wheel up:**

`@wheelUp = 3`


**Trigger group 2 on wheel down:**

`@wheelDown = 2`


### Cursor Follow

Makes all objects in the specified group follow the mouse cursor.

**Format:** `@cursor = {groupID}`

**Example:**

`@cursor = 4`


## Key Reference

### Function Keys
`f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12`

### Number Row
`1, 2, 3, 4, 5, 6, 7, 8, 9, 0`

### Letters (QWERTY Layout)

q, w, e, r, t, y, u, i, o, p
a, s, d, f, g, h, j, k, l
z, x, c, v, b, n, m


### Special Keys
`enter, space, escape, backspace`

### Modifiers
`leftCtrl, leftShift, leftAlt, rightCtrl, rightShift, rightAlt`

### Mouse Buttons
`leftMouse, rightMouse, middleMouse, mouse3, mouse4`

### Mouse Wheel
`wheelUp, wheelDown`

### Cursor
`cursor`

## Notes

- All key names are case-sensitive and must be exact
- Only one action per text label
- Group IDs must be valid numbers in your level
- Geometry Dash defaults to QWERTY, regardless of operating system settings and there is no way to work around that at the moment 


# Trouble Shooting

Enable console in the geode console, as there are quite many logs to help pinpoint issues.

# Bug Reports / Feature Request

File them on [Github Issues](https://github.com/iAndyHD3/InfiniteInputs/issues/new)
Or join the [Discord Server](discord.gg/7kyvwJVcqJ)