#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include "csoloader.h"

#ifdef __ANDROID__
#define SYSTEM_MATH_LIBRARY "libm.so"
#else
#define SYSTEM_MATH_LIBRARY "libm.so.6"
#endif

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s /path/to/libfixture.so file|anonymous\n",
            argv[0]);
    return 64;
  }

  void *libm = dlopen(SYSTEM_MATH_LIBRARY, RTLD_NOW | RTLD_GLOBAL);
  if (!libm) {
    fprintf(stderr, "failed to preload libm: %s\n", dlerror());
    return 65;
  }

  struct csoloader loader = { 0 };
  bool loaded = strcmp(argv[2], "anonymous") == 0
                  ? csoloader_load_anonymous(&loader, argv[1])
                  : csoloader_load(&loader, argv[1]);
  if (!loaded) {
    fprintf(stderr, "failed to load fixture in %s mode\n", argv[2]);
    dlclose(libm);
    return 66;
  }

  int (*run)(const char *) =
    (int (*)(const char *))csoloader_get_symbol(&loader, "libdl_compat_run");
  int (*open_lifetime_handle)(const char *) =
    (int (*)(const char *))csoloader_get_symbol(
      &loader, "libdl_compat_open_lifetime_handle");
  int (*close_lifetime_handle)(void) =
    (int (*)(void))csoloader_get_symbol(
      &loader, "libdl_compat_close_lifetime_handle");
  if (!run || !open_lifetime_handle || !close_lifetime_handle) {
    fprintf(stderr, "fixture entry points not found\n");
    csoloader_unload(&loader);
    dlclose(libm);
    return 67;
  }

  int result = run(argv[1]);
  if (result == 0 && open_lifetime_handle(argv[1]) != 0)
    result = 30;
  if (result == 0 && csoloader_unload(&loader))
    result = 31;
  if (result == 0 && close_lifetime_handle() != 0)
    result = 32;
  if (result == 0 && !csoloader_unload(&loader))
    result = 33;

  dlclose(libm);
  if (result != 0)
    fprintf(stderr, "libdl compatibility check failed: %d\n", result);
  return result;
}
