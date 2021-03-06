#include "fluxions_base_pch.hpp"
#include <fluxions_base.hpp>
#include <fluxions_file_system.hpp>
#include <fluxions_simple_geometry_mesh.hpp>


namespace Fluxions {
	SimpleGeometryMesh::SimpleGeometryMesh() {}


	SimpleGeometryMesh::~SimpleGeometryMesh() {}


	bool SimpleGeometryMesh::load() {
		if (!loadOBJ(path()))
			return false;
		return SimpleLoadableResource::load();
	}

	void SimpleGeometryMesh::unload() {
		SimpleLoadableResource::unload();
	}

	void SimpleGeometryMesh::cacheToDisk() {
		SimpleLoadableResource::cacheToDisk();
	}

	void SimpleGeometryMesh::fetchCache() {
		SimpleLoadableResource::fetchCache();
	}

	size_t SimpleGeometryMesh::sizeInBytes() const {
		size_t surfaceSize{ 0 };
		for (auto& s : Surfaces) {
			surfaceSize += s.sizeInBytes();
		}

		return SimpleLoadableResource::sizeInBytes() +
			(Vertex::sizeInBytes() * Vertices.size()) +
			(sizeof(unsigned) * Indices.size()) +
			surfaceSize;
	}

	bool SimpleGeometryMesh::loadOBJ(const std::string& filename) {
		std::string cache_filename = filename + ".cache";
		FilePathInfo fpi_original(filename);
		FilePathInfo fpi_cache(cache_filename);

		// Save name and path for possible reload later
		setName(fpi_original.filename());
		setPath(fpi_original.shortestPath());

		if (fpi_cache.exists()) {
			// Is the original file newer than the cache?
			if (fpi_original.lastWriteTime() <= fpi_cache.lastWriteTime()) {
				HFLOGINFO("'%s' ... reading cached OBJ '%s'", name_cstr(), cache_filename.c_str());
				return loadCache(cache_filename);
			}
		}

		HFLOGINFO("'%s' ... loading", name_cstr());
		int curSurface = 0;
		std::string surfaceName;
		std::string objectName;
		std::string materialLibrary;
		float v[3]{};
		int iv[3]{};
		std::vector<Vector3f> vList;
		std::vector<Vector3f> vnList;
		std::vector<Vector2f> vtList;
		std::vector<TVector3<int>> faceList;
		std::map<long long, int> indexMap; // this maps the unique combo of of vertices
		std::map<int, Vertex> vertexMap;
		long lastIndex = 0;
		int newIndex = 0;
		int first = 0;
		// int count = 0;
		int firstVertex = 0;
		double size = 1.0;

		bool optimizeIndexing = false;

		BoundingBox.reset();

		std::ifstream fin(filename.c_str());

		if (!fin)
			return false;

		createSimpleModel(0, 0, 1);
		int linecount = 0;
		int totallinecount = 0;
		while (1) {
			std::string line;
			getline(fin, line);
			std::istringstream istr(line);
			linecount++;
			totallinecount++;
			if (!fin) {
				// flush remaining polygons to last surface (if any)
				if (Surfaces.size() != 0) {
					Surfaces[curSurface].count = (int)faceList.size() * 3;

					HFLOGINFO("'%s' ... adding %d new faces starting at %d to '%s'", name_cstr(), faceList.size(), first, Surfaces[curSurface].name_cstr());

					// 2. add indices (triangles)
					for (auto it = faceList.begin(); it != faceList.end(); it++) {
						Indices.push_back(it->x + firstVertex);
						Indices.push_back(it->y + firstVertex);
						Indices.push_back(it->z + firstVertex);
					}
					first = (int)Indices.size();
					faceList.clear();
				}
				break;
			}

			std::string str;
			istr >> str;
			if (str == "o") {
				linecount = 0;
				istr >> objectName;
				toloweridentifier(objectName);
				HFLOGINFO("'%s' ... adding new object %s", name_cstr(), objectName.c_str());
			}
			else if (str == "g") {
				linecount = 0;
				istr >> surfaceName;
				toloweridentifier(surfaceName);
				HFLOGINFO("'%s' ... changing surface name to %s", name_cstr(), surfaceName.c_str());
				Surfaces[curSurface].surfaceName = surfaceName;
			}
			else if (str == "usemtl") {
				bool createSurface = false;

				// flush old polygons to last surface (if any)
				if (Surfaces.size() != 0 && !faceList.empty()) {
					Surfaces[curSurface].count = (int)faceList.size() * 3;

					HFLOGINFO("'%s' ... adding %d new faces starting at %d to %s", name_cstr(), faceList.size(), first, Surfaces[curSurface].surfaceName.c_str());

					// 2. add indices (triangles)
					for (auto it = faceList.begin(); it != faceList.end(); it++) {
						Indices.push_back(it->x + firstVertex);
						Indices.push_back(it->y + firstVertex);
						Indices.push_back(it->z + firstVertex);
					}
					first = (int)Indices.size();
					faceList.clear();
					createSurface = true;
				}

				if (createSurface) {
					// create new surface
					Surface surface;
					surface.first = first;
					surface.mode = SurfaceType::Triangles;
					Surfaces.push_back(surface);
					curSurface = (int)Surfaces.size() - 1;
				}

				istr >> str;
				toloweridentifier(str);
				Surfaces[curSurface].materialLibrary = materialLibrary;
				Surfaces[curSurface].materialName = str;
				Materials[str] = materialLibrary;
				HFLOGINFO("'%s' ... using material '%s' from '%s'", name_cstr(), str.c_str(), materialLibrary.c_str());
			}
			else if (str == "mtllib") {
				if (add_mtllib(istr, materialLibrary, fpi_original.parentPath())) {
					HFLOGINFO("'%s' ... adding mtllib '%s' to load list", name_cstr(), materialLibrary.c_str());
				}
				else {
					HFLOGWARN("'%s' ... mtllib '%s' was not found", name_cstr(), materialLibrary.c_str());
				}
			}
			else if (str == "v") {
				istr >> v[0] >> v[1] >> v[2];
				vList.push_back(Vector3f(v));
			}
			else if (str == "vn") {
				istr >> v[0] >> v[1] >> v[2];
				vnList.push_back(Vector3f(v));
			}
			else if (str == "vt") {
				istr >> v[0] >> v[1];
				vtList.push_back(Vector2f(v));
			}
			else if (str == "f") {
				int i1 = 0, i2 = 0, i3 = 0;
				long long longIndex;
				char trashChar;
				for (int k = 0; k < 3; k++) {
					istr >> i1;
					istr >> trashChar;
					if (trashChar != '/') {
						// single position (ptr only)
						istr.putback(trashChar);
						i2 = i3 = -1;
					}
					else {
						istr >> trashChar;
						if (trashChar == '/') {
							// ptr//vn case
							i2 = -1;
							istr >> i3;
						}
						else {
							istr.putback(trashChar);
							istr >> i2 >> trashChar;
							if (trashChar != '/') {
								istr.putback(trashChar);
								i3 = i2;
								i2 = -1;
							}
							else {
								istr >> i3;
							}
						}
					}
					if (i1 < 0)
						i1 = (int)vList.size() + i1;
					if (i2 < 0)
						i2 = (int)vtList.size() + i2;
					if (i3 < 0)
						i3 = (int)vnList.size() + i3;

					if (!optimizeIndexing) {
						// BEGIN NOT TRYING TO INDEX THESE THINGS
						iv[k] = lastIndex + k;
						Vertex vtx;
						vtx.position = vList[(size_t)i1 - 1];
						if (i3 >= 0)
							vtx.normal = vnList[(size_t)i3 - 1];
						if (i2 >= 0)
							vtx.texcoord = vtList[(size_t)i2 - 1];
						vertexMap[iv[k]] = vtx;
						BoundingBox += vtx.position;
						// END NOT TRYING TO INDEX THESE THINGS
					}
					else {
						longIndex = i1;
						if (i2 >= 0)
							longIndex |= (long long)(i2) << 48;
						if (i3 >= 0)
							longIndex |= (long long)(i3) << 24;
						if (indexMap.find(longIndex) == indexMap.end()) {
							indexMap[longIndex] = newIndex;
							newIndex++;
						}
						iv[k] = indexMap[longIndex];

						// create vertex if needed
						Vertex vtx;
						if (vertexMap.find(iv[k]) == vertexMap.end()) {
							vtx.position = vList[(size_t)i1 - 1];
							if (i3 >= 0)
								vtx.normal = vnList[(size_t)i3 - 1];
							if (i2 >= 0)
								vtx.texcoord = vtList[(size_t)i2 - 1];
							vertexMap[iv[k]] = vtx;
							BoundingBox += vtx.position;
						}
					}
				}
				if (optimizeIndexing) {
					// create face
					faceList.push_back(TVector3<int>(iv));
				}
				else {
					faceList.push_back(TVector3<int>(lastIndex, lastIndex + 1, lastIndex + 2));
					lastIndex += 3;
				}
			}
		}
		fin.close();

		float scale = 1;
		if (size < 0)
			scale = (float)(-2 * size / BoundingBox.maxSize());

		HFLOGINFO("'%s' ... scale is %f", name_cstr(), scale);
		for (auto it = vertexMap.begin(); it != vertexMap.end(); it++) {
			if (size < 0)
				it->second.position -= BoundingBox.center();

#define MAKE_FINITE(x)  \
	if (!isfinite((x))) \
		(x) = 0.0;
			//it->second.position *= scale;

			MAKE_FINITE(it->second.position.x);
			MAKE_FINITE(it->second.position.y);
			MAKE_FINITE(it->second.position.z);
			MAKE_FINITE(it->second.normal.x);
			MAKE_FINITE(it->second.normal.y);
			MAKE_FINITE(it->second.normal.z);
			MAKE_FINITE(it->second.texcoord.x);
			MAKE_FINITE(it->second.texcoord.y);
			MAKE_FINITE(it->second.binormal.x);
			MAKE_FINITE(it->second.binormal.y);
			MAKE_FINITE(it->second.binormal.z);
			MAKE_FINITE(it->second.tangent.x);
			MAKE_FINITE(it->second.tangent.y);
			MAKE_FINITE(it->second.tangent.z);

			Vertices.push_back(it->second);
		}
		HFLOGINFO("'%s' ... max uniform scale is %f", name_cstr(), BoundingBox.maxSize());

		computeTangentVectors();

		HFLOGINFO("'%s' ... writing cached OBJ '%s'", name_cstr(), cache_filename.c_str());
		return saveCache(cache_filename);
	}


	bool SimpleGeometryMesh::add_mtllib(std::istream& istr, std::string& mtllibname, const std::string& basepath) {
		std::string pathToMTL = ReadString(istr);
		FilePathInfo fpi(basepath + pathToMTL);

		// Check if we have added this map already
		if (mtllibs.count(fpi.filename())) {
			return true;
		}

		// If it doesn't exist, try the working directory
		if (fpi.notFound()) {
			fpi.reset("./" + pathToMTL);
		}

		if (fpi.notFound()) {
			return false;
		}

		// update the information in the map list
		mtllibname = fpi.filename();
		mtllibs[mtllibname] = fpi.shortestPath();
		return true;
	}


	bool SimpleGeometryMesh::saveOBJ(const std::string& filename) const {
		HFLOGINFO("Writing OBJ %s", filename.c_str());

		std::ofstream fout(filename.c_str());
		// 1. Output Vertices
		for (auto it = Vertices.begin(); it != Vertices.end(); it++) {
			fout << "v ";
			WriteVector3f(fout, it->position);
			fout << std::endl;
			fout << "vn ";
			WriteVector3f(fout, it->normal);
			fout << std::endl;
			fout << "vt ";
			WriteVector2f(fout, it->texcoord);
			fout << std::endl;
		}

		// 2. Output Surfaces
		for (auto it = Surfaces.begin(); it != Surfaces.end(); it++) {
			fout << "usemtl " << it->materialName << std::endl;
			int triangle = 2;
			for (unsigned i = 0; i < it->count; i++) {
				if (++triangle == 3) {
					fout << std::endl;
					if (it->count - i >= 3)
						fout << "f ";
					triangle = 0;
				}
				fout << Indices[(size_t)it->first + i] << "/";
				fout << Indices[(size_t)it->first + i] << "/";
				fout << Indices[(size_t)it->first + i] << " ";
			}
			fout << std::endl;
		}

		fout.close();
		return true;
	}


	int SimpleGeometryMesh::saveOBJByMaterial(const std::string& filename,
											  const std::string& mtllib,
											  const std::string& materialName) const {
		int count = 0;

		std::ofstream fout(filename.c_str());
		fout << "mtllib " << mtllib << "\n";

		size_t totalVertices = 0;
		for (auto& surface : Surfaces) {
			if (surface.materialName != materialName)
				continue;

			// 1. Output Vertices
			for (unsigned i = 0; i < surface.count; i++) {
				const Vertex& v = Vertices[(size_t)surface.first + i];
				fout << "v ";
				WriteVector3f(fout, v.position) << "\n";
				fout << "vn ";
				WriteVector3f(fout, v.normal) << "\n";
				fout << "vt ";
				WriteVector2f(fout, v.texcoord) << "\n";
			}

			fout << "o " << surface.surfaceName << "\n";
			fout << "usemtl " << materialName << "\n";

			// 2. Output Faces
			for (size_t i = 0; i < surface.count; i += 3) {
				size_t j1 = totalVertices + i;
				size_t j2 = j1 + 1;
				size_t j3 = j1 + 2;
				fout << "f ";
				WriteIndices(fout, Indices[j1], Indices[j1], Indices[j1]);
				WriteIndices(fout, Indices[j2], Indices[j2], Indices[j2]);
				WriteIndices(fout, Indices[j3], Indices[j3], Indices[j3]) << "\n";
			}

			totalVertices += surface.count;
			count++;
		}
		return count;
	}


	int SimpleGeometryMesh::saveOBJByMaterial(const std::string& filename,
											  const std::string& materialName,
											  int materialId) const {
		int count = 0;

		std::ofstream fout(filename.c_str());

		size_t totalVertices = 0;
		for (auto& surface : Surfaces) {
			if (surface.materialName != materialName)
				continue;

			// 1. Output Vertices
			for (unsigned i = 0; i < surface.count; i++) {
				const Vertex& v = Vertices[(size_t)surface.first + i];
				fout << "v ";
				WriteVector3f(fout, v.position) << "\n";
				fout << "vn ";
				WriteVector3f(fout, v.normal) << "\n";
				fout << "vt ";
				WriteVector2f(fout, v.texcoord) << "\n";
			}

			fout << "o " << surface.surfaceName << "\n";
			fout << "usemtl " << materialId << "\n";

			// 2. Output Faces
			for (size_t i = 0; i < surface.count; i += 3) {
				size_t j1 = totalVertices + i;
				size_t j2 = j1 + 1;
				size_t j3 = j1 + 2;
				fout << "f ";
				WriteIndices(fout, Indices[j1], Indices[j1], Indices[j1]);
				WriteIndices(fout, Indices[j2], Indices[j2], Indices[j2]);
				WriteIndices(fout, Indices[j3], Indices[j3], Indices[j3]) << "\n";
			}

			totalVertices += surface.count;
			count++;
		}
		return count;
	}


	bool SimpleGeometryMesh::saveCache(const std::string& filename) const {
		unsigned vertexCount = (unsigned)Vertices.size();
		unsigned indexCount = (unsigned)Indices.size();
		unsigned surfaceCount = (unsigned)Surfaces.size();

		if (!vertexCount || !indexCount) {
			HFLOGWARN("mesh has no vertices or indices");
			return false;
		}

		// save a cache
		std::ofstream fout(filename, std::ios::binary);
		WriteBinaryElement(fout, vertexCount);
		WriteBinaryElement(fout, indexCount);
		WriteBinaryElement(fout, surfaceCount);

		WriteBinaryStringMap(fout, mtllibs);

		WriteBinaryElement(fout, Vertices);
		WriteBinaryElement(fout, Indices);

		for (unsigned i = 0; i < surfaceCount; i++) {
			unsigned mode = (unsigned)Surfaces[i].mode;
			unsigned first = Surfaces[i].first;
			unsigned count = Surfaces[i].count;

			WriteBinaryElement(fout, mode);
			WriteBinaryElement(fout, first);
			WriteBinaryElement(fout, count);
			WriteBinaryString(fout, Surfaces[i].materialName);
			WriteBinaryString(fout, Surfaces[i].materialLibrary);
			WriteBinaryString(fout, Surfaces[i].surfaceName);
		}

		fout.close();
		return true;
	}


	bool SimpleGeometryMesh::loadCache(const std::string& filename) {
		std::ifstream fin(filename, std::ios::binary);
		if (!fin)
			return false;
		unsigned vertexCount = 0;
		unsigned indexCount = 0;
		unsigned surfaceCount = 0;

		ReadBinaryElement(fin, vertexCount);
		ReadBinaryElement(fin, indexCount);
		ReadBinaryElement(fin, surfaceCount);

		if (!vertexCount || !indexCount) {
			HFLOGWARN("Mesh has no vertices or indices");
			return false;
		}

		mtllibs.clear();
		ReadBinaryStringMap(fin, mtllibs);

		Vertices.resize(vertexCount);
		Indices.resize(indexCount);
		Surfaces.resize(surfaceCount);
		BoundingBox.reset();

		ReadBinaryElement(fin, Vertices, vertexCount);
		ReadBinaryElement(fin, Indices, indexCount);

		for (unsigned i = 0; i < surfaceCount; i++) {
			unsigned mode = 0;
			unsigned first = 0;
			unsigned count = 0;
			std::string mtlName;
			std::string mtllibName;
			std::string surfaceName;

			ReadBinaryElement(fin, mode);
			ReadBinaryElement(fin, first);
			ReadBinaryElement(fin, count);
			ReadBinaryString(fin, mtlName);
			ReadBinaryString(fin, mtllibName);
			ReadBinaryString(fin, surfaceName);

			Surfaces[i].mode = (SimpleGeometryMesh::SurfaceType)mode;
			Surfaces[i].first = first;
			Surfaces[i].count = count;
			Surfaces[i].materialName = mtlName;
			Surfaces[i].materialLibrary = mtllibName;
			Surfaces[i].surfaceName = surfaceName;
		}

		for (auto& v : Vertices) {
			BoundingBox += v.position;
		}

		Materials.clear();
		for (auto& surface : Surfaces) {
			Materials[surface.materialName] = surface.materialLibrary;
		}

		fin.close();
		return true;
	}


	void SimpleGeometryMesh::createSimpleModel(int vertexCount, int indexCount, int surfaceCount) {
		clear();
		resize(vertexCount, indexCount, surfaceCount);
	}


	void SimpleGeometryMesh::resize(int vertexCount, int indexCount, int surfaceCount) {
		if (vertexCount > 0)
			Vertices.resize(vertexCount);
		if (indexCount > 0)
			Indices.resize(indexCount);
		if (surfaceCount > 0)
			Surfaces.resize(surfaceCount);
	}


	void SimpleGeometryMesh::computeTangentVectors() {
		for (size_t i = 0; i < Vertices.size(); i++) {
			Vertices[i].tangent = Vector3f(0, 0, 0);
			Vertices[i].binormal = Vector3f(0, 0, 0);
		}

		for (size_t i = 0; i < Indices.size(); i += 3) {
			int indices[3];
			indices[0] = Indices[i];
			indices[1] = Indices[i + 1];
			indices[2] = Indices[i + 2];
			Vector3f v1, v2;
			v1 = Vertices[indices[1]].position - Vertices[indices[0]].position;
			v2 = Vertices[indices[2]].position - Vertices[indices[0]].position;
			Vector3f faceNormal = CrossProduct(v1, v2);
			faceNormal.normalize();

			float s1, t1, s2, t2;
			s1 = Vertices[indices[1]].texcoord.x - Vertices[indices[0]].texcoord.x;
			t1 = Vertices[indices[1]].texcoord.y - Vertices[indices[0]].texcoord.y;
			s2 = Vertices[indices[2]].texcoord.x - Vertices[indices[0]].texcoord.x;
			t2 = Vertices[indices[2]].texcoord.x - Vertices[indices[0]].texcoord.y;
			Vector3f Tx, Ty, Tz;
			float det = s1 * t2 - s2 * t1;
			float coef = 1.0f / det;
			Tx.x = coef * (v1.x * t2 - v2.x * t1);
			Tx.y = coef * (v1.y * t2 - v2.y * t1);
			Tx.z = coef * (v1.z * t2 - v2.z * t1);
			Ty.x = coef * (v2.x * s1 - v1.x * s2);
			Ty.y = coef * (v2.y * s1 - v1.y * s2);
			Ty.z = coef * (v2.z * s1 - v1.z * s2);

			for (int j = 0; j < 3; j++) {
				Vertices[indices[j]].tangent += Tx;
				Vertices[indices[j]].binormal += Ty;
			}
		}


		for (size_t i = 0; i < Vertices.size(); i++) {
			Vertices[i].normal.normalize();
			Vertices[i].tangent.normalize();
			Vertices[i].binormal.normalize();

			MAKE_FINITE(Vertices[i].position.x);
			MAKE_FINITE(Vertices[i].position.y);
			MAKE_FINITE(Vertices[i].position.z);
			MAKE_FINITE(Vertices[i].normal.x);
			MAKE_FINITE(Vertices[i].normal.y);
			MAKE_FINITE(Vertices[i].normal.z);
			MAKE_FINITE(Vertices[i].texcoord.x);
			MAKE_FINITE(Vertices[i].texcoord.y);
			MAKE_FINITE(Vertices[i].binormal.x);
			MAKE_FINITE(Vertices[i].binormal.y);
			MAKE_FINITE(Vertices[i].binormal.z);
			MAKE_FINITE(Vertices[i].tangent.x);
			MAKE_FINITE(Vertices[i].tangent.y);
			MAKE_FINITE(Vertices[i].tangent.z);
			//Vector3f n = Vertices[i].normal;
			//Vector3f t = Vertices[i].tangent;
			//Vector3f b = Vertices[i].binormal;
			//t.Normalize();
			//b.Normalize();

			//float T2 = t.LengthSquared();
			//Vector3f tprime = (t - n * DotProduct(n, t));
			//Vector3f bprime = (b - n * DotProduct(n, n) - tprime * DotProduct(tprime, b) / T2);

			//Matrix3f M(tprime.X, tprime.y, tprime.z, bprime.X, bprime.y, bprime.z, n.X, n.y, n.z);

			//Vertices[i].tangent = tprime;
			//Vertices[i].tangent.w = M.Determinant();
			//Vertices[i].tangent.w = (DotProduct(CrossProduct(n, t), b) < 0.0f) ? -1.0f : 1.0f;
			//Vertices[i].binormal = bprime;
		}
	}


	void SimpleGeometryMesh::clear() {
		Vertices.clear();
		Indices.clear();
		Surfaces.clear();
	}


	void SimpleGeometryMesh::transform(const Matrix4f& mat) {
		for (auto& vertex : Vertices) {
			vertex.position = mat * vertex.position;
		}
	}
} // namespace Fluxions
