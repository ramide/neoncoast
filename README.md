# Neon Coast

A cozy, relaxed OutRun-style 3D racing game built in C with raylib.

## Features

- 8 international stages (California, Japan, India, Mediterranean, NYC, Hawaii, Sahara, Guatemala)
- 5 modern cars with distinct stats (Sport Coupe, Muscle, Electric GT, Supercar, Compact EV)
- 6-speed automatic gearbox with RPM gauge
- Drifting mechanics with handbrake
- Traffic cars and roadside scenery (trees, palms, buildings, signs)
- Hill/elevation changes in road
- Cloud rendering with pseudo-3D perspective
- Pause menu (resume, restart, settings, quit)
- OGG music by congusbongus (CC0)
- Dynamic day/night cycle
- Keyboard + gamepad controls with on-the-fly switching
- Attract mode with road background, full menu system, high scores
- Cross-platform: Windows, Linux (x86_64 + ARM64), macOS, Android

## Building

### Desktop

```bash
git clone --recursive https://github.com/yourname/neoncoast.git
cd neoncoast
mkdir build && cd build
cmake ..
cmake --build .
```

### Android

```bash
cd platforms/android
./gradlew assembleRelease
```

## Controls

| Action | Keyboard | Gamepad |
|--------|----------|---------|
| Steer | Arrow keys / WASD | Left stick |
| Accelerate | Up / W | Left stick up |
| Brake | Down / S | Left stick down |
| Handbrake | Space | X / Square |
| Confirm | Enter | A / Cross |
| Back | Backspace | B / Circle |
| Pause | Escape | Start |

## License

MIT

## Credits

- Music by [congusbongus](https://opengameart.org/users/congusbongus) (CC0):
  - *Midnight Drive*
  - *Ganymede*
