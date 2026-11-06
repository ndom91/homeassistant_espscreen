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
- The 3.4-inch diagonal and 640 x 180 pixels give a nominal 195.5 dpi. Tessera's compact-grid proposal suggests
  three columns by one row, with cells roughly 26 x 10 mm. This is a calculation, not a reading test on glass.
- The vendor example uses the `axs15231` touch platform for the CST3530 touch controller. A community example uses
  different transforms and native-axis calibration, so the correct settings for a particular board need a touch test.

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

## Pointers
- [LilyGO ESPHome example](https://github.com/Xinyuan-LilyGO/T-Display-S3-Long/tree/T-Display-S3-Long-cst3530/examples/ESPHome_LVGL_HelloWorld)
- [Community touch calibration discussion](https://community.home-assistant.io/t/lilygo-t-display-s3-long-esphome-screen-support/1025774)
- `docs/ADDING_A_BOARD.md`, `docs/RESPONSIVE.md`, `packages/looks/compact.yaml` and `packages/core.yaml`
