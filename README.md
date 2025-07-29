# XCompute

XCompute is an arbitrary OpenGL compute shader runner. It takes a compute shader source file as
input
and runs it within a window, outputing the resulting image to the screen. Shaders must output an
image
in order to be run by XCompute.

## Usage

```
$ xcompute <shader_file>
```

Where `<shader_file>` is a GLSL source code file path.

## Examples

XCompute has some example shaders located in the [Shaders](Shaders) directory.

## Building

All third-party dependencies are fetched by CMake. All you need to do is configure and build.

### 1. Clone Repository

```
$ git clone https://github.com/jakerieger/XCompute.git
```

### 2. Configure CMake Project

```
$ mkdir build && cd build
$ cmake ..
```

### 3. Build Project

```
$ cmake --build . --config Release
```

The resulting executable will be located in `XCompute/build/bin`.

If you don't feel like building from source, there are
precompiled [releases](https://github.com/jakerieger/XCompute/releases) available to
download.

## License

XCompute is licensed under the [ISC](LICENSE) license.