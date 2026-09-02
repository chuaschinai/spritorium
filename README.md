# Spritorium

```bash
git clone --recurse-submodules https://github.com/chuaschinai/spritorium
cd spritorium
git submodule update --init --recursive

xmake f -m debug
xmake project -k compile_commands # optional
xmake run
```
