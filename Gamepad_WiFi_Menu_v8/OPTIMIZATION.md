# Li-Gyro ESP32 Gamepad WiFi Remote — 優化方案記錄

## 專案位置
`/home/charleyclaw/Li-Gyro-gamepad/Gamepad_WiFi_Menu/`

## 已知狀態
- 所有 5 個 Bug（Debounce、WiFi String、OLED 閃爍、EEPROM 無驗證、Magic Number）已修復 ✅
- tester 驗證全部通過 ✅

---

## 優化方向（待執行）

### 1. 記憶體優化
| 項目 | 說明 | 複雜度 |
|------|------|--------|
| 減少全域 `String` | 目前全域變數較多，檢查並減少 `String` 使用 | 低 |
| 整理全域變數 | 可整理進 struct 減少全域污染 | 低 |

### 2. 效能優化
| 項目 | 說明 | 複雜度 |
|------|------|--------|
| I2C 速度優化 | 確認是否為 400kHz，可提升到 1MHz | 低 |
| 減少 `digitalWrite()` | 用陣列批量處理 GPIO 減少函式呼叫開銷 | 中 |

### 3. 重構
| 項目 | 說明 | 複雜度 |
|------|------|--------|
| 抽取 `types.h` | `wifi.h` 和 `utility.h` 循環依賴，抽共用型別到獨立檔案 | 中 |
| 任務分離 | WiFi 和 OLED 可用 FreeRTOS task 分離，提高回應性 | 高 |
| 模組化 | 將 PID、WiFi、OLED 做成獨立 library 供其他專案使用 | 中 |

### 4. 功能增強
| 項目 | 說明 | 複雜度 |
|------|------|--------|
| OTA 更新 | 加入 WiFi OTA 功能，支援遠端更新韌體 | 中 |
| 電量顯示 | 讀取並顯示鋰電池電壓（若有偵側） | 低 |
| 多遙控器支援 | 掃描並連接多個飛控，切換目標 | 中 |

---

## 建議優先順序

1. **I2C 速度優化** — 簡單有效
2. **減少全域 `String`** — 記憶體穩定
3. **抽取 `types.h`** — 解決架構問題
4. **OTA 更新** — 實用性高，日後燒錄方便

---

## 備註

- 專案為 ESP32 平台，使用 Arduino framework
- 主要硬體：ESP32、OLED (SSD1306)、搖桿、MPU6050
- WiFi 模式：連接到現有 AP（目前為 FamilyCheng_ChungCheng）
