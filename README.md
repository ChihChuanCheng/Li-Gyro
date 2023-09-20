# Li-Gyro Open Source
## Introduction
Li-Gyro is a flight controller which supports light-weight self-stablization aircraft[^1]. It is powered by a 1s li-po battery (4.2v). Figure 1 shows the configuration of Li-Gyro flight controller. It is featured by ESP8266 micro processor, and thus can be connected via WiFi. With equipped by an external antenna, the communication range can reach up to 100 meters, which has been verified in many field tests. It is also featured by MPU6050 to enable self-stablization for aircraft. It allows beginners to enjoy this hobby. Regarding actuators, Li-Gyro is equipped by 2 DC motors and 2 servos. It is thus suitable for aircraft with differential thrust, fixed wing aircraft, and delta wing with mixture control. Finally, it has flash mode to enable flexibility for hobbists to customize their own flight controller.
Regarding transmitters, Li-Gyro can be directly controlled by smartphone with V7RC installed ([Android](https://play.google.com/store/apps/details?id=com.v7idea.v7rcliteandroidsdkversion&hl=zh_TW&gl=US), [iPhone](https://apps.apple.com/tw/app/v7rc/id1390983964)). Alternatively, it can also be controlled by Gamepad.

[^1]: Many thanks to Nicholas Rehm for giving permission to this work to extend [dRhemFlight](https://github.com/nickrehm/dRehmFlight) from VTOL to light-weight self-stabilization aircraft.

![Li-Gyro specification](https://github.com/ChihChuanCheng/Li-Gyro/blob/main/Li-GyroFlightController[en].png)

**Figure 1. Configuration of Li-Gyro**
 

## Introduction to open sources
### Receiver
#### Li-Gyro flight controller
Li-Gyro is the open source for Li-Gyro flight controller. It mainly receives control commands (encoded with V7RC protocol) transmitted from smartphone or gamepad, adjusts control commands based on current orientation of the aircraft, and configure adjusted control commands to motors.
### Transmitter
#### V7RC app
V7RC app is introduced in [V7RC](https://hackmd.io/@accomdemy/v7rc).
#### V7RC gamepad
To allow gamepad to connect to V7RC app (in gamepad mode) via bluetooth, which in turn connects to Li-Gyro via WiFi to deliver control command. In this way, gamepad is able to take the advantage of V7RC app for parameter adjustment, e.g, trimming.
#### WiFi gamepad
To allow gamepad to directly connect to Li-Gyro via WiFi. The advantage of this control mode is simplicity.

## Applications
**Wright Flyer (with differential thrust)**

**Hovercraft (with differential thrust)**

**Fixed wing aircraft**

**RC ornithopter**

## Disclaimer
This code is a shared, open source flight controller for small micro aerial vehicles and is intended to be modified to suit your needs. It is NOT intended to be used on manned vehicles. I do not claim any responsibility for any damage or injury that may be inflicted as a result of the use of this code. Use and modify at your own risk. More specifically put:
THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

