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
- [Community touch calibration discussion](https://community.home-assistant.io/t/lilygo-t-display-s3-long-esphome-screen-support/1025774)
- `docs/ADDING_A_BOARD.md`, `docs/RESPONSIVE.md`, `packages/looks/compact.yaml` and `packages/core.yaml`
