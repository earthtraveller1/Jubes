target_sources(
	Jubes PRIVATE

	"common.cpp"
	"devices.cpp"
    "graphics.cpp"
	"main.cpp"
	"present.cpp"

	"common.hpp"
	"devices.hpp"
    "graphics.hpp"
	"precompiled.hpp"
	"present.hpp"
)

target_precompile_headers(Jubes PRIVATE precompiled.hpp)
