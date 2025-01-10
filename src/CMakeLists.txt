target_sources(
	Jubes PRIVATE

    "buffers.cpp"
	"common.cpp"
	"devices.cpp"
    "graphics.cpp"
	"main.cpp"
	"present.cpp"
	"sync.cpp"

    "buffers.hpp"
	"common.hpp"
	"devices.hpp"
    "graphics.hpp"
	"precompiled.hpp"
	"present.hpp"
	"sync.hpp"
)

target_precompile_headers(Jubes PRIVATE precompiled.hpp)
