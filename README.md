# AOS-SQLITE

This project builds the AOS-SQLITE WASM Binary and Publishes it to Arweave.

## Build Process

1. Build docker image

```sh
cd container
./build.sh
```

2. Get Latest aos module

```sh
git submodule update --remote
```

3. Use docker image to compile process.wasm

```sh
cd aos/process
docker run -v .:/src p3rmaw3b/ao emcc-lua
```

4. Publish Module with tags via arkb

> You will need a funded wallet for this step 

```sh
export WALLET=~/.wallet.json
npm run deploy
```