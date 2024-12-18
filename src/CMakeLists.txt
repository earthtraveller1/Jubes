target_sources(
	Jubes PRIVATE

	main.cpp
	devices.cpp

	common.hpp
	devices.hpp
	precompiled.hpp
)

target_precompile_headers(Jubes PRIVATE precompiled.hpp)