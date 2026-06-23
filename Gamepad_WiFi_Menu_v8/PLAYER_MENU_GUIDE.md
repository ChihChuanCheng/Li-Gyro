# ESP32 航模遙控器 v8 玩家選單設定教學

這份文件給實際使用遙控器的玩家參考。內容以 OLED 選單操作為主，不需要理解程式碼。

## 快速安全提醒

- 第一次測試請不要接馬達或螺旋槳。
- 解鎖前，左搖桿 Y 油門必須在最低。
- `SWA` 是油門切斷，按下時油門強制輸出最低。
- 每次換模型後，都要重新確認通道方向、油門方向與油門切斷。

## 控制模式畫面

開機後會進入控制模式。

OLED 顯示格式：

```text
第 1 行：WiFi 狀態
第 2 行：模型名稱 + LOCK/ARM + LOW/HIGH
第 3 行：T:油門值 C:油門切斷
第 4 行：E:Expo LR:LowRate
```

範例：

```text
WiFi:OFF
TRAINER LOCK HIGH
T:1000 C:OFF
E:30% LR:70%
```

狀態說明：

- `WiFi:OFF`：目前沒有成功連上 WiFi。
- 顯示 SSID：代表已成功連上該 WiFi。
- `LOCK`：尚未解鎖，油門會被強制最低。
- `ARM`：已解鎖。
- `LOW`：目前使用 Low Rate。
- `HIGH`：目前使用 High Rate。
- `C:ON`：SWA 油門切斷啟動。
- `C:OFF`：油門切斷關閉。

## 通道配置

目前遙控器是左手油門模式。

```text
CH1 = Rudder   = 左搖桿 X
CH2 = Throttle = 左搖桿 Y
CH3 = Elevator = 右搖桿 Y
CH4 = Aileron  = 右搖桿 X
CH5-CH8 = 預留通道
```

`CH2 Throttle` 不會套用 Rate/Expo，避免油門行程變形。

## 解鎖與油門切斷

開機後預設 `LOCK`。

如果油門不是最低，OLED 會顯示：

```text
THROTTLE HIGH
```

請先把左搖桿 Y 拉到最低。

解鎖步驟：

1. 左搖桿 Y 拉到最低。
2. 同時按下兩個搖桿按鈕。
3. OLED 從 `LOCK` 變成 `ARM`。

解鎖是一次性的。解鎖後不會再用按鍵切回 `LOCK`，重新開機才會回到 `LOCK`。

油門切斷：

- 按下 `SWA`：`C:ON`，CH2 強制 `1000`。
- 放開 `SWA`：`C:OFF`，CH2 依左搖桿 Y 輸出。

## 進入主選單

在控制模式下：

1. 左搖桿打到左下角。
2. 同時按下 `SWA` 和 `SWB`。
3. OLED 顯示 `Entering Menu Mode...`。

主選單操作：

- 右搖桿 Y：上下移動游標。
- 右搖桿 X 向右：確認。
- 右搖桿 X 向左：返回或離開。

主選單項目：

```text
SSID SCAN MODE
PID TUNING MODE
CHANNEL SETUP
CHANNEL MONITOR
MODEL SETUP
RATE/EXPO SETUP
TESTING MODE
CALIBRATION MODE
EXIT
```

## 建議第一次設定順序

建議照這個順序設定：

1. `CALIBRATION MODE`
2. `MODEL SETUP`
3. `CHANNEL SETUP`
4. `CHANNEL MONITOR`
5. `RATE/EXPO SETUP`
6. `SSID SCAN MODE`
7. 回控制模式解鎖測試

## SSID SCAN MODE

用途：設定遙控器要連線的 WiFi。

可接受的 SSID 開頭：

- `Wright`
- `Hover`
- `Drone`

操作：

1. 進入 `SSID SCAN MODE`。
2. 等待掃描完成。
3. 用右搖桿 Y 選擇 SSID。
4. 右搖桿 X 向右確認。
5. 進入密碼輸入畫面。

密碼輸入：

- 右搖桿 Y：切換目前字元。
- 右搖桿 X 向右：下一格。
- 右搖桿 X 向左：退格。
- `SWB`：儲存並連線。

注意：控制畫面第一行只有在真的連線成功時才會顯示 SSID。若沒有連線成功，會顯示 `WiFi:OFF`。

## PID TUNING MODE

用途：調整接收端或飛控使用的 PID 參數。

可調項目：

```text
Roll  P / I / D
Pitch P / I / D
Yaw   P / I / D
```

操作：

- 右搖桿 Y：選擇項目或調整數值。
- 右搖桿 X 向右：進入/離開編輯。
- 右搖桿 X 向左：返回。
- `SWA`：切換調整步進。
- `SWB`：重設目前項目為預設值。

到 `EXIT` 並確認後會儲存並送出 PID 封包。

如果你還沒有接收端 PID 功能，可以先不用設定這個選單。

## CHANNEL SETUP

用途：設定每個通道的方向、舵量、中立點與端點。

第一層：選擇通道

```text
CH1
CH2
CH3
CH4
CH5
CH6
CH7
CH8
SAVE & EXIT
```

操作：

- 右搖桿 Y：選擇通道。
- 右搖桿 X 向右：進入通道設定。
- 右搖桿 X 向左：返回主選單。
- `SAVE & EXIT`：儲存並回控制模式。

第二層：通道設定

```text
REV
RATE
SUB
LOW
HIGH
BACK
```

項目說明：

- `REV`：通道反向，`N` 正向，`R` 反向。
- `RATE`：通道總舵量，範圍 `30%` 到 `125%`。
- `SUB`：中立點微調，範圍 `-250` 到 `+250`。
- `LOW`：低端點比例，範圍 `30%` 到 `125%`。
- `HIGH`：高端點比例，範圍 `30%` 到 `125%`。
- `BACK`：回到通道選擇頁。

建議調整方式：

1. 先用 `CHANNEL MONITOR` 看方向是否正確。
2. 如果方向反了，回 `CHANNEL SETUP` 改 `REV`。
3. 如果中立點偏掉，用 `SUB` 微調。
4. 如果行程太大或太小，用 `LOW/HIGH` 調端點。
5. 全部調完後回第一層選 `SAVE & EXIT`。

## CHANNEL MONITOR

用途：查看目前 CH1-CH8 的實際輸出值。

操作：

- 右搖桿 Y：切換頁面。
- 右搖桿 X 向左：返回主選單。

頁面：

```text
第 1 頁：CH1-CH4
第 2 頁：CH5-CH8
```

常見用途：

- 確認 CH1 Rudder 是否跟左搖桿 X 動。
- 確認 CH2 Throttle 是否跟左搖桿 Y 動。
- 確認 CH3 Elevator 是否跟右搖桿 Y 動。
- 確認 CH4 Aileron 是否跟右搖桿 X 動。
- 確認 `SWA` 油門切斷時 CH2 是否回到 `1000`。
- 確認 Rate/Expo 是否影響 CH1/CH3/CH4。

## MODEL SETUP

用途：選擇模型記憶與模型名稱。

目前支援 3 組模型：

```text
MODEL 1
MODEL 2
MODEL 3
```

每組模型會保存：

- CH1-CH8 通道設定
- Low Rate
- High Rate
- Expo
- 模型名稱

模型名稱可選：

```text
TRAINER
SPORT
GLIDER
DELTA
BOAT
CUSTOM
```

選擇模型：

1. 進入 `MODEL SETUP`。
2. 用右搖桿 Y 選模型。
3. 右搖桿 X 向右確認。
4. 系統會載入該模型並回控制模式。

設定模型名稱：

1. 先選好目前模型。
2. 再進入 `MODEL SETUP`。
3. 移到 `NAME:`。
4. 右搖桿 X 向右進入編輯。
5. 右搖桿 Y 切換名稱。
6. 右搖桿 X 向右或向左離開並儲存。

## RATE/EXPO SETUP

用途：設定操控手感。

項目：

- `LOW RATE`：按下 `SWB` 時使用。
- `HIGH RATE`：放開 `SWB` 時使用。
- `EXPO`：搖桿中心附近柔化。

目前 Rate/Expo 只套用：

```text
CH1 Rudder
CH3 Elevator
CH4 Aileron
```

不套用：

```text
CH2 Throttle
```

建議初始值：

```text
LOW RATE : 60% - 70%
HIGH RATE: 100%
EXPO     : 20% - 35%
```

使用方式：

- 起飛、降落、新手練習：按住 `SWB` 使用 Low Rate。
- 高空測試或需要大動作：放開 `SWB` 使用 High Rate。

## TESTING MODE

用途：檢查硬體輸入是否正常。

可查看：

- `SWA / SWB`
- 左搖桿按鈕
- 右搖桿按鈕
- 左右搖桿 X/Y 值
- MPU6050 姿態角

搖桿值會顯示為 `1000-2000`。

## CALIBRATION MODE

用途：校正搖桿中心點與最大/最小行程。

操作：

1. 進入 `CALIBRATION MODE`。
2. 兩支搖桿放回中心。
3. 按 `SWB` 記錄中心點。
4. 將四個搖桿軸慢慢推到所有最大與最小端點。
5. 再按一次 `SWB` 儲存。

畫面顯示值會和 Testing Mode 一樣是 `1000-2000`。

建議：

- 每次更換搖桿硬體後都重新校正。
- 如果中心點常常不是 `1500` 附近，也重新校正。

## EXIT

用途：離開主選單，回到控制模式。

## 常見問題

### 開機後顯示 THROTTLE HIGH

左搖桿 Y 油門不是最低。把左搖桿 Y 拉到底後再解鎖。

### 顯示 WiFi:OFF

代表目前沒有成功連上 WiFi。請進入 `SSID SCAN MODE` 重新設定 SSID 與密碼。

### 顯示模型名稱但不是我想要的

進入 `MODEL SETUP`，選目前模型後，移到 `NAME:` 修改名稱。

### 舵面方向反了

進入 `CHANNEL SETUP`，選對應通道，修改 `REV`。

### 油門不動

請確認：

1. OLED 是否仍是 `LOCK`。
2. 是否 `C:ON`，如果是，放開 `SWA`。
3. 是否 CH2 在 `CHANNEL MONITOR` 中有跟左搖桿 Y 變化。

### Low Rate 沒影響油門

這是正常的。Rate/Expo 不套用 CH2 油門，只套用 Rudder、Elevator、Aileron。
