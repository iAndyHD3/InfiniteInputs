# Infinite Inputs

Are you bored of only having 6 or whatever limited number of inputs for your geometry dash levels?

Wait no more: This mod adds keyboard and mouse support!!!! 

- Keyboard support (letters, number rows, function keys, modifiers)
- All mouse buttons
- Cursor support, damm how cool is that?

# Usage

it works with **text labels**

the format is precisely the following 
`@{keyName} {keyDown} = {groupID}

## keyName
type: string
The special key specifier, look below for a list.
The casing is camelCase and it must be exact for every key.
Examples: `f`, `g`, `enter`, `4`, `3`, `f5`, `leftShift`

## keyDown
type: number
1 for key down, 0 for key up

## groupID
type: number
The group ID that should be spawned

# Example

This spawns group 93 when the key F is pressed down
@f 1 = 93

This spawns group 94 when the key F is lifted up
@f 0 = 93

# Secial Keys
Mouse scrollwheel keys: `wheelUp` and `wheelDown` have no key down or up number in its format.
Example:
Format: @{wheelUp/Down} = {groupID}

spawn group 3 on wheel up
@wheelUp = 3 

spawn group 2 on wheel down
@wheelDown = 2

# Cursor Follow
Will make any objects in that group follow the cursor
example:
@cursor = 4


# Key List


f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,

1, 2, 3, 4, 5, 6, 7, 8, 9, 0,

q,w,e,r,t,z,u,i,o,p,

a,s,d,f,g,h,j,k,l,

y,x,c,v,b,n,m,

enter,

space,

leftCtrl,

leftShift,

leftAlt,


leftMouse,

rightMouse,

middleMouse,

mouse3,

mouse4,


wheelUp,

wheelDown,


cursor