extern "C" {
void *dlopen(const char *filename, int flag) {return 0; }
char *dlerror(void) { return 0; }
void *dlsym(void *handle, const char *symbol) { return 0;}
int dlclose(void *handle) { return 0; }
struct Dl_info {};
int dladdr(void *addr, Dl_info *info) { return 0; }
void *dlvsym(void *handle, char *symbol, char *version) { return 0; }
}