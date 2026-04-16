# Solar Oven 
This is part of the design and implementation for a solar oven for senior design. The goal is to track sun and concentrate sunlight to heat something up.
I am in charge of sensing temperature and creating the UI.

# Software
The microcontroller we will be using is a Raspberry Pi Pico W. I will be using it to read and store temperature, get and send data to a touchscreen, and log the sensor data to AdaFruit IO. 
I also plan on using an API to get the weather to give you an idea on how well the oven may perform under certain weather conditions.

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

  - [ ] UI Features
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

- [ ] WiFi
  - [x] Connect to Internet
  - [x] Sync Time (NTP) on boot
  - [ ] Send Data to Adafruit (HTTP)
  - [ ] ~Fetch Weather (OpenWeatherMap HTTPS)~
