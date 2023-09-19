# Li-Gyro開源程式
## 簡介
Li-Gyro是一片支援輕量自穩飛行器的飛控板 。其供電電池為1s鋰電池(4.2V)。搭載ESP8266微處理器，以WiFi為控制媒介，操作距離約莫100公尺。另配備MPU6050，提供陀螺儀自穩功能，能在飛行器偏離預期航道時，自動修正偏差航道，將飛行器拉回正常航道上。驅動器方面，支援2顆空心杯直流馬達、2顆伺服馬達，可應用於雙馬達(差速)遙控飛機、固定翼遙控飛機、三角翼混控飛機和氣墊船等載體。最後，其具備燒錄功能，讓玩家可自行客製化飛控板程式。

## Li-Gyro機板介紹
圖 1為Li-Gyro機板配置，從背面來看，其搭載ESP8266，以WiFi為控制媒介，並以外接天線的方式延伸控制距離，實際測試控制距離可達約100公尺，例如400公尺操場皆在控制範圍內。從正面來看，其內嵌MPU6050陀螺儀，能夠偵測飛行載體的飛行姿態，如與預期飛行姿態有偏差時，則可予以修正，驅動器支援2顆空心杯直流馬達、2顆伺服馬達，具有燒錄功能，可隨時修改飛控板程式延伸其應用範圍。
遙控器方面，可使用手機安裝V7RC(Android, iPhone)來操控Li-Gyro機板，或是以實體手把來操控皆可，相關的開源程式如下介紹。

![Li-Gyro specification](https://github.com/ChihChuanCheng/Li-Gyro/blob/main/Li-GyroFlightController.png)


**圖 1 Li-Gyro機板配置**
 

## 開源程式介紹
### 接收器
#### Li-Gyro	Li-Gyro飛控板程式
Li-Gyro為飛控板開源程式，其主程式專門接收符合V7RC協定的控制信令，透過陀螺儀的PID控制系統，為飛行器修正飛行姿態，以便將其鎖定在期待的航道上，最後再將修正完的控制命令下達給馬達，完成控制。
### 遙控器
#### V7RC app
V7RC app介紹請參考(https://hackmd.io/@accomdemy/v7rc)。
#### Gamepad_V7RC	V7RC gamepad程式
以藍芽連接手機V7RC app，再利用V7RC以WiFi連接Li-Gyro飛控板，其好處是可利用豐富的V7RC的功能，如調整遙桿校正、遙控範圍限定等。
#### Gamepad_WiFi	WiFi gamepad程式
Gamepad_WiFi: 直接以WiFi連接Li-Gyro飛控板，其好處是設定較為單純，可快速完成連線，享受飛行。

## 相關應用
**雙馬達(差速)萊特機**

**雙馬達(差速)氣墊船**

**固定翼衝浪者**

**電控仿生鳥**

## 免責聲明
本程式為輕量自穩飛行器的飛控板開源程式，並可被玩家修改以符合其使用需求。它不可被使用於載人的飛行器上。本人不負使用本程式時所造成的損害和人員損傷，請使用者自負使用或修改本程式所造成的後果。

THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

