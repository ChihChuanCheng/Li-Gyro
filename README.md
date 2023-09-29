# Li-Gyro
## Introduction to Hardware [[Chinese version]](https://github.com/ChihChuanCheng/Li-Gyro/blob/main/Li-Gyro[cn].pdf)
Li-Gyro is a flight controller which supports light-weight self-stabilization aircraft[^1]. Figure 1 shows the configuration of Li-Gyro flight controller. Li-Gyro is powered by 1s lipo battery (4.2V). It integrates all required functionalities into a PCB to simplify aircraft build. To make it more beginner friendly, ESP8266 is adopted as main processor, which can be controlled directly by smartphone via WiFi. In other words, a beginner does not need to purchase an additional transmitter for remote control. To eliminate the concern of communication range, an external antenna is pluged on ESP8266 to extend control range up to 100 meters, which is far enough for a light-weight aircraft. Moreover, Li-Gyro is featured by MPU6050 to enable self-stabilization. An aircraft with self-stabilization can automatically fix a bias from desired flight path. It thus reduces the gap for a beginner to join this hobby. To be able to support a variety of aircrafts, 2 DC motors and 2 servos are installed on Li-Gyro, allowing it to support differential thrust, fixed wing aircraft, and delta wing with mixture control. Finally, by enabling flash mode on PCB, users can modify code to customize Li-Gyro for their needs. 

[^1]: Many thanks to Nicholas Rehm for giving permission to this work to extend [dRhemFlight](https://github.com/nickrehm/dRehmFlight) from VTOL to light-weight self-stabilization aircraft.

![Li-Gyro specification](https://github.com/ChihChuanCheng/Li-Gyro/blob/main/Li-GyroFlightController[en].png)

**Figure 1. Configuration of Li-Gyro**

Regarding transmitter, as described above, Li-Gyro can be controlled by smartphone with V7RC installed ([Android](https://play.google.com/store/apps/details?id=com.v7idea.v7rcliteandroidsdkversion&hl=zh_TW&gl=US), [iPhone](https://apps.apple.com/tw/app/v7rc/id1390983964)). Nevertheless, for user who loves physical touch of joystick while flying a RC aircraft, a gamepad is also provided as an option. Figure 2 shows the configuration of a gamepad. Gamepad is powered by 14500 lipo battery (4.2V). ESP32 is selected as the main processor, because it supports both bluetooth and WiFi simultaneously. Four buttons can be configured on the gamepad, which are SWA, SWB, button S on left joystick, and button P on right joystick. Joystick is designed for lefthand throttle. In other words, left joystick is mainly for throttle and rudder control, while right joystick is mainly for elevator and aileron control. For standalone mode, OLED is used as panel to display gamepad status.

![Gamepad specification](https://github.com/ChihChuanCheng/Li-Gyro/blob/main/Gamepad[en].png)

**Figure 2. Configuration of a Gamepad**

## Introduction to Software
### Receiver
#### Li-Gyro flight controller [[Chinese version]](https://github.com/ChihChuanCheng/Li-Gyro/blob/main/Li-Gyro/Li-Gyro[Code][cn].pdf) [[English version]](https://github.com/ChihChuanCheng/Li-Gyro/blob/main/Li-Gyro/Li-Gyro[Code][en].pdf)
Li-Gyro is the code for Li-Gyro flight controller. It receives control commands (encoded by V7RC protocol) sent from smartphone or gamepad, tunes control commands to make the aircraft stick to desired its desired flying path, and configure the modified control commands to actuators.
### Transmitter
#### Smartphone with V7RC app installed
The introduction of V7RC app can be found in [V7RC](https://hackmd.io/@accomdemy/v7rc).
#### Gamepad for V7RC app [[Chinese version]](https://github.com/ChihChuanCheng/Li-Gyro/blob/main/Gamepad_V7RC/Gamepad_V7RC[cn].pdf) [[English version]](https://github.com/ChihChuanCheng/Li-Gyro/blob/main/Gamepad_V7RC/Gamepad_V7RC[en].pdf) [[demo]](https://youtu.be/-qkeLL97hfQ)
Gamepad_V7RC is the code for gamepad to connect, via bluetooth, to V7RC app in gamepad mode, which in turn connects to Li-Gyro flight controller via WiFi to realize remote control. In this way, gamepad can take the advantage of V7RC app for parameter adjustment, e.g, joystick trimming.
#### Gamepad for WiFi [[Chinese version]](https://github.com/ChihChuanCheng/Li-Gyro/blob/main/Gamepad_V7RC/Gamepad_WiFi[cn].pdf) [[English version]](https://github.com/ChihChuanCheng/Li-Gyro/blob/main/Gamepad_V7RC/Gamepad_WiFi[en].pdf)
Gamepad_Wifi is the code for gamepad to connect directly to Li-Gyro flight controller via WiFi. The benefit for this control mode is simplicity.

## Applications
**Wright Flyer (with differential thrust) [[Chinese version]](https://github.com/ChihChuanCheng/Li-Gyro/blob/main/Wright_Flyer/Wright_Flyer[cn].pdf) [[English version]](https://github.com/ChihChuanCheng/Li-Gyro/blob/main/Wright_Flyer/Wright_Flyer[en].pdf) [[demo]](https://youtu.be/dp7WQTxrX8g)**

Wright Flyer is a good airplane for beginner to step into the flying RC hobby. It is a two-motor airplane with differential thrust. General speaking, it is a 2-channel airplane, where one channel for throttle control and the other channel for direction control. It is easy to build, easy to fly, and, the most important is strong enough to resist crashes. 

**Hovercraft (with differential thrust) [[Chinese version]](https://github.com/ChihChuanCheng/Li-Gyro/blob/main/Hovercraft/Hovercraft[cn].pdf) [[English version]](https://github.com/ChihChuanCheng/Li-Gyro/blob/main/Hovercraft/Hovercraft[en].pdf) [[demo]](https://youtu.be/Aum4P2JLwZg)**

**Fixed wing aircraft**

**RC ornithopter**

## Disclaimer
This code is a shared, open source flight controller for small micro aerial vehicles and is intended to be modified to suit your needs. It is NOT intended to be used on manned vehicles. I do not claim any responsibility for any damage or injury that may be inflicted as a result of the use of this code. Use and modify at your own risk. More specifically put:

THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

