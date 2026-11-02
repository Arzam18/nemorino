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
#include <sstream>
#include <cstdlib>
#include "types.h"

#ifdef EMBED_NEMORINO_NET
extern "C" {
extern const unsigned char _binary_nemorino_net_16_nnue_start[];
extern const unsigned char _binary_nemorino_net_16_nnue_end[];
}
#endif
#include "gsl.h"
#include "network.h"

namespace nnue {

	bool Network::Load(std::string filename)
	{
		network_source = "";
		std::uint32_t size = 0;
		std::ifstream file_stream(filename, std::ios::binary);
		std::istringstream embedded_stream;
		std::istream* stream = &file_stream;
		bool using_embedded_network = false;

#ifdef EMBED_NEMORINO_NET
		if (!file_stream.is_open()) {
			const std::size_t embedded_size = static_cast<std::size_t>(
				_binary_nemorino_net_16_nnue_end - _binary_nemorino_net_16_nnue_start);
			if (embedded_size < 16) {
				std::cerr << "Embedded Nemorino NNUE network is missing or empty" << std::endl;
				return false;
			}
			std::string embedded_data(
				reinterpret_cast<const char*>(_binary_nemorino_net_16_nnue_start),
				embedded_size);
			embedded_stream.str(std::move(embedded_data));
			stream = &embedded_stream;
			using_embedded_network = true;
			filename = "embedded:nemorino_net_16.nnue";
		}
#endif

		stream->read(reinterpret_cast<char*>(&version), sizeof(version));
		stream->read(reinterpret_cast<char*>(&hash_value), sizeof(hash_value));
		if (!*stream) {
			std::cerr << "Unable to read NNUE network header: " << filename << std::endl;
			return false;
		}
		if (hash_value != ID_REDUCED_NETWORK) {
			if (hash_value == ID_REDUCED_NETWORK_0) {
				std::cerr << "Network doesn't match engine versions >= 6.03" << std::endl;
				return false;
			}
			if (using_embedded_network) {
				std::cerr << "Embedded NNUE network is not in Nemorino's reduced network format" << std::endl;
				return false;
			}
			std::string nfile = convert(filename);
			if (nfile.size() > 0)
				return Load(nfile);
			return false;
		}
		if (version != kVersion) {
			std::cerr << "NNUE network version does not match Nemorino 6.11" << std::endl;
			return false;
		}
		stream->read(reinterpret_cast<char*>(&size), sizeof(size));
		if (!*stream || size == 0) {
			std::cerr << "Invalid NNUE architecture section" << std::endl;
			return false;
		}
		architecture.resize(size);
		stream->read(&architecture.at(0), size);
		if (!*stream) {
			std::cerr << "Truncated NNUE architecture section" << std::endl;
			return false;
		}
		nn.reset(static_cast<network*>(std_aligned_alloc(alignof(network), sizeof(network))));
		if (!nn) {
			std::cerr << "Unable to allocate aligned NNUE network memory" << std::endl;
			return false;
		}
		std::memset(nn.get(), 0, sizeof(network));
		if (!nn->ReadParameters(*stream) || stream->peek() != std::ios::traits_type::eof()) {
			std::cerr << "NNUE network data is incomplete or has trailing data" << std::endl;
			nn.reset();
			return false;
		}
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
