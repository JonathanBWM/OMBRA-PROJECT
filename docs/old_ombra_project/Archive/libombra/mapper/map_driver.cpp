#include "map_driver.h"

#include "util.hpp"
#include "drv_image.h"
#include "kernel_ctx.h"
#include <intrin.h>
#include <algorithm>

namespace {
	void GenerateGarbage(uint8_t* buffer, size_t size) {
		uint64_t seed = __rdtsc();
		for (size_t i = 0; i < size; i++) {
			seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
			seed ^= (seed >> 33);
			buffer[i] = static_cast<uint8_t>(seed ^ i);
		}
	}

	bool ContainsPESignature(const uint8_t* buffer, size_t size) {
		for (size_t i = 0; i < size - 1; i++) {
			if (buffer[i] == 0x4D && buffer[i + 1] == 0x5A) return true;
			if (buffer[i] == 0x50 && buffer[i + 1] == 0x45) return true;
		}
		return false;
	}

	void WipeDriverHeaders(mapper::kernel_ctx& ctx, uint64_t pool_base, size_t header_size) {
		constexpr size_t CHUNK_SIZE = 0x1000;
		uint8_t garbage[CHUNK_SIZE];

		size_t offset = 0;
		while (offset < header_size) {
			size_t chunk = (std::min)(CHUNK_SIZE, header_size - offset);
			GenerateGarbage(garbage, chunk);
			while (ContainsPESignature(garbage, chunk)) {
				GenerateGarbage(garbage, chunk);
			}
			ctx.write_kernel(reinterpret_cast<void*>(pool_base + offset), garbage, chunk);
			offset += chunk;
		}
	}
}

NTSTATUS mapper::map_driver(std::string driver_name, uintptr_t param1, uintptr_t param2, bool bAllocationPtrParam1, bool bAllocationSizeParam2, uintptr_t* allocBase)
{
	std::vector<std::uint8_t> drv_buffer;
	util::open_binary_file(driver_name.c_str(), drv_buffer);
	if (!drv_buffer.size())
	{
		std::printf("[-] invalid drv_buffer size\n");
		return -1;
	}
	return map_driver(drv_buffer, param1, param2, bAllocationPtrParam1, bAllocationSizeParam2, allocBase);
}

NTSTATUS mapper::map_driver(const std::vector<std::uint8_t>& driver, uintptr_t param1, uintptr_t param2, bool bAllocationPtrParam1, bool bAllocationSizeParam2, uintptr_t* allocBase)
{
	if (!allocBase)
		return STATUS_INVALID_PARAMETER;

	mapper::drv_image image(driver);
	mapper::kernel_ctx ctx;

	const auto _get_export_name = [&](const char* base, const char* name)
	{
		return reinterpret_cast<std::uintptr_t>(util::get_kernel_export(base, name));
	};

	image.fix_imports(_get_export_name);
	image.map();

	void* pool_base = 0;

	if (!*allocBase) {
		pool_base =
			ctx.allocate_pool(
				image.size(),
				NonPagedPool
			);
		*allocBase = (uintptr_t)pool_base;
	}
	else {
		pool_base = (void*)*allocBase;
	}

	image.relocate(pool_base);
	ctx.write_kernel(pool_base, image.data(), image.size());
	auto entry_point = reinterpret_cast<std::uintptr_t>(pool_base) + image.entry_point();

	auto result = ctx.syscall<DRIVER_INITIALIZE>
		(
			(PVOID)entry_point,
			bAllocationPtrParam1 ? (uintptr_t)(pool_base) : (uintptr_t)(param1),
			bAllocationSizeParam2 ? (uintptr_t)(image.size()) : (uintptr_t)(param2)
		);

	//=========================================================================
	// PE Header Elimination - Anti-Memory Scanner Mitigation
	// Randomize PE headers with LCG-based garbage generation
	// Ensures no PE signatures remain in the garbage data
	// This runs AFTER DriverEntry succeeds - headers no longer needed
	//=========================================================================
	if (NT_SUCCESS(result))
	{
		WipeDriverHeaders(ctx, reinterpret_cast<uint64_t>(pool_base), image.header_size());

		// Also wipe DOS stub (first 64 bytes)
		uint8_t zeroes[64] = { 0 };
		ctx.write_kernel(pool_base, zeroes, sizeof(zeroes));
	}

	return result;
}
