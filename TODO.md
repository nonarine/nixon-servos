# TODO List

## Known Issues

### Repeat Command with Scripts
- **Issue**: The repeat command doesn't work quite right with scripts
- **Details**: When using `repeat 3 sweep` where `sweep` is a script name, the execution may not work as expected
- **Current Status**: Basic script name detection is implemented but needs refinement
- **Priority**: Medium

## Future Enhancements

### Script System
- Improve script-to-script calling reliability
- Add better error handling for nested script execution
- Consider script execution timeout mechanisms

### Command Sequencing
- Optimize command sequence memory usage
- Add ability to pause/resume command sequences
- Implement command sequence debugging tools