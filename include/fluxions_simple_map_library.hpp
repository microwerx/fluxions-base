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

		bool addMap(const std::string& path, const FilePathFinder& fpf);

		auto c3ubImages() const { return c3ubImages_; }
		auto c4ubImages() const { return c4ubImages_; }
		auto c3fImages() const { return c3fImages_; }
		auto c4fImages() const { return c4fImages_; }

	private:
		std::map<std::string, Image3ub> c3ubImages_;
		std::map<std::string, Image4ub> c4ubImages_;
		std::map<std::string, Image3f> c3fImages_;
		std::map<std::string, Image4f> c4fImages_;
	};
} // namespace Fluxions

#endif
