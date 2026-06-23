# ESP32 航模遙控器 v8 設定教學

本文件適用於 `Gamepad_WiFi_Menu_v8`。

## 1. 控制模式畫面

控制模式 OLED 顯示：

```text
第 1 行：WiFi 實際連線 SSID，未連線顯示 WiFi:OFF
第 2 行：模型名稱 + LOCK/ARM + LOW/HIGH
第 3 行：T:油門值 C:油門切斷狀態
第 4 行：E:Expo LR:LowRate
```

目前使用左手油門：

```text
CH1 = Rudder   = 左搖桿 X
CH2 = Throttle = 左搖桿 Y
CH3 = Elevator = 右搖桿 Y
CH4 = Aileron  = 右搖桿 X
```

## 2. 安全鎖與油門檢查

開機後預設 `LOCK`，油門通道 CH2 會被強制輸出 `1000`。

如果開機時油門不在最低位，OLED 會顯示：

```text
THROTTLE HIGH
```

此時不能解鎖。請先把左搖桿 Y 拉到最低。

解鎖方式：

1. 左搖桿 Y 油門最低。
2. 同時按下兩個搖桿按鈕。
3. OLED 從 `LOCK` 變成 `ARM`。

解鎖是一次性的：解鎖後不會再用按鍵回到 `LOCK`，重新開機才會回到 `LOCK`。

`SWA` 是油門切斷：

- `C:ON`：CH2 強制 `1000`
- `C:OFF`：CH2 依左搖桿 Y 輸出

## 3. 進入主選單

在控制模式下：

1. 左搖桿打到左下角。
2. 同時按下 `SWA` 和 `SWB`。
3. OLED 顯示 `Entering Menu Mode...` 後進入主選單。

主選單：

1. `SSID SCAN MODE`
2. `PID TUNING MODE`
3. `CHANNEL SETUP`
4. `CHANNEL MONITOR`
5. `MODEL SETUP`
6. `RATE/EXPO SETUP`
7. `TESTING MODE`
8. `CALIBRATION MODE`
9. `EXIT`

操作：

- 右搖桿 Y：上下移動
- 右搖桿 X 向右：確認
- 右搖桿 X 向左：返回

## 4. WiFi 設定

進入 `SSID SCAN MODE` 後會掃描 WiFi。

接受的 SSID 開頭：

- `Wright`
- `Hover`
- `Drone`

密碼編輯：

- 右搖桿 Y：切換目前字元
- 右搖桿 X 向右：下一格
- 右搖桿 X 向左：退格
- `SWB`：儲存並連線

控制畫面第一行只有在真的連線成功時才會顯示 SSID；未連線時顯示 `WiFi:OFF`。

## 5. Channel Setup

進入 `CHANNEL SETUP` 後，先選通道，再調整該通道。

第一層：

- `CH1` 到 `CH8`：進入該通道設定
- `SAVE & EXIT`：儲存並回到控制模式

第二層：

- `REV`：通道反向
- `RATE`：通道總舵量，`30%` 到 `125%`
- `SUB`：中立點微調，`-250` 到 `+250`
- `LOW`：低端點比例，`30%` 到 `125%`
- `HIGH`：高端點比例，`30%` 到 `125%`
- `BACK`：返回通道選擇頁

## 6. Channel Monitor

進入 `CHANNEL MONITOR` 可查看 CH1-CH8 目前輸出。

- 右搖桿 Y：切換 `CH1-CH4` / `CH5-CH8`
- 右搖桿 X 向左：返回主選單

這個畫面適合檢查通道方向、Subtrim、Endpoint、Rate/Expo 是否正確。

## 7. Model Setup

目前支援 3 組模型記憶：

- `MODEL 1`
- `MODEL 2`
- `MODEL 3`

每組模型保存：

- CH1-CH8 通道設定
- Low Rate
- High Rate
- Expo
- 模型名稱

設定模型名稱：

1. 進入 `MODEL SETUP`。
2. 選擇要設定的模型，使其成為目前模型。
3. 再進入 `MODEL SETUP`。
4. 移到 `NAME:`。
5. 右推進入編輯。
6. 用右搖桿 Y 切換名稱。
7. 右推或左推離開並儲存。

可選名稱：

- `TRAINER`
- `SPORT`
- `GLIDER`
- `DELTA`
- `BOAT`
- `CUSTOM`

## 8. Rate / Expo

進入 `RATE/EXPO SETUP` 可設定目前模型的舵量曲線。

- `LOW RATE`：按下 `SWB` 時使用
- `HIGH RATE`：放開 `SWB` 時使用
- `EXPO`：中心附近柔化

`Rate/Expo` 不會套用到 CH2 油門，只套用在 CH1 Rudder、CH3 Elevator、CH4 Aileron。

建議初始值：

```text
LOW RATE: 60% - 70%
HIGH RATE: 100%
EXPO: 20% - 35%
```

## 9. Calibration Mode

進入 `CALIBRATION MODE` 後：

1. 將兩支搖桿放回中心。
2. 按下 `SWB` 記錄中心點。
3. 將四個搖桿軸慢慢推到所有最大與最小端點。
4. 再按一次 `SWB` 儲存 min / center / max 校正值。

畫面顯示值會和 Testing Mode 一樣是 `1000-2000`。

## 10. Testing Mode

進入 `TESTING MODE` 可查看：

- `SWA / SWB` 狀態
- 左搖桿按鈕與 X/Y 數值
- 右搖桿按鈕與 X/Y 數值
- MPU6050 姿態角

## 11. 首次設定建議

1. 不要接馬達或螺旋槳。
2. 先做 `CALIBRATION MODE`。
3. 進入 `MODEL SETUP` 選模型與名稱。
4. 進入 `CHANNEL SETUP` 設定方向與端點。
5. 用 `CHANNEL MONITOR` 檢查 CH1-CH8。
6. 設定 `RATE/EXPO SETUP`。
7. 設定 WiFi。
8. 回控制模式確認 `WiFi:OFF` 或正確 SSID。
9. 左搖桿 Y 油門最低，按兩個搖桿按鈕解鎖。
10. 確認 `SWA` 可切斷 CH2 油門。

## 12. 安全提醒

- 解鎖前，油門必須最低。
- 第一次測試請拆槳。
- 接馬達前，務必確認 CH2 油門方向正確。
- `SWA` 應視為緊急油門切斷鍵。
- 每次更換模型後，都要重新確認通道方向與油門切斷。
