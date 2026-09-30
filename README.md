# Avatar Legends Name Mod

Avatar Legends: The Fighting Game always shows your Steam profile name, and there's no setting to change it. This mod lets you pick a different name for this game only. Your Steam account, profile and friends list stay the same.

Windows, Steam version only.

## Install

1. Download `AvatarNameMod-v1.0.zip` from [Releases](../../releases/latest).
2. Open the game folder. In Steam, right-click Avatar Legends, then Manage, then Browse local files.
3. Unzip it there. You should now have an `AvatarNameMod` folder next to `Atla.exe`.
4. Double-click `AvatarNameMod\install.bat`. Notepad opens `playername.txt`.
5. Type your name on the first line, save, and start the game.

To change the name later, edit `playername.txt` in the game folder and restart the game. If you leave the file empty, the game uses your Steam name again.

## After a game update

Updates can put Valve's original `steam_api64.dll` back, and then you're back to your Steam name. Run `install.bat` again. It notices the new file and keeps it as the backup.

## Uninstall

Run `AvatarNameMod\uninstall.bat`. Checking "Verify integrity of game files" in Steam also works.

## What it changes

The game asks Steam for your name, and the mod answers with the name in `playername.txt`. That covers everything your own game shows.

I haven't tested yet what opponents see online. If their game asks Steam for your name directly, they still see your real Steam name. If you try it, open an issue and say what showed up.

## Can I get banned for this?

The game has no anti-cheat, and the mod only swaps a display name. It doesn't touch gameplay, your save or the network code. Changing game files still probably breaks the terms of service, so I can't promise you're safe. Use it at your own risk.

Windows may warn you about an unsigned DLL or a .bat file from the internet. Every file in the zip is built from this repo, and the whole mod is about 150 lines in [`src/namemod.cpp`](src/namemod.cpp).

## How it works

The game loads `steam_api64.dll` from its own folder. `install.bat` renames Valve's DLL to `steam_api64_o.dll` and puts the mod in its place. The mod passes every call through to the original. The one exception is the call the game uses to get Steam's friends interface. There the mod replaces two entries in the interface's function table:

- `GetPersonaName()` returns the name from `playername.txt`.
- `GetFriendPersonaName()` does the same when the game asks about your own SteamID.

When the mod swaps the name, it writes a line with the old and the new name to `namemod.log` in the game folder. If your name doesn't change in game, check whether that line is there.

## Building it yourself

You need Visual Studio 2022 or the Build Tools with the "Desktop development with C++" workload.

```
build.bat
```

That creates `build\namemod.dll`, and `install.bat` finds it there too. `.\package.ps1 1.0` makes the release zip.

`src/exports.h` forwards all 1069 exports of the game's `steam_api64.dll`. I generated it from the export table with `dumpbin /exports`. If an update ships a newer DLL with new functions, that list has to be regenerated.

## License

MIT
