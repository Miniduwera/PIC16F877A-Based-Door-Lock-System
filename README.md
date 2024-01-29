# PIC16F877A-Based-Door-Lock-System

Overview:
This repository contains the source code for a microcontroller-based Door Lock System. The system is designed to secure a door by controlling an electronic lock mechanism. It incorporates a keypad for user input, an LCD for displaying information, and a motor to actuate the lock.

Features:
Keypad Input: The system uses a keypad for user input. Users can enter a predefined password to unlock the door.

LCD Display: A Liquid Crystal Display (LCD) is employed to provide feedback and prompts to the user. It displays messages such as "Password Please," "Correct Password," and "Invalid Password."

Motor Control: A motor is used to control the locking mechanism of the door. The system can lock or unlock the door based on user input and predefined conditions.

Password Security: The system supports a password-based security mechanism. Users are required to enter a correct password to unlock the door. The code includes functionality for handling incorrect password attempts and implementing delays for security purposes.

Bluetooth Integration: The code includes provisions for Bluetooth functionality, allowing users to connect their smartphones to the system for additional features.

EEPROM Storage: The system utilizes Internal Electrically Erasable Programmable Memory (EEPROM) to store and retrieve password information.

Buzzer Feedback: A buzzer provides audible feedback to the user, indicating successful or unsuccessful attempts, and system status.

Usage:
Setup: Connect the microcontroller to the keypad, LCD, motor, Bluetooth module, and other relevant components based on the hardware configuration.

Compile and Upload: Use an appropriate Integrated Development Environment (IDE) to compile the code and upload it to the microcontroller.

Operation: Upon startup, the system prompts the user to enter the password via the keypad. Follow the on-screen instructions for password entry. The system will lock or unlock the door based on the entered password.

Bluetooth Integration (Optional): If Bluetooth functionality is enabled, follow the instructions to connect a smartphone to the system for additional features.
After 5 failed attempts you can use blutooth to reset the system.

Contributing:
Contributions to this project are welcome. Feel free to open issues for bug reports or feature requests. If you've made improvements, please submit a pull request.

License:
This Door Lock System code is released under the MIT License.

