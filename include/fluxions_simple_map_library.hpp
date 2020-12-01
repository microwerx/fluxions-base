#ifndef FLUXIONS_SIMPLE_MAP_LIBRARY_HPP
#define FLUXIONS_SIMPLE_MAP_LIBRARY_HPP

#include <fluxions_simple_loadable_resource.hpp>
#include <fluxions_image_loader.hpp>

namespace Fluxions {
	class SimpleMapLibrary : public SimpleLoadableResource {
	public:
		SimpleMapLibrary();
		~SimpleMapLibrary();

		bool load() override;
		void unload() override;
		void cacheToDisk() override;
		void fetchCache() override;
		size_t sizeInBytes() const override;

		bool addMap(const std::string& path);

	private:
		std::map<std::string, Image4f> c4fImages;
		std::map<std::string, Image3f> c3fImages;
		std::map<std::string, Image4ub> c4ubImages;
		std::map<std::string, Image3ub> c3ubImages;
	};
} // namespace Fluxions

#endif
