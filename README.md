# CSOLoader

CSOLoader is a traceless and system linker independent SOTA custom linker for Android Linux.

## Features

- C99 compliant
- System linker independent (no dlopen)
- Dependency-less (no third-party dependency, except system ones)
- Near complete feature parity to AOSP linker

## Installation

### 1. Clone the repository

```shell
$ git clone https://github.com/ThePedroo/CSOLoader
$ cd CSOLoader
```

### 2. Connect your Android device

This is required to test CSOLoader capabilities. Before proceeding, connect your device and assure you can access `adb shell`.

### 3. Compile the project

```shell
$ make
```

### 4. Compile the shared library

```shell
$ cd shareds
$ make standalone
```

### 3. Run CSOLoader

```shell
$ adb shell su -c /data/local/tmp/csoloader /data/local/tmp/shared.so
```

## Usage

CSOLoader aims to provide a simple API to match the as-simple API that system linkers/libdl provides. Currently, there are 6 user-facing APIs.

In [csoloader.c](./src/csoloader.c), in `STANDALONE_TEST` `#ifdef`, there is a basic usage of CSOLoader. Usage of CSOLoader's `csoloader_abandon` follows the same as `csoloader_unload`.

### Dependency library search paths

By default, CSOLoader searches dependencies (DT_NEEDED) in a built-in list of directories (e.g. `/system/lib64`, `/vendor/lib64`, `/apex/com.android.runtime/lib64`, ...).

If you need to load a library whose dependencies live outside those defaults (for example, loading `/apex/com.android.art/lib64/libart.so` and resolving its dependencies from the same APEX), you can add extra search directories:

- **Environment variable**: set `CSOLOADER_LIBRARY_PATH` to a colon-separated list of directories (similar to `LD_LIBRARY_PATH`).

```shell
$ export CSOLOADER_LIBRARY_PATH=/apex/com.android.art/lib64:/apex/com.android.art/lib
```

- **Programmatic API**: call `linker_add_library_search_path()` (or `linker_add_library_search_paths_from_string()`) before `csoloader_load()`.

### Mapping modes

`csoloader_load()` keeps the original file-backed mapping behavior. Use
`csoloader_load_anonymous()` when PT_LOAD segments must be copied into anonymous
memory so the source library path is not exposed in `/proc/<pid>/maps`.
Dependencies that CSOLoader must load manually inherit the selected mapping
mode; dependencies already present in the process are reused unchanged.

### libdl compatibility

Define `CSOLOADER_MAKE_LINKER_HOOKS` to provide custom-loaded libraries with
CSOLoader-aware `dl_iterate_phdr`, `dladdr`, `dlopen`, `dlsym`, `dlclose`, and
`dlerror`. `dlsym` supports custom handles, `RTLD_DEFAULT`, and `RTLD_NEXT`
within the requesting CSOLoader scope, while calls for system handles are
forwarded to the platform libdl.

`CSOLOADER_HOOK_LIBDL` enables the same libdl bridge without enabling unrelated
future linker hooks. `CSOLOADER_HOOK_DLADDR` enables only the introspection
hooks. CSOLoader must remain mapped while code relocated to these bridge
functions can execute.

Custom `dlopen` handles pin their CSOLoader owner. `csoloader_unload()` returns
false while such handles or compatibility calls are active; close the handles
and retry the same `csoloader` object. A successful unload clears the object.

## Documentation

1. `bool csoloader_load(struct csoloader *lib, const char *lib_path)`

Loads and links the library with the original file-backed mapping mode, and loads the info to the `csoloader` structure for future management. Returns false if operations fail, and performs cleanup automatically.

2. `bool csoloader_load_ex(struct csoloader *lib, const char *lib_path, const struct csoloader_options *options)`

Loads and links the library using the selected mapping mode. A NULL options pointer selects file-backed mapping.

3. `bool csoloader_load_anonymous(struct csoloader *lib, const char *lib_path)`

Loads and links the library using anonymous PT_LOAD mappings.

4. `bool csoloader_unload(struct csoloader *lib)`

Unloads the library from the process' memory from the `csoloader` structure.
Returns false without clearing the object if custom libdl handles or calls still
pin it, so the caller can release them and retry.

5. `bool csoloader_abandon(struct csoloader *lib)`

Frees all internal management handles while allowing it to still continue operation, however, without TLS available. Returns false if operations fails, and will leak memory if that happens.

6. `void *csoloader_get_symbol(struct csoloader *lib, const char *symbol_name)`

Gets the symbol address from the loaded library, won't work if the library has been abandoned. Returns NULL if operation failed.

## Features

- [x] Indirect symbol support 
- [x] Load and link library's dependencies
- [x] Full relocation support
- [x] Android-specific relocations support
- [x] Memory protection
- [x] Constructors/Deconstructors calling
- [x] `libdl` functions support (`dl_iterate_phdr`, `dladdr`, `dlopen`,
      `dlsym`, `dlclose`, `dlerror`)
- [x] TLS support
- [x] (EH) Frame register
- [x] C++ exceptions support (testing)
- [ ] MTE (tagged-pointer) support
- [ ] Support be system linker
- [ ] `preinit` support (needed to be system linker)

The [testing file for Android](./shareds/cpp-shared.cpp) implements testing for almost all supported functionality.

## Support

Any question or issue related to CSOLoader or other PerformanC projects can be made in [PerformanC's Discord server](https://discord.gg/uPveNfTuCJ).

## Contribution

It is mandatory to follow the PerformanC's [contribution guidelines](https://github.com/PerformanC/contributing) to contribute to CSOLoader. Following its Security Policy, Code of Conduct and syntax standard.

## Projects using CSOLoader

- [ReZygisk](https://github.com/PerformanC/ReZygisk)

## License

CSOLoader is licensed under [GNU AGPLv3 License](LICENSE). You can read more about it on [Open Source Initiative](https://opensource.org/license/agpl-v3).

* This project is considered as: [leading standard](https://github.com/PerformanC/contributing?tab=readme-ov-file#project-information).
