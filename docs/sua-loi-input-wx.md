# Sửa lỗi input wxWidgets: mất phím, đếm trùng phím, kẹt phím

> Người sửa: Cường · Nhánh `feat/ecs-integration` · Commit `453d5f4` · 18/11/2026
> Phạm vi: `Core/Input/Wx/`, `Core/Window/Wx/WxWindow.cpp` (phần input của Huy/An), `Domain/Camera/OrbitCamera/OrbitCameraController.cpp`, `main.cpp`

## Tóm tắt

Khi kiểm thử viewer bằng phím thật (giả lập bằng `xdotool` dưới `xvfb`), phát hiện **ba lỗi có sẵn** trong phần input wx. Cả ba ảnh hưởng mọi phím tắt (1/2/3, R, F5/F9, Esc) và điều khiển camera bằng bàn phím:

| # | Lỗi | Triệu chứng người dùng thấy |
|---|---|---|
| 1 | Bấm nhanh bị mất | Bấm phím/click quá nhanh (nhấn và nhả trong cùng một frame, hoặc khi FPS thấp) thì không có tác dụng |
| 2 | wxGTK gửi lặp key-down | Một lần bấm bị tính **hai lần** (vd bật rồi tắt ngay wireframe, F5 lưu hai lần) |
| 3 | Kẹt phím (hệ quả của #2) | **Chạm phím mũi tên / W A S D một lần mà camera quay hoặc trượt mãi** cho tới lần bấm sau |

Lỗi #1 và #2 che nhau một cách tình cờ ở code cũ (cú bấm thật bị mất, nhưng bản lặp lại được tính), nên trước đây phím "có vẻ" chạy đúng — trừ việc thỉnh thoảng bị kẹt (#3).

---

## Lỗi 1 — bấm nhanh bị mất

**Nguyên nhân.** `WxKeyboardInput::CheckIsJustPressed` (và `WxMouseInput` tương tự) tính:

```cpp
return isHeld && !wasPressed;   // đang giữ lúc frame đọc && frame trước chưa giữ
```

Nếu `KEY_DOWN` và `KEY_UP` cùng đến trước khi frame đọc input, lúc đọc phím đã "không giữ" → `false` → cú bấm biến mất.

**Bằng chứng.** Chương trình nhỏ gọi `NotifyKeyDown('2'); NotifyKeyUp('2');` rồi đọc: `JustPressed = 0` (bàn phím và chuột đều vậy).

**Cách sửa.** Thêm cờ `mPressedThisFrame`:
- đặt `true` trong `NotifyKeyDown`/`NotifyButtonDown` **khi chuyển từ thả sang nhấn** (auto-repeat lúc đang giữ không tính);
- `CheckIsJustPressed` trả về cờ này;
- `Update()` (gọi ở `EndFrame`) xoá cờ;
- mất focus: `ResetInput()` thay cả object input nên cờ cũng bị xoá (giữ invariant "mất focus giải phóng phím").

**Ảnh hưởng phụ đã xử lý.** Giờ một click đã nhả vẫn là "vừa nhấn" tới hết frame. `OrbitCameraController::TryCreateOverridingState` dùng "vừa nhấn" để cho gesture mới giành quyền gesture đang chạy → một click phải nhanh có thể cắt ngang thao tác kéo chuột trái. Sửa: trong `IsGestureActive(..., justPressed = true)`, gesture phải **vừa nhấn và vẫn đang giữ** mới được giành quyền (hành vi chọn gesture giữ nguyên như cũ).

## Lỗi 2 — wxGTK gửi lặp key-down

**Nguyên nhân.** Trên Linux (wxGTK), `WxEngineCanvas::OnKeyDown` được gọi **hai lần cho cùng một lần nhấn**, cùng mã phím và **cùng timestamp**. Bản lặp có lúc đến ngay sau, có lúc đến **sau key-up**, có lúc **xen giữa phím khác**:

```
KEY_DOWN 50 ts=70675058
KEY_UP   50 ts=70675064
KEY_DOWN 51 ts=70675072
KEY_DOWN 50 ts=70675058   <- bản lặp của phím 2, sau key-up và sau phím 3
KEY_DOWN 51 ts=70675072   <- bản lặp của phím 3
```

`xev` xác nhận X server chỉ gửi **1** KeyPress + **1** KeyRelease cho mỗi lần bấm → bản lặp sinh ra trong wxGTK/GTK, không phải do công cụ test. Xảy ra cả khi giả lập bằng XTEST (giống bàn phím vật lý).

**Cách sửa.** Trong `WxEngineCanvas::OnKeyDown`, nhớ timestamp key-down cuối cùng **theo từng phím** (`mLastKeyDownTimestamps`); key-down trùng mã phím + timestamp thì bỏ qua (vẫn `evt.Skip()`). Timestamp `0` (sự kiện tự tạo, hoặc nền tảng không cung cấp) không lọc. Auto-repeat khi giữ phím có timestamp mới nên vẫn được nhận.

## Lỗi 3 — kẹt phím

Hệ quả của lỗi 2: chuỗi `KEY_DOWN, KEY_UP, KEY_DOWN(lặp)` làm phím ở trạng thái "đang giữ" mà không bao giờ có key-up tương ứng. Với phím mũi tên / W A S D / PageUp-PageDown, camera tiếp tục quay/trượt/dolly mãi.

**Bằng chứng trên code gốc:** chạm phím → một lần, chụp màn hình sau 0.8s và 1.5s: 1/3 lần chạy hai ảnh khác nhau 55 028 pixel (camera vẫn đang quay). Sau khi sửa: 5/5 lần giống hệt (0 pixel).

## `main.cpp`

Esc trước đây kiểm tra bằng `CheckIsPressed` (đang giữ) → Esc bấm nhanh bị mất (trước đây chạy được nhờ bản lặp gây kẹt phím). Nay đóng khi `CheckIsJustPressed || CheckIsPressed`.

---

## Kiểm chứng

**Test tự động mới**
- `tests/test_engine_logic.cpp` — `TestWxInputTapWithinOneFrame`: nhấn + nhả trong một frame (bàn phím, chuột), auto-repeat không tính lần mới, nhả rồi nhấn lại trong cùng frame, R bấm nhanh vẫn reset camera, click phải nhanh khi đang kéo chuột trái không cắt ngang orbit.
- `tests/test_wx_integration.cpp` — qua cửa sổ wx thật: key-down lặp (cùng timestamp) đến sau key-up bị bỏ, phím không kẹt; key-down mới (timestamp khác) vẫn nhận.
- Đã thử bỏ từng bản sửa: test tương ứng **fail** đúng chỗ (test thực sự bắt lỗi). Test cũ của nhóm (`TestCameraBoundariesAndKeyboard`, gesture arbitration, wx integration) **không phải sửa**.

**Chạy thật (xvfb + xdotool, so ảnh chụp màn hình)**

| Kịch bản | Trước | Sau |
|---|---|---|
| 10 phím (1,1,2,2,3,3,F5,1,3,F9), XTEST, 5 lần | 8–11 toggle, F5/F9 có lần ×2, Esc có lần không nhận | 8/8 toggle, F5 ×1, F9 ×1, Esc thoát — **5/5 lần** |
| Cùng kịch bản, XSendEvent, 5 lần | mất hoặc thừa phím | **5/5 lần đúng** |
| Bấm kiểu người (giữ 80 ms) | đúng | đúng |
| Chạm → một lần | camera đôi khi quay mãi | **dừng ngay** (5/5) |
| Giữ → 1.2 s rồi nhả | quay liên tục | quay liên tục, **dừng khi nhả** (3/3) |

**Bộ test:** `ctest` 12/12 (Debug ×5, Release ×3), ASan + UBSan không lỗi.

## Chưa kiểm chứng / lưu ý cho nhóm

- **Windows (wxMSW):** chưa chạy. Lỗi 2 quan sát trên wxGTK; bộ lọc theo timestamp vô hại nếu wxMSW không gửi lặp. Cần build + chạy `ctest --preset vcpkg-debug` và thử bấm phím trên Windows.
- **Chuột:** không thấy sự kiện chuột bị lặp; chỉ sửa lỗi 1 cho chuột.
- **Ngữ nghĩa mới của `CheckIsJustPressed`:** "đã có một lần chuyển thả → nhấn từ lần `Update()` trước", **không còn hàm ý đang giữ**. Code nào cần "vừa nhấn và còn giữ" thì kiểm tra thêm `CheckIsPressed` (như `OrbitCameraController`).
- Backend GLFW (`Core/Input/GLFW`) không nằm trong build CMake nên không sửa.
