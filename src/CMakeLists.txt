target_sources(
	Jubes PRIVATE

	common.cpp
	devices.cpp
	main.cpp

	common.hpp
	devices.hpp
	precompiled.hpp
)

target_precompile_headers(Jubes PRIVATE precompiled.hpp)