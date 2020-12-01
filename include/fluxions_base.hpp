#ifndef FLUXIONS_BASE_HPP
#define FLUXIONS_BASE_HPP

#include <hatchetfish.hpp>
#include <fluxions_stdcxx.hpp>
#include <fluxions_gte.hpp>
#include <fluxions_opengl.hpp>
#include <fluxions_gl1gl2_tools.hpp>
#include <fluxions_ibase_object.hpp>
#include <fluxions_colors.hpp>

namespace Fluxions {
	extern bool debugging;
	extern const std::string BlankString;
	
	std::map<std::string, std::string> MakeOptionsFromArgs(int argc, const char** argv);

	void ReadGLInfo();
	const std::string& GetRenderer();
	const std::string& GetVendor();
	const std::string& GetVersion();

	void Init();
	void Kill();
	void YieldThread();
} // namespace Fluxions

#endif
