#ifndef FLUXIONS_IMAGE_LOADER_HPP
#define FLUXIONS_IMAGE_LOADER_HPP

#include <fluxions_gte_image.hpp>

namespace Fluxions {
	bool LoadImage3ub(const std::string& path, Image3ub& image);
	bool LoadImage4ub(const std::string& path, Image4ub& image);
	bool LoadImage3f(const std::string& path, Image3f& image);
	bool LoadImage4f(const std::string& path, Image4f& image);

	// JPG is 3ub, PPM is 3ub
	bool IsImage3ub(const std::string& path);
	// PNG is 4ub
	bool IsImage4ub(const std::string& path);
	// PFM is 3f
	bool IsImage3f(const std::string& path);
	// EXR is 4f
	bool IsImage4f(const std::string& path);
}

#endif
