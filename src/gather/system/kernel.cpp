#include "gather/gather.hpp"
#include <sys/utsname.h>

namespace rfxh::gather {

void gather_kernel(GatherContext& ctx) {
    struct utsname u;
    uname(&u);
    add_info(ctx, "Kernel", "%s %s", u.sysname, u.release);
}

} // namespace rfxh::gather
