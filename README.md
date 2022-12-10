# PolyBlaster-Server

# PolyBlaster
PolyBlaster is multiplayer FPS game. \
You can download the game from [itch.io](https://jaamiekka.itch.io/polyblaster).

# How to run the server with Docker:
Docker is required and docker-compose is recommended. \
Clone this repository with dependencies
```
git clone --recursive https://github.com/viirret/PolyBlaster-Server.git
```

Run the server with **docker-compose**
```
docker-compose up -d
docker logs polyblaster
```
The default port is 11000. \
It can be changed from [docker-compose.yml](docker-compose.yml)

# How to run the server manually:
Again clone this repository with dependencies
```
git clone --recursive https://github.com/viirret/PolyBlaster-Server.git
```
Install build dependencies
- Ubuntu/Debian: `apt install build-essential cmake libboost-all-dev`
- Arch Linux: `pacman -S base-devel cmake boost`

compile using build script
```
./build.sh
```
and run the server with default port of 8080, or change it with command line argument
```
./start.sh
./start.sh 9132
```

