# Updating the LilyGO lab setup

The experimental LilyGO T-Display S3 Long uses two separate builds: an ESPHome image for the screen and a local Tessera Screen Manager app for Home Assistant. Neither is a production release. Keep the board out of the supported catalog while it is still being validated.

Use `.esphome/lilygo-deploy.local.md` for the names and targets on the lab machine. If it is missing, get the details from the operator rather than guessing. Home Assistant and the screen are read-only by default. Get explicit approval for each app update or firmware upload. An app rebuild can briefly interrupt the manager, and a firmware upload restarts the screen. Neither gives permission to change layouts, settings, or other devices.

## Build and update the screen

1. Use the private lab profile that matches the **installed** LilyGO: same device name, Wi-Fi, API encryption, OTA credentials, CST3530 configuration and plugin sidecar. The old compile-only profile with dummy Wi-Fi is not for flashing. Check that the profile includes this checkout's shared firmware and local board files, not an upstream package ref. Do not print or copy resolved configuration or unfiltered logs into chat.
2. Compile from the repo root with the pinned ESPHome image. Record the OTA image path, build time and SHA-256, and do not run another compile that uses the same build directory before uploading. Capture ESPHome output privately, especially if running `config`, which can print credentials.

   ```sh
   docker run --rm -v "$PWD:/config" -w /config "$ESPHOME_IMAGE" \
     compile "$LILYGO_PROFILE"
   shasum -a 256 "$OTA_IMAGE"
   ```

3. After upload approval, use the encrypted ESPHome API to read `device_info` at the chosen OTA address and compare its MAC to the recorded LilyGO identity. Stop if the connection, identity, or artifact hash cannot be verified. Upload the exact OTA image from step 2, with `$OTA_IMAGE` relative to the repo root. Do not use Tessera's **Update** button for this board. If OTA is unavailable, use the USB recovery workflow only with separate approval and after identifying the actual port by unplugging/reconnecting the LilyGO. Do not flash another connected ESP32 or use an old backup as the OTA image.

   ```sh
   docker run --rm -v "$PWD:/config" -w /config "$ESPHOME_IMAGE" \
     upload "$LILYGO_PROFILE" --device "$LILYGO_ADDRESS" --file "/config/$OTA_IMAGE"
   ```

4. After the reboot, read encrypted `device_info` again. Check the identity and compilation time, then confirm Tessera shows the screen online. Ask the operator to test the changed behavior on the glass. A successful upload and reconnect are not a physical acceptance test.

## Rebuild the experimental Home Assistant app

1. Run the relevant tests locally. If the editor changed, run `npm test` and `npm run build` in `web/`; the app serves the generated `screen_manager/app/static/` directory. For app-only changes, do not rebuild or flash the screen unless the change actually needs new firmware.
2. With app-update approval, query the installed apps and check that the experimental app has the expected slug, and the original manager is stopped before rebuilding or starting the experimental one. Both managers can discover the same screens and must not send competing layouts. They have separate `/data` directories but can see the same ESPHome profiles.
3. Copy only the app source being deployed into the experimental local app directory (under `/addons` on the lab SSH app). Keep its existing `config.yaml` with the distinct slug, panel title, manual startup, and separate published camera port. Do not copy private profiles, secrets, build caches or firmware images, and do not touch the existing app or either app's `/data`. For an editor update, copy the new hashed assets **before** the new `app/static/index.html`. If both the app and firmware change, rebuild and verify the app before uploading the firmware.
4. Ask Supervisor to rebuild the installed local app. A plain SSH `ha` may lack a Supervisor token. On the lab host, the existing CLI container can issue the commands through `sudo -n docker exec`. Do not extract or print the token.

   ```sh
   ssh "$HA_HOST" "sudo -n docker exec hassio_cli ha apps rebuild '$EXPERIMENTAL_APP' --no-progress"
   ssh "$HA_HOST" "sudo -n docker exec hassio_cli ha apps info '$EXPERIMENTAL_APP' --raw-json" \
     | jq -e 'select(.result == "ok" and .data.state == "started") | .data | {name, state, boot}'
   ssh "$HA_HOST" "sudo -n docker exec 'app_$EXPERIMENTAL_APP' curl -fsS --max-time 10 http://127.0.0.1:8099/health"
   ```

5. Verify that the experimental app is started, `/health` returns `ok`, and fresh logs say `Home Assistant connected`. For an editor update, check that the running container's `app/static/index.html` references the new bundle, then refresh the ingress page. Check that the original manager remains stopped. Do not restart it as an automatic fallback. If a handover back is needed, get approval and stop the experimental manager first.

## Sync with upstream

On the LilyGO fork, `origin` is upstream Tessera and `fork` is the writable fork. The branch `lilygo-s3-long` tracks `fork/lilygo-s3-long` for publishing, but merges from **`origin/dev`**. A plain `git pull` updates from the fork, not from upstream. Confirm the remotes with `git remote -v` if using a different clone.

Check for incoming changes without touching the working files:

```sh
git status --short --branch
git fetch origin dev
git rev-list --left-right --count origin/dev...lilygo-s3-long
git log --oneline lilygo-s3-long..origin/dev
```

The two counts are upstream-only and fork-only commits, respectively. To integrate them:

1. Review and commit the intended local work on `lilygo-s3-long` first, including the built editor if `web/src` changed. Do not stage private profiles, credentials, logs or firmware binaries. Keep the last known-good app and firmware available separately before deploying a merged build.
2. From the clean fork branch, use a temporary sync branch in a separate worktree so the working lab checkout stays intact. For example, choose unused names and run:

   ```sh
   git worktree add -b sync/lilygo-upstream ../tessera-lilygo-sync lilygo-s3-long
   git -C ../tessera-lilygo-sync merge origin/dev
   ```

   Review conflicts before completing the merge. Resolve source and changelog conflicts first. Regenerate derived tables with their generators and rebuild `screen_manager/app/static/` from the merged editor, rather than hand-merging generated bundles. The ESPHome lab profile and touch component are ignored and will not appear in a new worktree; provide the needed private files locally and check that the profile points to the **merged** sources before compiling.
3. Run the checks for the changed code and build the experimental app and LilyGO firmware as needed. Pay particular attention to the fork's tile orientation capability and picker, layout import, and the LilyGO's touch and rotation. An upstream app release can change the pinned ESPHome version, so check `screen_manager/Dockerfile` and verify the local CST3530 component before changing the lab compiler.
4. Once the merge is verified and the original worktree is clean, bring it onto `lilygo-s3-long` with `git merge --ff-only sync/lilygo-upstream`. If it cannot fast-forward, review the new fork commits instead of forcing it. Publish only to `fork` when approved. Syncing Git does not authorize installing the app or flashing the screen; follow the separate procedures above.

For the board's current validation status, see [the LilyGO lab issue](open-issues/lilygo-s3-long.md).
