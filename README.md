# GoalShotLauncher

BakkesMod Freeplay training plugin.

- Press **D** to launch the ball toward the opponent goal.
- Default shot speed: **1000 KPH**.
- Change speed in the BakkesMod console with `gs_speed <KPH>`.
- Manual shot command: `gs_shoot`.
- The notifier is restricted to **Freeplay** and the code also checks `IsInFreeplay()` before changing ball physics.

The DLL is built by GitHub Actions against the official BakkesModSDK.
