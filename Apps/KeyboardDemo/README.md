# PocuterUtil::Keyboard -- Keyboard Demo Application
**This application is an advanced usage reference for the [PocuterUtil::Keyboard](/Libs/Keyboard) utility class and demonstrates the following concepts:**

- How to use multiple keyboard objects in a single application
- How to bind a keyboard to a configuration file for data persistance
- How to create a 'smart' keyboard that reacts to user input in real-time
- General code flow for an application which uses a keyboard

## Application Usage
**The application has three keyboards available, which are triggered using the input buttons:**

- **Button A:** Opens a hexadecimal 'smart' keyboard that displays the input as a color swatch in real-time
- **Button B:** Opens an full keyboard with default text
- **Button C:** Opens an IP address keyboard

**After selecting '[OK]' the resulting text is displayed at the bottom of the screen**