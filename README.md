# Solar Oven 
This is part of the design and implementation for a solar oven for senior design. The goal is to track sun and concentrate sunlight to heat something up.
I am in charge of sensing temperature and creating the UI.

# Software
The microcontroller we will be using is a Raspberry Pi Pico W. I will be using it to read and store temperature, get and send data to a touchscreen, and log the sensor data to AdaFruit IO. 
I also plan on using an API to get the weather to give you an idea on how well the oven may perform under certain weather conditions.


# Project Structure and Code Overview
 - ### Build System
     - Uses CMake which is the recommended approach for the Raspberry Pi Pico
     - Can be compiled without wifi support (useful for developing and testing)
         - ```bash
            cmake -DENABLE_WIFI=0 -DCMAKE_BUILD_TYPE=Debug ..
            ```
  - ### File Structure
      - Top directory &rarr; CMakeLists.txt, README, and .gitignore
      - `src/` directory &rarr; Contains all of the code
          - `src/wconfig` directory &rarr; Holds configuration files for LWIP and mbed TLS
      - `fonts/` directory &rarr; Contains fonts from the AdaFruitGFX library
  - ### Files
      - `src/main.c`
        - Initializes all the subsystems and sets up the main drawing loop
      - `src/display.c`
        - Sets up the driver for the LCD
        - Handles some basic primitive rendering
        - Handles text rendering
           - The text rendering function was modified from [AdaFruitGFX DrawChar](https://github.com/adafruit/Adafruit-GFX-Library/blob/ac6d7c3869a693d406f77b9bfcd486b0673169f0/Adafruit_GFX.cpp#L1391) 
        
     - `src/ui.c`
       - Handles the placement of all text and buttons, along with the button callback functions
     - `src/debug.c`
          - Sets up some useful debug functions that will get optimized out during release builds
     - `src/sensors.c`
          - Sets up the PIO state machine along with the ADC
          - Defines interrupt handlers for the state machine and ADC
          - Converts the correspoding input to resistance then to temperature
     - `src/pulsewidth.pio`
          - Pio program that triggers a 555 Monostable circuit and measures the corresponding pulsewidth
          - Pushes the measured pulse width to the FIFO buffer and triggers an interrupt
     - `src/state.h`
          - Stores the global system state
     - `src/pins.h`
          - Pin Mapping header for all peripherals
     - `src/wifi.c`
          - Handles connecting to Wifi and getting the connection status
     - `src/ntp.c`
          - Syncs the RTC with an NTP server
          - Making the NTP request was done with the help of code from the [pico-examples repo](https://github.com/raspberrypi/pico-examples/blob/master/pico_w/wifi/ntp_client/picow_ntp_client.c)
     - `src/https.c`
          - Handles making HTTPS requests
          - This code was taken and modified from [LWIP HTTP Client](https://github.com/lwip-tcpip/lwip/blob/77dcd25a72509eb83f72b033d219b1d40cd8eb95/src/apps/http/http_client.c)
            - The orignal code could only handle GET Requests. I had to perform some modifications to perform POST Requests to upload temperature
            - I also removed a lot of unused settings checks and the synchronous functions API
     - `src/adafruit.c`
          - Handles making the HTTPS Request to adafruit IO
          - Modified some of the code from the [pico examples repo](https://github.com/raspberrypi/pico-examples/blob/master/pico_w/wifi/http_client/example_http_client_util.c)
     - `src/touchscreen.c`
          - Sets up the touchscreen driver
          - Code was converted from [Python to C](https://www.bisonacademy.com/ECE476/Code/28%20gt911.txt)

# Feature Outline
- [ ] Temperature Sensing
  - [x] Hardware
    - [x] 555 Monostable Circuit
    - [x] Voltage Divider into Op amp  
  - [ ] Software
    - [x] Monostable PIO code
    - [x] ADC Code
    - [ ] Calibration

- [ ] Display (Touchscreen - ST7796)
  - [x] Graphics Driver
  - [x] Touch Driver (GT911)

  - [x] UI Features
    - [x] Text Renderer 
    - [x] Temperature Display
    - [x] Timer Display
    - [x] Current Time Display
    - [ ] ~Weather Display~

  - [ ] Settings
    - [x] Toggle °C / °F
    - [x] Set Time (UTC Offset)
    - [ ] Manual Time Sync
    - [ ] Diagnostic Viewer
    - [ ] Store values in flash

  - [x] Status Indicators
    - [x] WiFi Status

- [x] WiFi
  - [x] Connect to Internet
  - [x] Sync Time (NTP) on boot
  - [x] Send Data to Adafruit (HTTP)
  - [ ] ~Fetch Weather (OpenWeatherMap HTTPS)~


# Build Instructions (May not be complete)
## Clone the Raspberry Pi SDK
```bash
git clone https://github.com/raspberrypi/pico-sdk.git --branch master
cd pico-sdk
git submodule update --init
```
## Set the SDK path environment variable
```bash
export PICO_SDK_PATH=/full/path/to/pico-sdk
```
## Install the dependencies
### Macos
```zsh
# Install Homebrew first if not installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake ninja arm-none-eabi-gcc
xcode-select --install  # provides system gcc/g++
```
### Arch Linux
```bash
sudo pacman -Syu cmake gcc arm-none-eabi-gcc ninja
```
## Download the project
```bash
git clone https://github.com/Brady-Brandt/solar_oven.git
cd solar_oven
mkdir build
```
### Build the Project
```bash
cd build
# set wifi to 0 to disable wifi and change Release to debug to get printing
cmake -DENABLE_WIFI=1 -DCMAKE_BUILD_TYPE=Release ..
make -j4
```
