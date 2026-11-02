/*
This file is part of Nemorino.

Nemorino is free software : you can redistribute it and /or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Nemorino is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Nemorino.If not, see < http://www.gnu.org/licenses/>.
*/



#include <fstream>
#include <cassert>
#include <iostream>
#include <algorithm>
#include "gsl.h"
#include "network.h"

namespace nnue {

	bool Network::Load(std::string filename)
	{
		network_source = "";
		std::uint32_t size = 0;
		std::ifstream stream(filename, std::ios::binary);
		stream.read(reinterpret_cast<char*>(&version), sizeof(version));
		stream.read(reinterpret_cast<char*>(&hash_value), sizeof(hash_value));
		if (hash_value != ID_REDUCED_NETWORK) {
			if (hash_value == ID_REDUCED_NETWORK_0) {
				std::cerr << "Network doesn't match engine versions >= 6.03" << std::endl;
				return false;
			}
			std::string nfile = convert(filename);
			if (nfile.size() > 0) {
				return Load(nfile);
			}
			else return false;
		}
		stream.read(reinterpret_cast<char*>(&size), sizeof(size));
		assert(stream && version == kVersion);
		architecture.resize(size);
		stream.read(&architecture.at(0), size);
		nn.reset(static_cast<network*>(std_aligned_alloc(alignof(network), sizeof(network))));
		std::memset(nn.get(), 0, sizeof(network));
		nn->ReadParameters(stream);
		assert(stream && stream.peek() == std::ios::traits_type::eof());
		std::cout << filename << " loaded!" << std::endl;
		network_source = filename;
		return true;
	}

	int16_t Network::score(InputAdapter& inputAdapter) const
	{
		alignas(kCacheLineSize) char buffer[network::kBufferSize];
		alignas(kCacheLineSize) TransformedFeatureType input_data[512];
		const auto output = nn->Propagate(inputAdapter, input_data, buffer);
		return gsl::narrow_cast<int16_t>(output[0] / FV_SCALE);
	}

	std::string Network::convert(std::string ifile)
	{
		const size_t index = ifile.rfind(".");
		std::string ofile;
		if (index == std::string::npos) {
			ofile = ifile + ".converted";
		}
		else {
			ofile = ifile.substr(0, index) + ".converted" + ifile.substr(index);
		}
		return convert(ifile, ofile);
	}

	std::string Network::convert(std::string ifile, std::string ofile)
	{
		std::ifstream f(ofile);
		if (f.good()) {
			std::cout << ofile << " already exists!" << std::endl;
			return ofile;
		}

		std::ifstream stream;
		stream.open(ifile, std::ios::binary);
		std::string l;
		if (!stream.is_open()) {
			std::cout << "Couldn't open " << ifile << std::endl;
			return "";
		}
		// get length of file:
		stream.seekg(0, stream.end);
		const long long length = stream.tellg();
		stream.seekg(0, stream.beg);
		std::uint32_t size = 0;
		std::uint32_t version = 0;
		std::uint32_t hash_value = 0;
		std::uint32_t header = 0;
		stream.read(reinterpret_cast<char*>(&version), sizeof(version));
		stream.read(reinterpret_cast<char*>(&hash_value), sizeof(hash_value));
		if (hash_value == ID_REDUCED_NETWORK) {
			std::cout << "File " << ifile << " is already converted!" << std::endl;
		}
		else {
			std::ofstream output;
			output.open(ofile, std::ios::out | std::ios::binary);
			if (!output.is_open()) {
				std::cout << "Couldn't open " << ofile << std::endl;
				return "";
			}
			output.write(reinterpret_cast<char*>(&version), sizeof(version));
			hash_value = ID_REDUCED_NETWORK;
			output.write(reinterpret_cast<char*>(&hash_value), sizeof(hash_value));
			stream.read(reinterpret_cast<char*>(&size), sizeof(size));
			output.write(reinterpret_cast<char*>(&size), sizeof(size));
			std::array<char, 256 * 640 * 2> buffer;
			stream.read(buffer.data(), size);
			output.write(buffer.data(), size);
			stream.read(reinterpret_cast<char*>(&header), sizeof(header));
			output.write(reinterpret_cast<char*>(&header), sizeof(header));
			stream.read(buffer.data(), 512);
			output.write(buffer.data(), 512);
			for (int i = 0; i < 64; ++i) {
				stream.read(buffer.data(), 256 * 2); //These are the bonanza weights, which aren't used
				stream.read(buffer.data(), 256 * 640 * 2);
				output.write(buffer.data(), 256 * 640 * 2);
			}
			const long long cur_pos = stream.tellg();
			const long long remaining = length - cur_pos;
			stream.read(buffer.data(), remaining);
			output.write(buffer.data(), remaining);
			output.close();
		}
		stream.close();
		return ofile;
	}

}
