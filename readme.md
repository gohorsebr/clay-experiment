# clay-experiment

## setup for windows

```bash
# download dependencies & setup it (sdl, sdl_ttf, sdl_image...)

./download.sh

# build nt
make nt

# run nt build
make run-nt

```

## setup for linux

```bash
# need to install SDL2, SDL2_ttf, SDL2_image develop packages
apt install -y libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev

# build
make

# run
make run

```