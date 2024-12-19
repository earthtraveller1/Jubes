target_sources(
	Jubes PRIVATE

	"common.cpp"
	"devices.cpp"
	"main.cpp"
	"present.cpp"

	"common.hpp"
	"devices.hpp"
	"precompiled.hpp"
	"present.hpp"
)

target_precompile_headers(Jubes PRIVATE precompiled.hpp)