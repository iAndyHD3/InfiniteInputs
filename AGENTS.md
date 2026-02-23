
# Rules

- NEVER change the format string specified in TextParsing.cpp. It is key that these stay the same
- NEVER use git commands unless explicitly asked to do some work that needs it.

## Building

Once you have finished a batch of your changes you can try to get the list of errors from the LSP if you have a tool for that. If not, the command to test a build is always the following:
`cmake --build build`

When you are completely done with your work and the previous commands succeeds, run the following to open geometry dash.: 
`geode run --background`