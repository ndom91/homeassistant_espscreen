# LilyGO T-Display S3 Long: board port is not yet verified

## The symptom
Tessera does not yet support the 640 x 180 LilyGO T-Display S3 Long. Its short display needs a one-row layout, and
its AXS15231B panel needs display settings that differ from existing Tessera boards.

## What we know
- LilyGO's ESPHome example drives the panel at its native 180 x 640, turns LVGL 90 degrees, and uses full refresh,
  a 100% buffer and a draw rounding of 4. It reports testing with ESPHome 2026.8.2. The example documents partial
  redraw corruption and a crash with a fractional buffer.
- On 2026-11-06 ESPHome 2026.8.0 resolved a test configuration with rounding 4 on LVGL alone to rounding 8, because
  LVGL takes the maximum of its own rounding and the display's. Setting 4 on both resolved both to 4.
- On 2026-11-06 a compile-only lab profile built with ESPHome 2026.8.0 and ESP-IDF 5.5.5. The image was 2,342,479
  bytes; the link reported 159,691 bytes of 341,760 bytes of internal DIRAM used. These are build figures, not free
  heap measurements on a running screen. The build used dummy Wi-Fi and no API encryption and must not be flashed.
- On 2026-11-06 the compile-only profile built with ESPHome 2026.9.0 and ESP-IDF 5.5.5. Capturing the container's
  stdout and stderr separately confirmed exit status 0 and ESPHome's success message; the image was 2,333,971 bytes,
  with 159,635 bytes of 341,760 bytes of DIRAM used at link time. Generated LVGL code specifies a full buffer, full
  refresh and rounding 4. This image also uses dummy Wi-Fi and no API encryption and must not be flashed.
- The 3.4-inch diagonal and 640 x 180 pixels give a nominal 195.5 dpi. Tessera's compact-grid proposal suggests
  three columns by one row, with cells roughly 26 x 10 mm. This is a calculation, not a reading test on glass.
- On 2026-11-17 the local lab profile was changed to four columns by one row, and one column by four rows standing up.
  ESPHome 2026.9.0 built a private test configuration with encrypted API. On the same day the factory image was flashed
  to a board using the ROM bootloader and its write hash verified. The serial port later reported a memory health line,
  but no display, touch, Wi-Fi or Home Assistant behavior was confirmed in that observation. The three-column figures
  and tests below remain historical.
- Before that flash, two independent full 16 MB ROM-bootloader reads matched. Two reads through the ESPHome flasher stub
  had stopped early with serial data-stream errors, so the stub path is still unproven. The private backup was not restored
  to the board and has not been tested as a recovery image.
- On 2026-11-17 the screen joined Wi-Fi, was added to Home Assistant and appeared online in Tessera with a four-column,
  one-row grid. A tile appeared on the physical panel, but presses did not operate it or wake the screen. Turning off
  page buttons made the tiles taller according to the person testing the glass. Touch remains unverified and broken in
  this test; the page-button band is a significant use of the panel's height.
- A 60-second serial observation during reported touches on 2026-11-17 contained no matching touch presses or touch/I2C
  warnings. This does not prove the controller is absent: the logging and the tap timing need checking. The lab profile
  uses ESPHome's `axs15231` touch driver as in LilyGO's ESPHome example, while LilyGO's separate CST3530 touch example
  uses its Hynitron driver. The controller identity, I2C address, protocol and any power or reset requirements need
  checking before choosing a replacement driver.
- On 2026-11-17 review of [PR #115](https://github.com/MaxGramser/homeassistant_espscreen/pull/115) found that its
  previously used LilyGO profile also selects `axs15231`, but sets the core's touch substitutions to swap axes and
  mirror native Y, with calibration x 6..176 and y 10..627. Its author reported using that screen, but these values have
  not been tested on this board. The current lab profile instead put transforms in the touchscreen block; a resolved
  ESPHome 2026.9.0 config shows the core's extension replaced them with swap and both mirrors false, while calibration
  remains x 0..640 and y 0..180. Test the PR's transform as a candidate before changing the driver; the absence of
  logged presses still needs a separate explanation if that test fails.
- On 2026-11-17 a private build with that candidate transform and the selected Status and Timers plugin packages
  compiled and was uploaded over OTA. The encrypted API reconnected afterwards. The transform, plugin execution and
  retention of settings and layout still need physical validation; compilation and reconnection do not establish them.
- After the candidate transform update the person testing reported that touch still did not work. A 90-second serial
  capture around a requested reset and taps recorded no matching touch or I2C lines; it did not show the USB disconnect
  or a boot scan, so it cannot establish whether the reset or the boot diagnostics were captured. A further diagnostic
  build or another capture is needed before attributing the failure to the controller, bus or driver.
- On 2026-11-17 a private diagnostic build with debug logging, repeated boot-scan output and a touch callback compiled
  and was uploaded over OTA; its API reconnected. A 26-second serial observation had no lines and a 26-second encrypted
  API log observation did not show the diagnostic or I2C lines. Two API log messages reported the previous boot's
  lowest free internal memory, 192,544 bytes at 305 seconds. These observations do not establish an I2C scan result or
  a cause of the touch failure; check the running firmware's health and logging before interpreting missing messages.
- The person testing reported that the screen stayed on and online. Source inspection found a reason for the missing
  INFO diagnostics: `runtime_tiles::apply_debug_mode()` sets the runtime logger to WARN unless the screen's Debug mode
  is on, even when the firmware was compiled at DEBUG. Enable that setting for the diagnostic capture, then turn it off.
- LilyGO's separate CST3530 example configures SDA 15, SCL 10, interrupt 11 and reset 2, and its Hynitron driver
  identifies normal I2C address `0x58`, boot address `0x5A` and a 32-bit `0xD0070000` touch report register.
  ESPHome's `axs15231` driver instead defaults to `0x3B` and uses a different request. LilyGO's wiki and master
  example call GPIO16 the touch reset, while the CST3530 branch calls GPIO2 touch reset and GPIO16 display reset.
  The delivered controller and wiring have not yet been identified: the branch and wiki disagree, and the existing
  scan has not been observed. Do not pulse either pin or replace the driver on that evidence alone.
- An independently published `cst3530` ESPHome component targets `0x58` and the same report register on a different
  Waveshare board. It is a candidate, not a tested drop-in: its bus transaction semantics, licensing and compatibility
  with this project's pinned ESPHome version need review before use.
- On 2026-11-17, after the person testing enabled Debug mode, the diagnostic firmware's boot-time scan reported
  `0x58` and `0x6A` on SDA 15 / SCL 10 at 50 kHz, with no `0x3B` in the reported results. This matches the CST3530
  example's normal touch address and the documented charger address, but an address match alone does not prove chip
  identity or working touch. The running `axs15231` driver requests `0x3B`, so test a CST3530-protocol driver against
  `0x58` before trying further coordinate changes.
- On 2026-11-17 a local test adapted the independent CST3530 component at commit `34f17b88f866433e536b4f7133227743e870b980`
  to use ESPHome's combined I2C write/read transaction, built on ESPHome 2026.9.0 and uploaded over OTA. It uses `0x58`,
  polling and no reset or interrupt pin, retaining the existing candidate calibration. A 90-second API log capture
  produced no matching lines; whether Debug mode was enabled and whether touches occurred during it were not established.
  No touch result or driver correctness is claimed from the build or upload.
- The vendor example uses the `axs15231` touch platform for the CST3530 touch controller. A community example uses
  different transforms and native-axis calibration, so the correct settings for a particular board need a touch test.
- In ESPHome 2026.8.0, touch coordinates are transformed and normalized to the display's native dimensions before
  LVGL rotates them. For a 90-degree LVGL rotation, LVGL maps native `(x, y)` to `(y, width - x - 1)`. Raw controller
  readings and calibration still determine where a physical touch lands, so this does not select a transform by itself.
- On 2026-11-06 the 17 existing `tests/test_grid.py` tests passed. They exercise general grid handling, not this
  board's rendered tiles, page navigation or dialogs. The layout audit does not include an uncatalogued lab board.
- On 2026-11-06 the existing WASM layout audit refused this lab shape at initialization: its generated preview has no
  compact look at this density. No card layouts were checked. The resolved 2026.9.0 profile has a 48 px top offset,
  49 px page bar, 9 px side margins and 8 px column gaps. On a 640 x 180 canvas, the grid code therefore gives a
  three-column, one-row page approximately 202 x 83 px per cell with the page bar, about 26.2 x 10.8 mm at 195.5 dpi.
  With one page, the hidden bar lets the grid grow to 123 px high. These are calculations, not measurements of drawn
  cards, navigation, settings or dialogs; a board-specific preview and physical checks are still needed.
- On 2026-11-06 a direct check of the add-on's default `Grid(3, 1)` confirmed three slots per page, eight pages and
  24 single-tile slots in total. A wide tile fits starting in either of the first two cells, a full tile starts at the
  first, and tall and square tiles do not fit. A screen's negotiated page and tile limits can differ; this check does
  not exercise firmware placement or actual controls.
- The WASM preview's styles and fonts are generated per catalog density by `web/wasm/preview_profiles.py` and
  `generate_host_ui.py`. A board-specific preview needs a temporary compact profile near 196 dpi and a WASM rebuild
  with Emscripten; that compiler was not available on the investigation machine. Check the preview's integer density
  against the board's 195.538 dpi substitutions before relying on its measurements. Do not use an unrelated density
  as proof of layout fit.
- LilyGO's repository contains `firmware/factory-cst3530.bin` and a factory example. The binary's exact board revision,
  contents and flash offsets have not been verified; it is a recovery candidate, not proof of a restorable factory
  installation. The vendor describes BOOT and RST button entry into download mode in its README.
- The published standard and SD-slot schematics show GPIO4, 5, 6 and 7 on the expansion header as possible external
  audio candidates. Confirm the delivered board revision, header and pin conflicts before choosing pins or connecting
  an amplifier. A separate optional plugin, not the base board, should own added audio hardware.

## Ruled out
Nothing on a physical board yet. A valid ESPHome configuration alone does not establish that touch, rotation or
redraws work.

## Candidates
- A board profile using LilyGO's QSPI, I2C and backlight pins, with Tessera's compact look and capacitive touch.
- The one-row layout or its dialogs may need changes to fit the glass, and full-screen refresh may limit responsiveness.

## The deciding experiment
With a board positively identified and its existing firmware recoverable, flash a test profile and check all four
touch corners, a slider drag, page swipes, colour, backlight, standby, reconnect and redraws. Measure free internal RAM
and interaction responsiveness with a representative layout. Check the settings and detail cards for clipped controls.
Do not promote the board to the catalog on the strength of a configuration check alone.

Before any flash, identify the USB port by connecting the board and comparing the port list. Check the chip's security
and flash settings, and whether its existing flash can be read and restored; protect any backup as potentially sensitive.
Verify the recovery image and offsets for that revision independently. Prepare a separate local profile with real
network settings and API encryption through secrets, and confirm the target and obtain permission before writing.
For touch, record raw and resulting coordinates at the center and near all four corners, then test a continuous drag;
change one transform at a time and repeat, without performing Home Assistant actions during the test.

## Pointers
- [LilyGO ESPHome example](https://github.com/Xinyuan-LilyGO/T-Display-S3-Long/tree/T-Display-S3-Long-cst3530/examples/ESPHome_LVGL_HelloWorld)
- [LilyGO board repository and recovery instructions](https://github.com/Xinyuan-LilyGO/T-Display-S3-Long/tree/T-Display-S3-Long-cst3530)
- [LilyGO wiki](https://wiki.lilygo.cc/products/t-display-series/t-display-s3-long/) and
  [CST3530 driver](https://github.com/Xinyuan-LilyGO/T-Display-S3-Long/blob/3deece89098f25e7d155e41e4e0f5dd8b368ab02/lib/hyn_driver_for_esp32/hyn_chips/hyn_cst66xx.c)
- [Independent CST3530 component](https://github.com/itconor/esphome-cst3530)
- [Community touch calibration discussion](https://community.home-assistant.io/t/lilygo-t-display-s3-long-esphome-screen-support/1025774)
- `docs/ADDING_A_BOARD.md`, `docs/RESPONSIVE.md`, `packages/looks/compact.yaml` and `packages/core.yaml`
