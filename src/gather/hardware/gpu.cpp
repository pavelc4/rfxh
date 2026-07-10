#include "gather/gather.hpp"
#include <cstdio>
#include <cstring>
#include <dirent.h>

namespace rfxh::gather {

static bool gpu_lookup_lspci(const char* pci_id, char* out, int outlen) {
    if (!pci_id || !pci_id[0])
        return false;
    char cmd[128];
    std::snprintf(cmd, sizeof(cmd), "lspci -d %s 2>/dev/null", pci_id);
    FILE* fp = popen(cmd, "r");
    if (!fp)
        return false;
    char line[256];
    bool ok = false;
    if (std::fgets(line, sizeof(line), fp)) {
        int l = std::strlen(line);
        while (l > 0 && (line[l - 1] == '\n' || line[l - 1] == '\r'))
            line[--l] = '\0';
        char* rev = std::strstr(line, " (rev ");
        if (rev)
            *rev = '\0';
        char* lb = std::strrchr(line, '[');
        char* rb = std::strrchr(line, ']');
        const char* name = nullptr;
        if (lb && rb && rb > lb) {
            *rb = '\0';
            name = lb + 1;
        } else {
            char* corp = std::strstr(line, " Corporation ");
            int skip = corp ? 13 : 0;
            if (!corp) {
                corp = std::strstr(line, " Corp ");
                if (corp)
                    skip = 6;
            }
            if (!corp) {
                char* c1 = std::strchr(line, ':');
                if (c1) {
                    char* c2 = std::strchr(c1 + 1, ':');
                    if (c2) {
                        corp = c2;
                        skip = 1;
                    }
                }
            }
            name = corp ? (corp + skip) : line;
            while (*name == ' ')
                name++;
        }
        if (name && *name) {
            std::strncpy(out, name, outlen - 1);
            out[outlen - 1] = '\0';
            ok = true;
        }
    }
    pclose(fp);
    return ok;
}

void gather_gpu(GatherContext& ctx) {
    DIR* d = opendir("/sys/class/drm");
    if (!d)
        return;
    struct dirent* ent;
    while ((ent = readdir(d))) {
        if (std::strncmp(ent->d_name, "card", 4) != 0)
            continue;
        bool all_digits = true;
        for (int i = 4; ent->d_name[i]; i++) {
            if (ent->d_name[i] < '0' || ent->d_name[i] > '9') {
                all_digits = false;
                break;
            }
        }
        if (!all_digits)
            continue;

        char path[256];
        std::snprintf(path, sizeof(path), "/sys/class/drm/%s/device/uevent",
                      ent->d_name);
        FILE* fp = std::fopen(path, "r");
        if (!fp)
            continue;
        char driver[32] = "", pci_id[16] = "", compat[64] = "";
        char buf[256];
        while (std::fgets(buf, sizeof(buf), fp)) {
            if (std::strncmp(buf, "DRIVER=", 7) == 0) {
                char* v = buf + 7;
                int l = std::strlen(v);
                while (l > 0 && (v[l - 1] == '\n' || v[l - 1] == '\r'))
                    v[--l] = '\0';
                std::strncpy(driver, v, sizeof(driver) - 1);
            } else if (std::strncmp(buf, "PCI_ID=", 7) == 0) {
                char* v = buf + 7;
                int l = std::strlen(v);
                while (l > 0 && (v[l - 1] == '\n' || v[l - 1] == '\r'))
                    v[--l] = '\0';
                std::strncpy(pci_id, v, sizeof(pci_id) - 1);
            } else if (std::strncmp(buf, "OF_COMPATIBLE_0=", 16) == 0) {
                char* v = buf + 16;
                int l = std::strlen(v);
                while (l > 0 && (v[l - 1] == '\n' || v[l - 1] == '\r'))
                    v[--l] = '\0';
                std::strncpy(compat, v, sizeof(compat) - 1);
            }
        }
        std::fclose(fp);

        char name[160] = "";
        const char* type = "";

        if (std::strstr(compat, "display-subsystem"))
            continue;

        if (std::strncmp(compat, "apple,agx", 9) == 0) {
            char cpu[64] = "";
            FILE* mfp = std::fopen("/proc/device-tree/model", "r");
            if (mfp) {
                char model[128];
                if (std::fgets(model, sizeof(model), mfp)) {
                    char* paren = std::strchr(model, '(');
                    if (paren) {
                        paren++;
                        char* comma = std::strchr(paren, ',');
                        char* end = comma ? comma : std::strchr(paren, ')');
                        if (end)
                            std::snprintf(cpu, sizeof(cpu), "%.*s",
                                          static_cast<int>(end - paren), paren);
                    }
                }
                std::fclose(mfp);
            }
            if (cpu[0])
                std::snprintf(name, sizeof(name), "Apple %s", cpu);
            else
                std::strcpy(name, "Apple GPU");
            type = "Integrated";
        } else if (pci_id[0] && gpu_lookup_lspci(pci_id, name, sizeof(name))) {
        } else if (std::strcmp(driver, "i915") == 0 || std::strcmp(driver, "xe") == 0) {
            std::strcpy(name, "Intel Graphics");
        } else if (std::strcmp(driver, "amdgpu") == 0 || std::strcmp(driver, "radeon") == 0) {
            std::strcpy(name, "AMD Graphics");
        } else if (std::strcmp(driver, "nvidia") == 0 || std::strcmp(driver, "nouveau") == 0) {
            std::strcpy(name, "NVIDIA GPU");
        } else if (driver[0]) {
            std::strncpy(name, driver, sizeof(name) - 1);
        }

        if (!type[0]) {
            if (!std::strcmp(driver, "i915") || !std::strcmp(driver, "xe"))
                type = "Integrated";
            else if (!std::strcmp(driver, "nvidia") || !std::strcmp(driver, "nouveau"))
                type = "Discrete";
        }

        if (!name[0])
            continue;
        if (type[0])
            add_info(ctx, "GPU", "%s [%s]", name, type);
        else
            add_info(ctx, "GPU", "%s", name);
    }
    closedir(d);
}

} // namespace rfxh::gather
